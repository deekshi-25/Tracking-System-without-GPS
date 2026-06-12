#include <ESP8266WiFi.h>
#include <WiFiManager.h>   // NEW
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= RFID =================
#define SS_PIN 15
#define RST_PIN 0
MFRC522 rfid(SS_PIN, RST_PIN);

// ================= HALL SENSOR =================
#define HALL_PIN 4

volatile int pulseCount = 0;
unsigned long lastPulseTime = 0;

float radius = 0.03;

// ================= RFID UID =================
String CP1 = "83f9c5f";
String CP2 = "936e10e";

bool cp1_done = false;
bool cp2_done = false;

// ========== INTERRUPT ==========
void ICACHE_RAM_ATTR countPulse() {
  unsigned long now = millis();

  if (now - lastPulseTime > 50) {
    pulseCount++;
    lastPulseTime = now;
  }
}

// ========== MQTT RECONNECT ==========
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266_TRACK")) {
      Serial.println("MQTT Connected");
    } else {
      delay(1000);
    }
  }
}

// ========== SEND ==========
void sendCheckpoint(String cp, float dist) {
  String msg = "{";
  msg += "\"cp\":\"" + cp + "\",";
  msg += "\"dist\":" + String(dist);
  msg += "}";

  client.publish("iot/vehicle/data", msg.c_str());
}

void sendLive(float dist) {
  client.publish("iot/vehicle/live", String(dist).c_str());
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  // ===== WIFI MANAGER START =====
  WiFiManager wm;

  bool res = wm.autoConnect("ESP8266_TRACK_SETUP");

  if (!res) {
    Serial.println("Failed to connect WiFi");
    ESP.restart();
  }

  Serial.println("WiFi Connected ✅");
  // ===== WIFI DONE =====

  client.setServer(mqtt_server, 1883);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(HALL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), countPulse, FALLING);
}

// ========== LOOP ==========
void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  float distance = pulseCount * (2 * 3.14 * radius);

  // ===== RFID =====
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    String uid = "";

    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }

    Serial.println(uid);

    if (uid == CP1 && !cp1_done) {
      sendCheckpoint("CP1", distance);
      cp1_done = true;
      pulseCount = 0;
    }

    else if (uid == CP2 && cp1_done && !cp2_done) {
      sendCheckpoint("CP2", distance);
      cp2_done = true;
      pulseCount = 0;
    }

    delay(1000);
  }

  // ===== LIVE =====
  sendLive(distance);

  delay(300);
}