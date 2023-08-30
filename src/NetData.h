#ifndef __NETDATA_H
#define __NETDATA_H

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <string>

class NetChartData
{
public:
    int api;
    String id;
    String name;

    int view_update_every;
    int update_every;
    long first_entry;
    long last_entry;
    long before;
    long after;
    String group;
    String options_0;
    String options_1;

    JsonArray dimension_names;
    JsonArray dimension_ids;
    JsonArray latest_values;
    JsonArray view_latest_values;
    int dimensions;
    int points;
    String format;
    JsonArray result;
    double min;
    double max;
};

void parseNetDataResponse(WiFiClient &client, NetChartData &data)
{
    // Stream& input;

    DynamicJsonDocument doc(2048);

    DeserializationError error = deserializeJson(doc, client);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    data.api = doc["api"]; // 1
    data.id = doc["id"].as<String>();
    data.name = doc["name"].as<String>();
    data.view_update_every = doc["view_update_every"]; // 1
    data.update_every = doc["update_every"];           // 1
    data.first_entry = doc["first_entry"];             // 1691505357
    data.last_entry = doc["last_entry"];               // 1691505905
    data.after = doc["after"];                         // 1691505903
    data.before = doc["before"];                       // 1691505904
    data.group = doc["group"].as<String>();            // "average"

    data.options_0 = doc["options"][0].as<String>(); // "jsonwrap"
    data.options_1 = doc["options"][1].as<String>(); // "natural-points"

    data.dimension_names = doc["dimension_names"];
    data.dimension_ids = doc["dimension_ids"];
    data.latest_values = doc["latest_values"];
    data.view_latest_values = doc["view_latest_values"];

    data.dimensions = doc["dimensions"];      // 3
    data.points = doc["points"];              // 2
    data.format = doc["format"].as<String>(); // "array"

    JsonArray db_points_per_tier = doc["db_points_per_tier"];
    data.result = doc["result"];
    data.min = doc["min"]; // 2.1594684
    data.max = doc["max"]; // 3.7468776
}

/**
 * 从软路由NetData获取监控信息
 * ChartID:
 *  system.cpu - CPU占用率信息
 *  sensors.temp_thermal_zone0_thermal_thermal_zone - CPU 温度信息
 */

bool getNetDataInfoWithDimension(String chartID, NetChartData &data, String dimensions_filter)
{
    const char *NETDATA_HOST = "192.168.31.66";
    int NETDATA_PORT = 19999;
    // String reqRes = "/api/v0/data?chart=sensors.temp_thermal_zone0_thermal_thermal_zone0&format=json&points=9&group=average&gtime=0&options=s%7Cjsonwrap%7Cnonzero&after=-10";
    String reqRes = "/api/v1/data?chart=" + chartID + "&format=array&points=9&group=average&gtime=0&options=s%7Cjsonwrap%7Cnonzero&after=-2";
    reqRes = reqRes + "&dimensions=" + dimensions_filter;

    WiFiClient client;
    bool ret = false;

    // 建立http请求信息
    String httpRequest = String("GET ") + reqRes + " HTTP/0.1\r\n" + "Host: " + NETDATA_HOST + "\r\n" + "Connection: close\r\n\r\n";

    // 尝试连接服务器
    if (client.connect(NETDATA_HOST, NETDATA_PORT))
    {
        // 向服务器发送http请求信息
        client.print(httpRequest);
        Serial.println("Sending request: ");
        Serial.println(httpRequest);

        // 获取并显示服务器响应状态行
        String status_response = client.readStringUntil('\n');
        Serial.print("status_response: ");
        Serial.println(status_response);
        // 使用find跳过HTTP响应头
        if (client.find("\r\n\r\n"))
        {
            Serial.println("Found Header End. Start Parsing.");
        }

        // 利用ArduinoJson库解析NetData返回的信息
        parseNetDataResponse(client, data);
        ret = true;
    }
    else
    {
        Serial.println(" connection failed!");
    }
    // 断开客户端与服务器连接工作
    client.stop();
    return ret;
}

bool getNetDataInfo(String chartID, NetChartData &data)
{
    return getNetDataInfoWithDimension(chartID, data, "");
}

#endif
