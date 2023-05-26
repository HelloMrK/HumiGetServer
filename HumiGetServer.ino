#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// 替换为您的网络凭据和静态IP地址
const char* ssid = "1202"; // 你的WiFi名称，中文需要转换为UTF-8编码
const char* password = "1234567890"; // 你的WiFi密码
//IPAddress staticIP(192, 168, 43, 222); // 静态IP地址
//IPAddress gateway(192, 168, 43, 1); // 网关
//IPAddress subnet(255, 255, 255, 0); // 子网掩码

ESP8266WebServer server(80); // 创建一个Web服务器对象，监听端口80
int ASignal = A0;

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org");

void handleRoot() {
  int sensorValue = analogRead(A0);
  float percentageValue = 100 - ((float)sensorValue / 1024.0) * 100.0;

  // 将 percentageValue 保留两位小数
  double formattedPercentageValue = round(percentageValue * 100.0) / 100.0;

  // 创建一个 JSON 缓冲区
  StaticJsonDocument<200> jsonBuffer;
  
  // 创建一个空的 JSON 对象
  JsonObject json = jsonBuffer.to<JsonObject>();
  
  // 将 sensorValue 和 percentageValue 添加到 JSON 对象中
  json["sensorValue"] = sensorValue;
  json["percentageValue"] = formattedPercentageValue;
  
  // 获取当前时间
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();

  // 将时间戳转换为时间结构体
  struct tm *tm;
  time_t t = epochTime;
  tm = localtime(&t);

  // 手动调整时区偏移量为+8时区（东八区）
  tm->tm_hour += 8;
  mktime(tm);

  // 创建一个字符数组存储格式化后的时间
  char formattedTime[20];
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", tm);

  // 将格式化后的时间添加到JSON对象中
  json["currentTime"] = formattedTime;
  
  // 将 JSON 对象转换为字符串
  String jsonString;
  serializeJson(json, jsonString);
  
  server.send(200, "application/json", jsonString); // 响应请求，并设置内容类型为JSON
}



void setup() {
  Serial.begin(115200);
  
  // 连接到Wi-Fi网络
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  // 设置静态IP地址
//  WiFi.config(staticIP, gateway, subnet);
  
  // 初始化mDNS服务
  if (MDNS.begin("esphumisensor")) {
    Serial.println("MDNS responder started");
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("当前IP地址: ");
  Serial.println(WiFi.localIP());

  // 处理根路径请求
  server.on("/", handleRoot);
  
  // 开始服务器
  server.begin();

  // 初始化NTP客户端
  timeClient.begin();
  
  Serial.println("HTTP server started");
  pinMode(ASignal, INPUT);       
}

void loop() {
  // 处理客户端请求
  server.handleClient();
}
