#include "mosquittopp.h"
#include "../libs/libutils/strutils.h"
#include "../libs/libwb/WBDevice.h"
#include "../libs/librf/RFProtocolNooLite.h"
#include "../libs/libutils/ConfigItem.h"

class RFM69OOK;

class CMqttConnection
    : public mosqpp::mosquittopp
{
    typedef CWBDevice::CWBDeviceMap CWBDeviceMap;
    string m_Server;
    CLog *m_Log;
    bool m_isConnected;
    CWBDeviceMap m_Devices;
    RFM69OOK *m_RFM;
    CRFProtocolNooLite m_nooLite;
    CConfigItem *m_devicesConfig;

    String lastMessage;
    size_t lastMessageCount, lastMessageNeedCount;

    time_t lastMessageReceiveTime;


  public:
    CMqttConnection(string Server, CLog *log, RFM69OOK *rfm, CConfigItem *devicesConfig = nullptr);
    ~CMqttConnection();
    void NewMessage(strutils::String message);

    // Should be called regularly to devices be checked for aliveness
    // and errors be sent if they are not alive
    void SendAliveness();
    void SendScheduledChanges();

  private:
    virtual void on_connect(int rc);
    virtual void on_disconnect(int rc);
    virtual void on_publish(int mid);
    virtual void on_message(const struct mosquitto_message *message);
    virtual void on_subscribe(int mid, int qos_count, const int *granted_qos);
    virtual void on_unsubscribe(int mid);
    virtual void on_log(int level, const char *str);
    virtual void on_error();

    void publishString(const string &path, const string &value);
    void publishStringMap(const CWBDevice::StringMap &values);

    void SendUpdate();
    void CreateDevice(CWBDevice *dev);
};
