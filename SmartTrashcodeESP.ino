

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <EEPROM.h>

#include <SoftwareSerial.h>
uint32_t last_check = 0;
uint32_t lastRead = 0;
uint32_t lastSendData = 0;

ESP8266WiFiMulti wifiMulti;
SocketIOclient socketIO;

SoftwareSerial mySerial(D4, D3);  // Tạo giao diện UART ảo với chân RX là D5, chân TX là D6



void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      Serial.printf("[IOc] Connected to url: %s\n", payload);

      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      Serial.printf("[IOc] get event: %s\n", payload);
      break;
    case sIOtype_ACK:
      Serial.printf("[IOc] get ack: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      Serial.printf("[IOc] get error: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      Serial.printf("[IOc] get binary: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      Serial.printf("[IOc] get binary ack: %u\n", length);
      hexdump(payload, length);
      break;
  }
}

void setup() {
  Serial.begin(9600);     // Baud rate của UART với Serial Monitor
  mySerial.begin(57600);  // Baud rate của UART
  Serial.println();

  if (WiFi.getMode() & WIFI_AP) {
    WiFi.softAPdisconnect(true);
  }
  wifiMulti.addAP("American Study", "66668888");
  wifiMulti.addAP("thungracthongminh", "66668888");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //ket noi server
  //  socketIO.begin("http://localhost:3002",3002, /socket.io/?EIO=4);  // /socket.io/?EIO=4
  socketIO.begin("smtrash.com", 80, "/socket.io/?EIO=4");  // /socket.io/?EIO=4
  socketIO.onEvent(socketIOEvent);
}

bool trangthaiphunsuong, trangthaikhoa, trangthaithungrac;
float MQ135_ppm, MQ2_ppm, test;

void loop() {
  socketIO.loop();
  if (mySerial.available() >= sizeof(float) * 5) { // Đọc dữ liệu khi có đủ dữ liệu cho 5 phần tử float ////
  float data[6];
  mySerial.readBytes((byte *)data, sizeof(data));  // Đọc dữ liệu
  Serial.print("Received data: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
    socketIO.loop();
  }
  Serial.println();
  MQ135_ppm = data[1];
  MQ2_ppm = 0;
  trangthaiphunsuong = data[2];
  trangthaikhoa = data[3];
  trangthaithungrac = data[4];
  test = data[0];
  socketIO.loop();
  Serial.print("MQ135:");
  Serial.println(MQ135_ppm);
  Serial.print("MQ2:");
  Serial.println(MQ2_ppm);
  socketIO.loop();
  Serial.print("trangthaiphunsuong:");
  Serial.println(trangthaiphunsuong);
  Serial.print("trangthaikhoa:");
  Serial.println(trangthaikhoa);
  socketIO.loop();
  Serial.print("trangthaithungrac:");
  Serial.println(trangthaithungrac);
  socketIO.loop();
  if (millis() - lastSendData > 2000) {
    sendData();
    Serial.println("upload done ");
    socketIO.loop();
    lastSendData = millis();
  }


  }
}

void sendData() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  array.add("message");
  JsonObject param1 = array.createNestedObject();

  param1["MQ135"] = String(MQ135_ppm);
  param1["MQ2"] = String(MQ2_ppm);
  param1["phunsuong"] = String(trangthaiphunsuong);
  param1["khoa"] = String(trangthaikhoa);
  param1["thungrac"] = String(trangthaithungrac);

  String output;
  serializeJson(doc, output);
  socketIO.sendEVENT(output);
  Serial.println(output);
  delay(20);
}