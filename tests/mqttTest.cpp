/* This test is a test of class CWBDevice and mosquittopp
 * It seems to be a manual test.
 * */


#include <unistd.h>

#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "mosquittopp.h"
#include "../libs/libwb/WBDevice.h"

using std::string;

class TestMqttConnection
    : public mosqpp::mosquittopp
{
    std::string m_Server, m_sName;
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
    : mosquittopp("TestMqttConnection"), m_isConnected(false)
{
    m_Server  = "192.168.1.20";

    connect(m_Server.c_str());
    loop_start();

}

TestMqttConnection::~TestMqttConnection()
{

}


void TestMqttConnection::on_connect(int rc)
{

    LOG(INFO) << m_sName << "::on_connect " << rc;

    if (!rc) {
        m_isConnected = true;
    }

}

void TestMqttConnection::on_disconnect(int rc)
{
    m_isConnected = false;
    LOG(INFO) << m_sName << "::on_disconnect " << rc;

}

void TestMqttConnection::on_publish(int mid)
{
    LOG(INFO) << m_sName << "::on_publish " << mid;

}

void TestMqttConnection::on_message(const struct mosquitto_message *message)
{
    LOG(INFO) << m_sName << "::on_message(" << message->topic << " = " << message->payload << ")";

}

void TestMqttConnection::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    LOG(INFO) << m_sName << "::on_subscribe " << mid;

}

void TestMqttConnection::on_unsubscribe(int mid)
{
    LOG(INFO) << m_sName << "::on_message " << mid;

}

void TestMqttConnection::on_log(int level, const char *str)
{
    LOG(INFO) << m_sName << "::on_log(" << level << ", " << str << ")";

}

void TestMqttConnection::on_error()
{
    LOG(INFO) <<  m_sName << "::on_error()";

}

void TestMqttConnection::Test()
{
    while (!m_isConnected)
        sleep(1);


    CWBDevice wbdev("TestDev", "Тестовое устройство");
    wbdev.addControl("Time", CWBControl::Text, true);
    wbdev.addControl("Random", CWBControl::Temperature, true);
    CWBDevice::StringMap v;

    wbdev.createDeviceValues(v);
    LOG(INFO) << "Create test device";

    for (auto i : v) {
        publish(NULL, i.first.c_str(), i.second.size(), i.second.c_str(), 0, true);
        LOG(INFO) << "publish " << i.first << " = " << i.second;
    }
    LOG(INFO) << "Created";


    char Buffer[50];
    long Time = time(NULL);
    strftime(Buffer, sizeof(Buffer), "%d/%m %H:%M:%S ", localtime(&Time));

    wbdev.set("Time", Buffer);
    wbdev.set("Random", rand() * 0.01);

    v.clear();
    wbdev.updateValues(v);
    LOG(INFO) << "Update test device";

    for (auto i : v) {
        while (!m_isConnected)
            usleep(10);

        publish(NULL, i.first.c_str(), i.second.size(), i.second.c_str(), 0, true);
        LOG(INFO) << "publish " << i.first << " = " << i.second;
    }
    LOG(INFO) << "Updated";


    sleep(3);
}

void MqttTest()
{

    TestMqttConnection test;
    test.Test();

}
