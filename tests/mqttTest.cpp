/* This test is a test of class CWBDevice and mosquittopp
 * It seems to be a manual test.
 * */

#include "stdafx.h"
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "mosquittopp.h"
#include "../libs/libwb/WBDevice.h"

class TestMqttConnection
    : public mosqpp::mosquittopp
{
    string m_Server, m_sName;
    CLog *m_Log;
    bool m_isConnected;
  public:
    TestMqttConnection();
    ~TestMqttConnection();


    virtual void on_connect(int rc);
    virtual void on_disconnect(int rc);
    virtual void on_publish(int mid);
    virtual void on_message(const struct mosquitto_message *message);
    virtual void on_subscribe(int mid, int qos_count, const int *granted_qos);
    virtual void on_unsubscribe(int mid);
    virtual void on_log(int level, const char *str);
    virtual void on_error();

    void Test();
};

TestMqttConnection::TestMqttConnection()
    : m_isConnected(false), mosquittopp("TestMqttConnection")
{
    m_Server  = "192.168.1.20";
    m_Log = CLog::GetLog("Main");

    connect(m_Server.c_str());
    loop_start();

}

TestMqttConnection::~TestMqttConnection()
{

}


void TestMqttConnection::on_connect(int rc)
{

    m_Log->Printf(1, "on_connect(%d)", rc);

    if (!rc) {
        m_isConnected = true;
    }

}

void TestMqttConnection::on_disconnect(int rc)
{
    m_isConnected = false;
    m_Log->Printf(1, "%s::on_disconnect(%d)", m_sName.c_str(), rc);

}

void TestMqttConnection::on_publish(int mid)
{
    m_Log->Printf(1, "%s::on_publish(%d)", m_sName.c_str(), mid);

}

void TestMqttConnection::on_message(const struct mosquitto_message *message)
{
    m_Log->Printf(1, "%s::on_message(%s=%s)", m_sName.c_str(), message->topic, message->payload);

}

void TestMqttConnection::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    m_Log->Printf(1, "%s::on_subscribe(%d)", m_sName.c_str(), mid);

}

void TestMqttConnection::on_unsubscribe(int mid)
{
    m_Log->Printf(1, "%s::on_message(%d)", m_sName.c_str(), mid);

}

void TestMqttConnection::on_log(int level, const char *str)
{
    m_Log->Printf(1, "%s::on_log(%d, %s)", m_sName.c_str(), level, str);

}

void TestMqttConnection::on_error()
{
    m_Log->Printf(1, "%s::on_error()");

}

void TestMqttConnection::Test()
{
    while (!m_isConnected)
        sleep(1);


    CWBDevice wbdev("TestDev", "Тестовое устройство");
    wbdev.AddControl("Time", CWBControl::Text, true);
    wbdev.AddControl("Random", CWBControl::Temperature, true);
    string_map v;

    wbdev.CreateDeviceValues(v);
    m_Log->Printf(1, "Create test device");

    for_each(string_map, v, i) {
        publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(1, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }
    m_Log->Printf(1, "Created");


    char Buffer[50];
    long Time = time(NULL);
    strftime(Buffer, sizeof(Buffer), "%d/%m %H:%M:%S ", localtime(&Time));

    wbdev.set("Time", Buffer);
    wbdev.set("Random", rand() * 0.01);

    v.clear();
    wbdev.UpdateValues(v);
    m_Log->Printf(1, "Update test device");

    for_each(string_map, v, i) {
        while (!m_isConnected)
            usleep(10);

        publish(NULL, i->first.c_str(), i->second.size(), i->second.c_str(), 0, true);
        m_Log->Printf(1, "publish %s=%s", i->first.c_str(), i->second.c_str());
    }
    m_Log->Printf(1, "Updated");


    sleep(3);
}

void MqttTest()
{

    TestMqttConnection test;
    test.Test();

}
