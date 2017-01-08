#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "MqttConnect.h"
#include "../libs/librf/RFM69OOK.h"

using namespace strutils;


CMqttConnection::CMqttConnection(string Server, CLog *log, RFM69OOK *rfm)
    : m_isConnected(false), mosquittopp("TestMqttConnection"), m_RFM(rfm)
{
    m_Server  = Server;
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
        m_RFM->send(buffer, bufferSize);
        m_RFM->receiveBegin();
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

void CMqttConnection::NewMessage(String message)
{
    String type, value;
    if (message.SplitByExactlyOneDelimiter(':', type, value) != 0) {
        m_Log->Printf(3, "CMqttConnection::NewMessage - Incorrect message: %s", message.c_str());
        return;
    }

    if (type == "RST") {
        m_Log->Printf(3, "Msg from RST %s", value.c_str());

        String::Map values = value.SplitToPairs();
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
        String::Map values = value.SplitToPairs();

        string id = values["addr"], cmd = values["cmd"];

        if (id.empty() || cmd.empty()) {
            m_Log->Printf(3, "Msg from nooLite INCORRECT %s", value.c_str());
            return;
        }

        if (cmd == "21") {
            //noolite_rx_0x1492
            string name = string("noolite_rx_0x") + id;
            string t = values["t"], h = values["h"];
            CWBDevice *dev = m_Devices[name];
            if (!dev) {
                string desc = string("Noolite Sensor PT111") + " [0x" + id + "]";
                dev = new CWBDevice(name, desc);
                dev->addControl("Temperature", CWBControl::Temperature, true);

                if (h.length() > 0)
                    dev->addControl("Humidity", CWBControl::RelativeHumidity, true);

                CreateDevice(dev);
            }

            dev->set("Temperature", t);
            if (h.length() > 0)
                dev->set("Humidity", h);
        } else if (cmd == "0" || cmd == "4" || cmd == "2") {
            //noolite_rx_0x1492
            string name = string("noolite_rx_0x") + id;
            static const string movement_control_name = "Is there a movement";
            CWBDevice *dev = m_Devices[name];
            if (!dev) {
                string desc = string("Noolite Sensor PM111") + " [0x" + id + "]";
                dev = new CWBDevice(name, desc);
                dev->addControl(movement_control_name, CWBControl::Switch, true);

                CreateDevice(dev);
            }

            if (cmd == "0")
                dev->set(movement_control_name, "0");
            else if (cmd == "2")
                dev->set(movement_control_name, "1");
            else if (cmd == "4")
                dev->set(movement_control_name, dev->getString(movement_control_name) == "1" ? "0" : "1");
            else
                dev->set(movement_control_name, "0");
        }
    } else if (type == "Oregon") {
        m_Log->Printf(3, "Msg from Oregon %s", value.c_str());

        String::Map values = value.SplitToPairs();

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
            {"battery", CWBControl::BatteryLow},
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
    /*for (auto i : valuesForUpdate) {
        publishString(i->first, i->second);
        //publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(5, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }*/
}

void CMqttConnection::CreateDevice(CWBDevice *dev)
{
    m_Devices[dev->getName()] = dev;
    CWBDevice::StringMap valuesForCreate;
    dev->createDeviceValues(valuesForCreate);
    publishStringMap(valuesForCreate);
    /*for (auto i : valuesForCreate) {
        publishString(i->first, i->second);
        //publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(5, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }*/
}
