#include "stdafx.h"
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "MqttConnect.h"
#include "../libs/librf/RFM69OOK.h"


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
        string_vector v;
        SplitString(message->topic, '/', v);
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
            string_vector v;
            SplitString((char *)message->payload, ';', v);
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

void CMqttConnection::NewMessage(string message)
{
    string type, value;
    SplitPair(message, ':', type, value);

    if (type == "RST") {
        string_map values;
        SplitValues(value, values);
        string id = values["id"], t = values["t"], h = values["h"];

        string name = string("RST_") + id;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            string desc = string("RST sensor ") + id;
            dev = new CWBDevice(name, desc);
            dev->AddControl("Temperature", CWBControl::Temperature, true);
            dev->AddControl("Humidity", CWBControl::RelativeHumidity, true);
            CreateDevice(dev);
        }

        dev->set("Temperature", t);
        dev->set("Humidity", h);
        m_Log->Printf(3, "Msg from RST %s", value.c_str());
    } else if (type == "nooLite") {
        // nooLite:sync=80 cmd=21 type=2 t=24.6 h=39 s3=ff bat=0 addr=1492 fmt=07 crc=a2

        string_map values;
        SplitValues(value, values);
        string sensorType = values["type"], id = values["addr"], cmd = values["cmd"], t = values["t"],
               h = values["h"];

        if (cmd == "21") {
            //noolite_rx_0x1492
            string name = string("noolite_rx_0x") + id;
            CWBDevice *dev = m_Devices[name];
            if (!dev) {
                string desc = string("Noolite Sensor 0x") + id;
                dev = new CWBDevice(name, desc);
                dev->AddControl("Temperature", CWBControl::Temperature, true);

                if (h.length() > 0)
                    dev->AddControl("Humidity", CWBControl::RelativeHumidity, true);

                CreateDevice(dev);
            }

            dev->set("Temperature", t);
            if (h.length() > 0)
                dev->set("Humidity", h);
        } else if (cmd == "0" || cmd == "4" || cmd == "2") {
            //noolite_rx_0x1492
            string name = string("noolite_rx_0x") + id;
            CWBDevice *dev = m_Devices[name];
            if (!dev) {
                string desc = string("Noolite Sensor 0x") + id;
                dev = new CWBDevice(name, desc);
                dev->AddControl("State", CWBControl::Switch, true);

                CreateDevice(dev);
            }

            if (cmd == "0")
                dev->set("State", "0");
            else if (cmd == "2")
                dev->set("State", "1");
            else if (cmd == "4")
                dev->set("State", dev->getS("State") == "1" ? "0" : "1");
            else
                dev->set("State", "0");
        }
        m_Log->Printf(3, "Msg from nooLite %s", value.c_str());
    } else if (type == "Oregon") {
        string_map values;
        SplitValues(value, values);
        const string sensorType = values["type"], id = values["id"], ch = values["ch"];
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
        for (auto control_pair: key_and_controls) {
            auto value_iterator = values.find(control_pair.first);
            if (value_iterator != values.end())
                control_and_value.push_back({control_pair.second, value_iterator->second});
        }
    
        const string name = string("oregon_rx_") + sensorType + "_" + id + "_" + ch;
        CWBDevice *dev = m_Devices[name];
        if (!dev) {
            const string desc = string("Oregon sensor [") + sensorType + "] (" + id + "-" + ch + ")";
            dev = new CWBDevice(name, desc);
            
            for (auto control_pair: control_and_value) 
                dev->AddControl("", control_pair.first, true);
            
            CreateDevice(dev);
        }
        for (auto control_pair: control_and_value) 
            dev->set(control_pair.first, control_pair.second);
  
        m_Log->Printf(3, "Msg from Oregon %s", value.c_str());
    } else if (type == "X10") {
        CWBDevice *dev = m_Devices["X10"];
        if (!dev) {
            dev = new CWBDevice("X10", "X10");
            dev->AddControl("Command", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set("Command", value);
        m_Log->Printf(3, "Msg from X10 %s", message.c_str());
    } else if (type == "Raex" || type == "Livolo" || type == "Rubitek" ) {
        CWBDevice *dev = m_Devices["Remotes"];
        if (!dev) {
            dev = new CWBDevice("Remotes", "RF Remote controls");
            dev->AddControl("Raex", CWBControl::Text, true);
            dev->AddControl("Livolo", CWBControl::Text, true);
            dev->AddControl("Rubitek", CWBControl::Text, true);
            //dev->AddControl("Raw", CWBControl::Text, true);
            CreateDevice(dev);
        }

        dev->set(type, value);
        m_Log->Printf(3, "New RF message %s", message.c_str());
    }

    SendUpdate();
}

void CMqttConnection::SendUpdate()
{
    string_map v;

    for_each(CWBDeviceMap, m_Devices, dev) {
        if (dev->second)
            dev->second->UpdateValues(v);
    }

    for_each(string_map, v, i) {
        publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(5, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }
}

void CMqttConnection::CreateDevice(CWBDevice *dev)
{
    m_Devices[dev->getName()] = dev;
    string_map v;
    dev->CreateDeviceValues(v);
    for_each(string_map, v, i) {
        publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(5, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }
}
