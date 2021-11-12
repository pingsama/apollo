#pragma onece
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <curl/curl.h>

namespace apollo_client{

typedef struct {
    std::string meta; //apollometa服务器ip和地址
    std::string appId;//应用名称,比如网关gateway
    std::string clusterName;//集群名称，默认default
    std::string namespaceName;//命名空间名称 如application.ymal
    std::string secret; //密钥
} apollo_env;

enum CONFIG_EVENT_TYPE {
    CONFIG_EVENT_ADD = 0,
    CONFIG_EVENT_MOD = 1,
    CONFIG_EVENT_DEL = 2,
};

typedef struct {
    CONFIG_EVENT_TYPE type;
    std::string key;
    std::string old_value;
    std::string new_value;
} config_event;

using Properties = std::map<std::string, std::string>;
using Config_changed = std::list<config_event>;

class ApolloClient
{
private:
    bool flag_ = true;
    int notificationId_ = -1;
    apollo_env apollo_env_;
    CURLcode getNoCachePropertyString(std::string* resp);
    CURLcode checkNotifications(long* response_code, std::string* resp);
    std::string signature(std::string timestamp, std::string path);
    Config_changed GetChangedConfig(Properties& old_properties, Properties& new_properties);
public:
    ApolloClient(apollo_env env);
    ~ApolloClient();

    static ApolloClient& Instance(apollo_env env);

    void jsonStrToProperties(std::string properStr, std::string childNodeName, Properties& properties);


    void submitNotifications(void (*callback)(Config_changed& changed_cfg));

    /**
    * listen config event, report all configs as CONFIG_EVENT_ADD at start time.
    */
    void submitNotificationsAsync(void (*callback)(Config_changed& changed_cfg));

    /*
    * get all config in apollo
    */
    void getNoCacheProperty(Properties& properties);
};

}