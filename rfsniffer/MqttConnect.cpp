#include "../libs/libutils/logging.h"
#include "../libs/libutils/strutils.h"
#include "../libs/libutils/Exception.h"
#include "MqttConnect.h"
#include "../libs/librf/RFM69OOK.h"

using namespace strutils;


CMqttConnection::CMqttConnection(string Server, CLog *log, RFM69OOK *rfm,
                                 CConfigItem *devicesConfig)
    : m_Server(Server), m_Log(log), m_isConnected(false),
      mosquittopp("RFsniffer"), m_RFM(rfm), m_devicesConfig(devicesConfig)
{
    m_Server = Server;
    m_Log = log;

    connect(m_Server.c_str());
    loop_start();
}

CMqttConnection::~CMqttConnection()
{
    loop_stop(true);
}


void CMqttConnection::on_connect(int rc)
{
    m_Log->Printf(1, "mqtt::on_connect(%d)", rc);

    if (!rc) {
        m_isConnected = true;
    }

    subscribe(NULL, "/devices/noolite_tx_0xd61/controls/#");
    subscribe(NULL, "/devices/noolite_tx_0xd62/controls/#");
    subscribe(NULL, "/devices/noolite_tx_0xd63/controls/#");
}

void CMqttConnection::on_disconnect(int rc)
{
    m_isConnected = false;
    m_Log->Printf(1, "mqtt::on_disconnect(%d)", rc);
}

void CMqttConnection::on_publish(int mid)
{
    m_Log->Printf(5, "mqtt::on_publish(%d)", mid);
}

void CMqttConnection::on_message(const struct mosquitto_message *message)
{
    try {
        m_Log->Printf(6, "mqtt::on_message(%s=%s)", message->topic, message->payload);
        String::Vector v = String(message->topic).Split('/');

        if (v.size() != 6)
            return;

        if (v[5] != "on")
            return;

        string addr = v[2];
        size_t pos = addr.find("0x");
        if (pos == string::npos || pos > addr.length() - 2)
            return;
        addr = addr.substr(pos + 2);

        string control = v[4];

        m_Log->Printf(1, "%s control %s set to %s", addr.c_str(), control.c_str(), message->payload);
        uint8_t cmd = 4;
        string extra;

        if (control == "state")
            cmd = atoi((char *)message->payload) ? 2 : 0;
        else if (control == "level") {
            cmd = 6;
            extra = string(" level=") + (char *)message->payload;
        } else if (control == "color") {
            cmd = 6;
            String::Vector v = String((char *)message->payload).Split(';');
            if (v.size() == 3) {
                extra += " fmt=3 r=" + v[0] + " g=" + v[1] + " b=" + v[2];
            }
        }

        else {
            cmd = m_nooLite.getCommand(control);
            if (cmd == CRFProtocolNooLite::nlcmd_error)
                return;
        }


        static uint8_t buffer[100];
        size_t bufferSize = sizeof(buffer);
        string command = "nooLite:cmd=" + itoa(cmd) + " addr=" + addr + extra;
        m_Log->Printf(1, "%s", command.c_str());
        m_nooLite.EncodeData(command, 2000, buffer, bufferSize);
        if (m_RFM) {
            m_RFM->send(buffer, bufferSize);
            m_RFM->receiveBegin();
        }
    } catch (CHaException ex) {
        m_Log->Printf(0, "Exception %s (%d)", ex.GetMsg().c_str(), ex.GetCode());
    }

}

void CMqttConnection::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    m_Log->Printf(5, "mqtt::on_subscribe(%d)", mid);
}

void CMqttConnection::on_unsubscribe(int mid)
{
    m_Log->Printf(5, "mqtt::on_message(%d)", mid);
}

void CMqttConnection::on_log(int level, const char *str)
{
    m_Log->Printf(10, "mqtt::on_log(%d, %s)", level, str);
}

void CMqttConnection::on_error()
{
    m_Log->Printf(1, "mqtt::on_error()");
}

