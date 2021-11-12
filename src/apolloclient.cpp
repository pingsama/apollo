#include "apolloclient.h"
#include "pthread.h"
#include <sstream>
#include <iostream>
#include <string>
#include <thread>
#include <stdlib.h>
#include "json/json.h"
//#include "glog/logging.h"
#include "algorithm"
#include <chrono>
#include <iomanip>
#include "signature.h"
#include <regex>


std::string GetTimeString()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::stringstream ss;
    ss << ms.count();
    return ss.str();
}

std::string url2PathWithQuery(std::string url)
{
    // + http://
    std::string path = url.substr(7);
    auto pos = path.find_first_of("/");
    path = path.substr(pos);
    //LOG(INFO) << "path "<< path;
    return path; 
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string* str = dynamic_cast<std::string *>((std::string *)stream);
    str->append((char *)ptr, size * nmemb);
    return nmemb;
}


struct curl_slist * setCommonHeader(CURLcode* res, char* tokenHeader){
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, CONTENT_HEADER);
    if(tokenHeader) {
        headers = curl_slist_append(headers, tokenHeader);
    }
    return headers;
}


namespace apollo_client{

ApolloClient::ApolloClient(apollo_env env)
{
    apollo_env_ = env;
    //LOG(INFO) << "env: "<< env.meta << " "<< env.appId << " "<< env.clusterName << " "<< env.namespaceName;
}

ApolloClient::~ApolloClient()
{
    //LOG(INFO) << "ApolloClient destory";
    flag_ = false;
}

ApolloClient& ApolloClient::Instance(apollo_env env) {
    static std::shared_ptr<ApolloClient> s_instance(new ApolloClient(env));
    static ApolloClient &s_insteanc_ref = *s_instance;
    return s_insteanc_ref;
}

std::string ApolloClient::signature(std::string timestamp, std::string path)
{
    std::string strToSign = timestamp + "\n" + path;
    unsigned char *signedByte = (unsigned char *)malloc(64);
	memset(signedByte, 0, 64);
    unsigned int signedLen = 0;
    HmacEncode("SHA1", apollo_env_.secret, strToSign, signedByte, signedLen);
    std::string strSigned = base64_encode(signedByte,signedLen);
    //LOG(INFO) << " signed  "<< strSigned;
    return strSigned;
}

CURLcode ApolloClient::getNoCachePropertyString(std::string* resp){
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        if(apollo_env_.clusterName.empty()){
            apollo_env_.clusterName = DEFAULT_CLUSTER_NAME;
        }

        std::stringstream url;
        url << "http://";
        url <<  apollo_env_.meta;
        url << "/configs/";
        url << apollo_env_.appId;
        url << "/";
        url << apollo_env_.clusterName;
        url << "/";
        url << apollo_env_.namespaceName;

        std::string timeStr = GetTimeString();
        std::string path = url2PathWithQuery(url.str());
        std::string signedStr = signature(timeStr, path);
        std::string timestamp = "Timestamp:"+timeStr;
        std::string signHeade = "Authorization: Apollo " + apollo_env_.appId + ":" + signedStr;
        struct curl_slist *headers =setCommonHeader(&res, (char*)signHeade.c_str());
        headers = curl_slist_append(headers, timestamp.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)resp);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        //LOG(INFO) << "cur "<< url.str() << " resp "<< *resp;
        return res;
    }
}

void ApolloClient::jsonStrToProperties(std::string properStr, std::string childNodeName, Properties& properties)
{
    //LOG(INFO) << "jsonStrToProperties "<< properStr;
    Json::Reader reader;
    Json::Value root;
    if (properStr.empty())
    {
        return;
    }
    if (!reader.parse(properStr, root))
    {
        //LOG(INFO) << " parse properStr failed"; 
        return;
    }
    Json::Value::Members mem;
    mem = root.getMemberNames();
    Json::Value::Members::iterator it;
    //LOG(INFO) << " parse successs "; 

    for (it = mem.begin(); it != mem.end(); it++)
    {
        if(childNodeName.empty())
        {
            std::string val = root[*it].toStyledString();
            val = std::regex_replace(val, std::regex("\\\\n"), "");
            val = std::regex_replace(val, std::regex("\\\\t"), "");
            val = std::regex_replace(val, std::regex("\\\\"), "");
            val = std::regex_replace(val, std::regex("^\\\"+"), "");
            val = std::regex_replace(val, std::regex("\\\"+\\\n$"), "");
            properties.emplace(*it, val);
            //LOG(INFO)<< "key: "<< *it << " type:"<< root[*it].type() << " "<< " val "<< val;
            continue;
        }

        if(childNodeName == *it)
        {
            //LOG(INFO)<< "find child key: "<< *it << " type:"<< root[*it].type();
            jsonStrToProperties( root[*it].toStyledString(), "", properties);
            return;
        }

        //LOG(INFO)<< "abandon key: "<< *it << " type:"<< root[*it].type();
    }
}

