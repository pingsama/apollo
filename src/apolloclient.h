#pragma onece
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <curl/curl.h>
#include "json/json.h"
#include "apollo.h"

#define CONTENT_HEADER "Content-Type:application/json;charset=UTF-8"
#define DEFAULT_CLUSTER_NAME "default"

/**
 * 不带缓存配置信息url地址
 */
#define APOLLO_CONFIG_NOCACHE_URL "http://%s/configs/%s/%s/%s"
/**
 * http long pooling对应url地址
 */
#define APOLLO_CONFIG_NOTI_URL "http://%s/notifications/v2?appId=%s&cluster=%s&notifications=%s"

#define NOTIFICATION_ID_PLACEHOLDER -1

#define AUTHORIZATION_FORMAT  "Apollo %s:%s";
#define DELIMITER  "\n";

#define HTTP_HEADER_AUTHORIZATION  "Authorization";
#define HTTP_HEADER_TIMESTAMP  "Timestamp";

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
struct curl_slist * setCommonHeader(CURLcode* res, char* tokenHeader);