/*!
 *  NewMessage: function gets a string looking like:
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
        m_Log->Printf(3, "CMqttConnection::NewMessage - Incorrect message: %s", message.c_str());
        return;
    }
    String::Map values = value.SplitToPairs(' ', '=');

    // process copies
    {
        if (message != lastMessage) {
            static const String repeatString = "__repeat";

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
            if (difftime(time(NULL), lastMessageReceiveTime) < 2)
                return;
            else {
                lastMessageCount = 1;
            }
        }
        if (lastMessageCount < lastMessageNeedCount)
            return;
    }

    if (type == "RST") {
        m_Log->Printf(3, "Msg from RST %s", value.c_str());

        string id = values["id"], t = values["t"], h = values["h"];

        if (id.empty() || t.empty() || h.empty()) {
            m_Log->Printf(3, "Msg from RST INCORRECT %s", value.c_str());
            return;
        }

        string name = string("RST_") + id;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            string desc = string("RST sensor") + " [" + id + "]";
            dev = new CWBDevice(name, desc);
            dev->addControl("Temperature", CWBControl::Temperature, true);
            dev->addControl("Humidity", CWBControl::RelativeHumidity, true);
            CreateDevice(dev);
        }

        dev->set("Temperature", t);
        dev->set("Humidity", h);
    } else if (type == "nooLite") {
        m_Log->Printf(3, "Msg from nooLite %s", value.c_str());

        // nooLite:sync=80 cmd=21 type=2 t=24.6 h=39 s3=ff bat=0 addr=1492 fmt=07 crc=a2

        String id = values["addr"], cmd = values["cmd"];

        if (id.empty() || cmd.empty()) {
            m_Log->Printf(3, "Msg from nooLite INCORRECT %s", value.c_str());
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
                string name = string("noolite_rx_0x_switch") + id;
                bool enableForAWhile = (cmdInt == 24 || cmdInt == 25);
                static const string control_name = "state";
                static const string interval_control_name = "timeout";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    string desc = string("Noolite switch ") + " [0x" + id + "]";
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
                string name = string("noolite_rx_0x_color") + id;
                static const string control_name = "Color";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    string desc = string("Noolite color ") + " [0x" + id + "]";
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
                string name = string("noolite_rx_0x_th") + id;
                string t = values["t"], h = values["h"];
                static const string low_battery_control_name = "Low battery";
                CWBDevice *dev = m_Devices[name];
                if (!dev) {
                    string desc = string("Noolite Sensor PT111") + " [0x" + id + "]";
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
                string name = string("noolite_rx_0x_unknown") + id;
                CWBDevice *dev = m_Devices[name];
                static const string cmd_control_name = "command",
                                    cmd_desc_control_name = "command_description";
                if (!dev) {
                    string desc = string("Noolite device ") + " [0x" + id + "]";
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
        m_Log->Printf(3, "Msg from Oregon %s", value.c_str());

        const string sensorType = values["type"], id = values["id"], ch = values["ch"];

        if (sensorType.empty() || id.empty() || ch.empty()) {
            m_Log->Printf(3, "Msg from Oregon INCORRECT %s", value.c_str());
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

        const string name = string("oregon_rx_") + sensorType + "_" + id + "_" + ch;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            const string desc = string("Oregon sensor [") + sensorType + "] (" + id + "-" + ch + ")";
            dev = new CWBDevice(name, desc);

            for (auto control_pair : control_and_value)
                dev->addControl("", control_pair.first, true);

            CreateDevice(dev);
        }
        for (auto control_pair : control_and_value)
            dev->set(control_pair.first, control_pair.second);

    } else if (type == "X10") {
        m_Log->Printf(3, "Msg from X10 %s", message.c_str());

        CWBDevice *dev = m_Devices["X10"];
        if (!dev) {
            dev = new CWBDevice("X10", "X10");
            dev->addControl("Command", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set("Command", value);
    } else if (type == "Raex" || type == "Livolo" || type == "Rubitek" ) {
        m_Log->Printf(3, "Msg from remote control (Raex | Livolo | Rubitek) %s", message.c_str());

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
        int msg = values["msg_id"].IntValue(), ch = values["msg_id"].IntValue();
        
        string name = String::ComposeFormat("hs24bits_%d_%d", msg, ch);
        
        static const string control_name = "state";
        
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            string desc = String::ComposeFormat("HS24Bits %d (%d)", msg, ch);
            dev = new CWBDevice(name, desc);
            dev->addControl(control_name, CWBControl::Switch, true);
            CreateDevice(dev);
        }
        
        dev->setForAndThen(control_name, "1", 10, "0");        
    }

    SendUpdate();
}

void CMqttConnection::publishString(const string &path, const string &value)
{
    publish(NULL, path.c_str(), value.size(), value.c_str(), 0, true);
}

void CMqttConnection::publishStringMap(const CWBDevice::StringMap &values)
{
    for (auto i : values) {
        publishString(i.first, i.second);
        m_Log->Printf(5, "publish %s=%s", i.first.c_str(), i.second.c_str());
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
