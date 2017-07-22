#include "MqttConnect.h"

#include <vector>
#include <algorithm>

#include "../libs/libutils/logging.h"
#include "../libs/libutils/strutils.h"
#include "../libs/libutils/Exception.h"
#include "../libs/libutils/DebugPrintf.h"
#include "../libs/librf/RFM69OOK.h"

using namespace strutils;
typedef std::string string;


CMqttConnection::CMqttConnection(string Server, RFM69OOK *rfm,
                                 Json::Value devicesConfig, const std::vector<std::string> &enabledFeatures)
    : mosquittopp("RFsniffer"), m_Server(Server), m_isConnected(false),
      m_RFM(rfm), m_devicesConfig(devicesConfig)
{
    DPRINTF_DECLARE(dprintf, false);
    m_Server = Server;

    m_NooLiteTxEnabled = (std::find(enabledFeatures.begin(), enabledFeatures.end(), "noolite_tx") != enabledFeatures.end());

    connect(m_Server.c_str());
    loop_start();

    dprintf("$P CMqttConnection inited, noolite_tx %s\n", (m_NooLiteTxEnabled ? "enabled" : "disabled"));
}

CMqttConnection::~CMqttConnection()
{
    loop_stop(true);
}



void CMqttConnection::CreateNooliteTxUniversal(const std::string &addr) {
    std::string name = String::ComposeFormat("noolite_tx_%s", addr.c_str());

    CWBDevice *dev = m_Devices[name];

    if (dev)
        return;

    std::string desc = String::ComposeFormat("Noolite TX %s", addr.c_str());
    dev = new CWBDevice(name, desc);

    dev->addControl("level", CWBControl::Range, "0", "1", false);
    dev->setMax("level", "100");
    dev->addControl("state", CWBControl::Switch, "0", "2", false);
    dev->addControl("switch", CWBControl::PushButton, "0", "4", false);
    dev->addControl("color", CWBControl::Rgb, "0;0;0", "5", false);
    dev->addControl("slowup", CWBControl::PushButton, "0", "6", false);
    dev->addControl("slowdown", CWBControl::PushButton, "0", "7", false);
    dev->addControl("slowswitch", CWBControl::PushButton, "0", "8", false);
    dev->addControl("slowstop", CWBControl::PushButton, "0", "9", false);
    dev->addControl("shadow_level", CWBControl::Range, "0", "10", false);
    dev->setMax("shadow_level", "100");
    dev->addControl("bind", CWBControl::PushButton, "0", "20", false);
    dev->addControl("unbind", CWBControl::PushButton, "0", "21", false);

    CreateDevice(dev);

    /*
    'bind'  : { 'value' : 0,
                'meta': {  'type' : 'pushbutton',
                           'order' : '20',
                           'export' : '0', // what is it??
                        },
              },
    */
}


void CMqttConnection::on_connect(int rc)
{
    LOG(INFO) << "mqtt::on_connect(" << rc << ")";

    if (!rc) {
        m_isConnected = true;
    }
    else {
        LOG(WARN) << "mqtt::on_connect Connection failed";
    }

    if (m_NooLiteTxEnabled) {
        for (const std::string &addr : {"0xd61"/*, "0xd62", "0xd63"*/}) {
            CreateNooliteTxUniversal(addr);
            string topic = String::ComposeFormat("/devices/noolite_tx_%s/controls/#", addr.c_str());

            LOG(INFO) << "subscribe to " << topic;
            subscribe(NULL, topic.c_str());
        }

        SendUpdate();
    }
}

void CMqttConnection::on_disconnect(int rc)
{
    m_isConnected = false;
    LOG(INFO) << "mqtt::on_disconnect(" << rc << ")";
    reconnect_async();
}

void CMqttConnection::on_publish(int mid)
{
    //LOG(INFO) << "mqtt::on_publish(" << mid << ")";
}

