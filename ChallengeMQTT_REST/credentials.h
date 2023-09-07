class credentials
{
private:
    /* data */
public:
    const char* ssid = "MERCUSYS98DC";
    const char* password =  "plaztilina2023";
    const char* mqttServer = "192.168.1.102";
    const int mqttPort = 1883;
    const char* mqttUser = "";
    const char* mqttPassword = "";
    const char* TopicSub = "input";
    const char* TopicPub1 = "alive";
    const char* TopicPub2 = "StatusRequest";
    const char* TopicPub3 = "JsonStatus";
    const char* TopicPub4 = "Output";
    ~credentials();
};