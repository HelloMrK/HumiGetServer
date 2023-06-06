#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

const char* ssid = "xxx"; // 替换为你的WiFi名称
const char* password = "xxxxxxxx"; // 替换为你的WiFi密码
const int SERVER_PORT = 80; // 定义服务器端口常量
const char* NTP_SERVER = "pool.ntp.org"; // 定义NTP服务器地址常量
ESP8266WebServer server(SERVER_PORT); // 创建一个Web服务器对象，监听端口80
int ASignal = A0;

WiFiUDP udp;
NTPClient timeClient(udp, NTP_SERVER);

// 读取土壤湿度
float readSoilMoisture() {
  int sensorValue = analogRead(A0);
  float percentageValue = 100 - ((float)sensorValue / 1024.0) * 100.0;
  double formattedPercentageValue = round(percentageValue * 100.0) / 100.0;
  return formattedPercentageValue;
}

// 读取当前时间
String getCurrentTime() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *tm;
  time_t t = epochTime;
  tm = localtime(&t);
  tm->tm_hour += 8;
  mktime(tm);
  char formattedTime[20];
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", tm);
  return String(formattedTime);
}

// 组装响应
String createResponse(float soilMoisture, const String& currentTime) {
  StaticJsonDocument<200> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
  json["sensorValue"] = soilMoisture;
  json["percentageValue"] = soilMoisture;
  json["currentTime"] = currentTime;

  String jsonString;
  serializeJson(json, jsonString);
  return jsonString;
}

// 处理根路径请求
void handleRoot() {
  float soilMoisture = readSoilMoisture();
  String currentTime = getCurrentTime();
  String response = createResponse(soilMoisture, currentTime);

  server.send(200, "application/json", response);
}

// 连接到WiFi网络
void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("当前IP地址: ");
  Serial.println(WiFi.localIP());
}

// 初始化mDNS服务
void initializeMDNS() {
  if (MDNS.begin("esphumisensor")) {
    Serial.println("MDNS responder started");
  }
}

// 初始化NTP客户端
void initializeNTPClient() {
  timeClient.begin();
}

// 设置静态IP地址
void setStaticIP() {
  IPAddress staticIP(192, 168, 1, 100);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(staticIP, gateway, subnet);
}

// 开始Web服务器
void startWebServer() {
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);

  connectToWiFi();

  // 设置静态IP地址（可选）
  // setStaticIP();
  
  initializeMDNS();

  pinMode(ASignal, INPUT);

  initializeNTPClient();

  startWebServer();
}

void loop() {
  server.handleClient();
}