void CMqttConnection::on_message(const struct mosquitto_message *message)
{
    DPRINTF_DECLARE(dprintf, true);
    dprintf("$P Start!\n");
    try {
        if (!message->topic || !message->payload) {
            return;
        }

        String payload = (char*)message->payload;
        String topic = (char*)message->topic;
        LOG(INFO) << "CMqttConnection::on_message " << topic << " = " << payload << ")";
        //     /devices /noolite_tx_0xd62/controls      /switch      /on
        String devicesConst, deviceName, controlsConst, controlName, onConst;
        std::tie(devicesConst, deviceName, controlsConst, controlName, onConst) = topic.Split<5>('/');

        if (devicesConst != "devices" || controlsConst != "controls" || onConst != "on") {
            return;
        }

        size_t pos = deviceName.find("0x");
        if (pos == string::npos || pos > deviceName.length() - 2)
            return;
        String addr = deviceName.substr(pos + 2);

        uint8_t cmd = 4;
        std::string extra;

        if (controlName == "state")
            cmd = payload.IntValue() ? 2 : 0;
        else if (controlName == "level") {
            cmd = 6;
            extra = string(" level=") + payload;
        } else if (controlName == "color") {
            cmd = 6;
            String r, g, b;
            std::tie(r, g, b) = payload.Split<3>(';');
            if (!r.empty() && !g.empty() && !b.empty()) {
                extra += " fmt=3 r=" + r + " g=" + g + " b=" + b;
            }
        }
        else {
            cmd = m_nooLite.getCommand(controlName);
            if (cmd == CRFProtocolNooLite::nlcmd_error)
                return;
        }

        static uint8_t buffer[1000];
        size_t bufferSize = sizeof(buffer);
        std::string command = "nooLite:cmd=" + itoa(cmd) + " addr=" + addr + extra;
        LOG(INFO) << "CMqttConnection::on_message command: " << command;

        dprintf("$P Before encoding\n");
        m_nooLite.EncodeData(command, 2000, buffer, bufferSize);
        dprintf("$P After encoding\n");
        if (m_RFM) {
            dprintf("$P Before receiveEnd\n");
            m_RFM->receiveEnd();
            dprintf("$P Before send\n");
            m_RFM->send(buffer, bufferSize);
            dprintf("$P Before receiveBegin\n");
            m_RFM->receiveBegin();
        }
    } catch (CHaException ex) {
        LOG(INFO) << "on_message: Exception " << ex.GetExplanation() << ")";
    } catch (std::exception e) {
        LOG(INFO) << "on_message: caught exception - " << e.what();
    }

}

void CMqttConnection::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    LOG(INFO) << "mqtt::on_subscribe(" << mid << ")";
}

void CMqttConnection::on_unsubscribe(int mid)
{
    LOG(INFO) << "mqtt::on_message(" << mid << ")";
}

void CMqttConnection::on_log(int level, const char *str)
{
    if (level & (MOSQ_LOG_ERR | MOSQ_LOG_WARNING | MOSQ_LOG_NOTICE | MOSQ_LOG_INFO)) {
        LOG(INFO) << "mqtt::on_log(" << level << ", " << str << ")";
    }
}

void CMqttConnection::on_error()
{
    LOG(INFO) << "mqtt::on_error()";
}

/*!
 *  NewMessage: function gets a std::string looking like:
 *      ProtocolName: flip=0 second_arg=123 addr=0x13 low_battery=0 crc=19 __repeat=2
 *      (here flip, ..., crc - usual data, and after them:
 *          __repeat=N - demand to wait
 *          N consequent copies of this message before processing.
 *          This is made because different messages of the same protocol
 *          may need different repeat count)
 *  and parses it and sends update to mqtt.
 */