void ApolloClient::getNoCacheProperty(Properties& properties){
    std::string resp;
    //LOG(INFO) << "getNoCacheProperty " << apollo_env_.meta << " "<< apollo_env_.appId << " "<< apollo_env_.secret;
    CURLcode code = getNoCachePropertyString(&resp);
    jsonStrToProperties(resp ,"configurations", properties);
}

CURLcode ApolloClient::checkNotifications(long* response_code, std::string* resp){
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        if(apollo_env_.clusterName.empty()){
            apollo_env_.clusterName=DEFAULT_CLUSTER_NAME;
        }
        std::stringstream notify;
        notify << "[";
        notify << "{";
        notify << "\"namespaceName\": \"";
        notify << apollo_env_.namespaceName;
        notify << "\",";
        notify << "\"notificationId\": \"";
        notify << notificationId_;
        notify << "\"";
        notify << "}";
        notify << "]";
    
        char* code_notify = curl_escape(notify.str().c_str(),notify.str().length());
        std::stringstream url;
        url << "http://";
        url <<  apollo_env_.meta;
        url << "/notifications/v2?appId=";
        url << apollo_env_.appId;
        url << "&cluster=";
        url << apollo_env_.clusterName;
        url << "&notifications=";
        url << code_notify;


        std::string timeStr = GetTimeString();
        std::string path = url2PathWithQuery(url.str());
        std::string signedStr = signature(timeStr, path);
        std::string timestamp = "Timestamp:"+timeStr;
        std::string signHeade = "Authorization: Apollo " + apollo_env_.appId + ":" + signedStr;
        struct curl_slist *headers =setCommonHeader(&res, (char*)signHeade.c_str());
        headers = curl_slist_append(headers, timestamp.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)resp);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
        }
        curl_easy_cleanup(curl);
        //LOG(INFO) << "check notify "<< res << " code: "<< *response_code << " resp:"<< *resp;
        return res;
    }
}

Config_changed ApolloClient::GetChangedConfig(Properties& old_properties, Properties& new_properties)
{
    Config_changed config_list;
    for(auto it : new_properties)
    {
        config_event event;
        event.key = it.first;
        event.new_value = it.second;
        auto pro = old_properties.find(it.first);
        if(pro == old_properties.end())
        {
            event.type = CONFIG_EVENT_ADD;
            event.old_value = "";
            config_list.emplace_back(event);
        }
        else
        {
            if(pro->second != it.second)
            {
                event.type = CONFIG_EVENT_MOD;
                event.old_value = pro->second;
                config_list.emplace_back(event);
            }
        }
    }

    for(auto it : old_properties)
    {
        auto pro = new_properties.find(it.first);
        if(pro == new_properties.end())
        {
            config_event event;
            event.type = CONFIG_EVENT_DEL;
            event.old_value = it.second;
            event.new_value = "";
            config_list.emplace_back(event);
        }
    }
    return config_list;
}

void ApolloClient::submitNotifications( void (*callback)(Config_changed& changed_cfg))
{
    //第一次获取资源信息。
    Properties oldProperties;
    Properties tmpProperties;
    getNoCacheProperty(oldProperties);
    Config_changed list = GetChangedConfig(tmpProperties, oldProperties);
    callback(list);
    long responseCode;
    std::string resp;
    Properties newProperties;
    CURLcode res=checkNotifications(&responseCode, &resp);
    //根据动态flag决定是否需要继续long pooling
    while(flag_) {
        if (res == CURLE_OK) {
            //304直接使用当前数据直接调用当前方法继续递归
            if (responseCode == 304) {
                res=checkNotifications(&responseCode, &resp);
                continue;
            } else {
                //200不管是否都需要获取最新的配置项
                if (responseCode == 200) {
                    //获取最新的配置。
                    //LOG(INFO) << "get notify 200 ok";
                    newProperties.clear();
                    getNoCacheProperty(newProperties);
                    Json::Reader reader;
                    Json::Value root;
                    if (!reader.parse(resp, root))
                    {
                        //LOG(INFO) << "get notify parse failed ";
                        continue;
                    }
                    int notificationId = root[0]["notificationId"].asInt();
                    //LOG(INFO) << "get notify id " <<  notificationId << " " << notificationId_;

                    if(notificationId_>NOTIFICATION_ID_PLACEHOLDER){
                        list.clear();
                        list = GetChangedConfig(oldProperties, newProperties);
                        callback(list);
                    }
                    notificationId_ = notificationId;
                    oldProperties = newProperties;

                    resp.clear();
                    res=checkNotifications(&responseCode, &resp);
                }
            }

        }
    }
}

void ApolloClient::submitNotificationsAsync(void (*callback)(Config_changed& changed_cfg))
{
    flag_ = true;
    std::thread notify_trd(&ApolloClient::submitNotifications,this, callback);
    notify_trd.detach();
}

}