void CMqttConnection::NewMessage(String message)
{
    String type, value;
    if (message.SplitByExactlyOneDelimiter(':', type, value) != 0) {
        LOG(INFO) << "CMqttConnection::NewMessage - Incorrect message: " << message;
        return;
    }
    String::Map values = value.SplitToPairs(' ', '=');

    // process copies
    {
        if (message != lastMessage) {
            static const std::string repeatString = "__repeat";

            lastMessageReceiveTime = time(NULL);

            lastMessage = message;
            lastMessageCount = 0;
            if (values.count(repeatString))
                lastMessageNeedCount = values[repeatString].IntValue();
            else
                lastMessageNeedCount = 1;
        }

        lastMessageCount++;
        // if lastMessageCount == lastMessageNeedCount then go through block
        if (lastMessageCount > lastMessageNeedCount) {
            // if too often of field "flip" exists then skip message
            if (difftime(time(NULL), lastMessageReceiveTime) < 2 || values.count("flip") > 0)
                return;
            else {
                lastMessageCount = 1;
            }
        }
        if (lastMessageCount < lastMessageNeedCount)
            return;
    }

    if (type == "RST") {
        LOG(INFO) << "Msg from RST " << value;

        String id = values["id"], t = values["t"], h = values["h"];

        if (id.empty() || t.empty() || h.empty()) {
            LOG(INFO) << "Msg from RST INCORRECT " << value;
            return;
        }

        String name = string("RST_") + id;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            String desc = string("RST sensor") + " [" + id + "]";
            dev = new CWBDevice(name, desc);
            dev->addControl("Temperature", CWBControl::Temperature, true);
            dev->addControl("Humidity", CWBControl::RelativeHumidity, true);
            CreateDevice(dev);
        }

        dev->set("Temperature", t);
        dev->set("Humidity", h);
    } else if (type == "nooLite") {
        LOG(INFO) << "Msg from nooLite " << value;

        // nooLite:sync=80 cmd=21 type=2 t=24.6 h=39 s3=ff bat=0 addr=1492 fmt=07 crc=a2

        String id = values["addr"], cmd = values["cmd"];

        if (id.empty() || cmd.empty()) {
            LOG(INFO) << "Msg from nooLite INCORRECT " << value;
            return;
        }

        int cmdInt = cmd.IntValue();

        switch (cmdInt) {
            // Motion sensors PM111, PM112, ...
            case 0: // set 0
            case 2: // set 1
            case 4: // change value between 0 and 1
            case 24:
            case 25: { // set as 1 for a while
                String name = string("noolite_rx_0x_switch") + id;
                bool enableForAWhile = (cmdInt == 24 || cmdInt == 25);
                static const String control_name = "state";
                static const String interval_control_name = "timeout";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    String desc = string("Noolite switch ") + " [0x" + id + "]";
                    dev = new CWBDevice(name, desc);
                    dev->addControl(control_name, CWBControl::Switch, true);
                    if (enableForAWhile)
                        dev->addControl(interval_control_name, CWBControl::Generic, true);
                    CreateDevice(dev);
                }
                if (enableForAWhile) {
                    // PM112, ...
                    dev->setForAndThen(control_name, "1", values["time"].IntValue(), "0");
                    //dev->set(control_name, "1");
                    dev->set(interval_control_name, values["time"]);

                } else if (cmd == "0")
                    dev->set(control_name, "0");
                else if (cmd == "2")
                    dev->set(control_name, "1");
                else if (cmd == "4")
                    dev->set(control_name, dev->getString(control_name) == "1" ? "0" : "1");

                break;
            }

            case 6: { // set brightness
                String name = string("noolite_rx_0x_color") + id;
                static const String control_name = "Color";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    String desc = string("Noolite color ") + " [0x" + id + "]";
                    dev = new CWBDevice(name, desc);
                    dev->addControl(control_name, CWBControl::Rgb, true);
                    CreateDevice(dev);
                }
                dev->set(control_name, String::ComposeFormat("%s;%s;%s",
                         values["r"].c_str(), values["g"].c_str(), values["b"].c_str()));
                break;
            }

            // Temperature sensor
            case 21: { // puts info about temperature and humidity
                String name = string("noolite_rx_0x_th") + id;
                String t = values["t"], h = values["h"];
                static const String low_battery_control_name = "Low battery";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    String desc = string("Noolite Sensor PT111") + " [0x" + id + "]";
                    dev = new CWBDevice(name, desc);
                    dev->addControl("Temperature", CWBControl::Temperature, true);

                    if (h.length() > 0)
                        dev->addControl("Humidity", CWBControl::RelativeHumidity, true);


                    dev->addControl("", CWBControl::BatteryLow, true);

                    CreateDevice(dev);
                }

                dev->set("Temperature", t);
                if (h.length() > 0)
                    dev->set("Humidity", h);
                dev->set(CWBControl::BatteryLow, values["low_bat"]);
                break;
            }

            default: {
                String name = string("noolite_rx_0x_unknown") + id;
                CWBDevice *dev = m_Devices[name];
                static const String cmd_control_name = "command",
                                    cmd_desc_control_name = "command_description";
                if (!dev) {
                    String desc = string("Noolite device ") + " [0x" + id + "]";
                    dev = new CWBDevice(name, desc);
                    dev->addControl(cmd_control_name, CWBControl::Generic, true);
                    dev->addControl(cmd_desc_control_name, CWBControl::Text, true);
                    CreateDevice(dev);
                }
                dev->set(cmd_control_name, cmd);
                dev->set(cmd_desc_control_name, CRFProtocolNooLite::getDescription(cmdInt));
                break;
            }
        }

    } else if (type == "Oregon") {
        LOG(INFO) << "Msg from Oregon " << value;

        const String sensorType = values["type"], id = values["id"], ch = values["ch"];

        if (sensorType.empty() || id.empty() || ch.empty()) {
            LOG(INFO) << "Msg from Oregon INCORRECT " << value;
            return;
        }


        // Fields of data from sensor
        // Format is vector of pairs (key in input string, conforming CWBControl)
        // keys are taken from RFProtocolOregon (in librf)
        const static std::vector< std::pair<string, CWBControl::ControlType> > key_and_controls = {
            {"t", CWBControl::Temperature},
            {"h", CWBControl::RelativeHumidity},

            {"low_bat", CWBControl::BatteryLow},
            {"uv", CWBControl::UltravioletIndex},
            {"rain_rate", CWBControl::PrecipitationRate},
            {"rain_total", CWBControl::PrecipitationTotal},
            {"wind_dir", CWBControl::WindDirection},
            {"wind_speed", CWBControl::WindSpeed},
            {"wind_avg_speed", CWBControl::WindAverageSpeed},
            {"pressure", CWBControl::AtmosphericPressure},
            {"forecast", CWBControl::Forecast},
            {"comfort", CWBControl::Comfort}
        };
        // Getting values of fields
        // Format is vector of pairs (type of control conforming CWBControl, value in input string)
        std::vector< std::pair<CWBControl::ControlType, string> > control_and_value;
        for (auto control_pair : key_and_controls) {
            auto value_iterator = values.find(control_pair.first);
            if (value_iterator != values.end())
                control_and_value.push_back({control_pair.second, value_iterator->second});
        }

        const String name = string("oregon_rx_") + sensorType + "_" + id + "_" + ch;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            const String desc = string("Oregon sensor [") + sensorType + "] (" + id + "-" + ch + ")";
            dev = new CWBDevice(name, desc);

            for (auto control_pair : control_and_value)
                dev->addControl("", control_pair.first, true);

            CreateDevice(dev);
        }
        for (auto control_pair : control_and_value)
            dev->set(control_pair.first, control_pair.second);

    } else if (type == "X10") {
        LOG(INFO) << "Msg from X10 " << message;

        CWBDevice *dev = m_Devices["X10"];
        if (!dev) {
            dev = new CWBDevice("X10", "X10");
            dev->addControl("Command", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set("Command", value);
    } else if (type == "VHome") {
        const String addr = values["addr"];
        const String btn = values["btn"];

        const String devName = "vhome_" + addr;

        CWBDevice *dev = m_Devices[devName];
        if (!dev)
        {
            dev = new CWBDevice(devName, type + " " + addr);
            for (int i = 1; i <= 4; i++)
                dev->addControl(String::ValueOf(i), CWBControl::Switch, true);
            CreateDevice(dev);
        }

        dev->set(btn, dev->getString(btn) != "0" ? "0" : "1");
        LOG(INFO) << "Msg from " << type << " " << message << ". Set " << btn << " to " << dev->getString(btn);
    } else if (type == "EV1527") {
        const String addr = values["addr"];
        const int cmd = values["cmd"].IntValue();

        const String devName = "ev1527_" + addr;

        CWBDevice *dev = m_Devices[devName];
        if (!dev)
        {
            dev = new CWBDevice(devName, type + " " + addr);
            dev->addControl("cmd", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set("cmd", cmd);
        LOG(INFO) << "Msg from " << type << " " << message << ". Cmd: " << cmd;
    } else if (type == "Livolo" || type == "Raex" || type == "Rubitek" ) {
        LOG(INFO) << "Msg from remote control (Raex | Livolo | Rubitek) " << message;

        CWBDevice *dev = m_Devices["Remotes"];
        if (!dev) {
            dev = new CWBDevice("Remotes", "RF Remote controls");
            dev->addControl("Raex", CWBControl::Text, true);
            dev->addControl("Livolo", CWBControl::Text, true);
            dev->addControl("Rubitek", CWBControl::Text, true);
            //dev->AddControl("Raw", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set(type, value);
    } else if (type == "HS24Bits") {
        int msg = values["msg_id"].IntValue(), ch = values["ch"].IntValue();

        String name = String::ComposeFormat("hs24bits_%d_%d", msg, ch);

        static const String control_name = "state";

        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            String desc = String::ComposeFormat("HS24Bits %d (%d)", msg, ch);
            dev = new CWBDevice(name, desc);
            dev->addControl(control_name, CWBControl::Switch, true);
            CreateDevice(dev);
        }

        dev->setForAndThen(control_name, "1", 10, "0");
    }

    SendUpdate();
}

void CMqttConnection::publishString(const std::string &path, const std::string &value)
{
    publish(NULL, path.c_str(), value.size(), value.c_str(), 0, true);
}

void CMqttConnection::publishStringMap(const CWBDevice::StringMap &values)
{
    for (auto i : values) {
        publishString(i.first, i.second);
        LOG(INFO) << "publish " << i.first << " = " << i.second;
    }
}

void CMqttConnection::SendUpdate()
{
    CWBDevice::StringMap valuesForUpdate;

    // read changes into "valuesForUpdate"
    for (auto dev : m_Devices) {
        if (dev.second)
            dev.second->updateValues(valuesForUpdate);
    }

    // do these changes in mqtt
    publishStringMap(valuesForUpdate);
}


void CMqttConnection::SendAliveness()
{
    CWBDevice::StringMap valuesForUpdate;

    // read changes into "valuesForUpdate"
    for (auto dev : m_Devices) {
        if (dev.second)
            dev.second->updateAliveness(valuesForUpdate);
    }

    // do these changes in mqtt
    publishStringMap(valuesForUpdate);
}

void CMqttConnection::SendScheduledChanges()
{
    CWBDevice::StringMap valuesForUpdate;

    // read changes into "valuesForUpdate"
    for (auto dev : m_Devices) {
        if (dev.second)
            dev.second->updateScheduled(valuesForUpdate);
    }

    // do these changes in mqtt
    publishStringMap(valuesForUpdate);
}



void CMqttConnection::CreateDevice(CWBDevice *dev)
{
    // force device to find himself in the device list and set some settings
    dev->findAndSetConfigs(m_devicesConfig);

    m_Devices[dev->getName()] = dev;
    CWBDevice::StringMap valuesForCreate;
    dev->createDeviceValues(valuesForCreate);
    publishStringMap(valuesForCreate);

}
