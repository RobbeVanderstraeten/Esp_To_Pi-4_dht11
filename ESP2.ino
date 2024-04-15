#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "*****"
#define BLYNK_TEMPLATE_NAME "*******"

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <EEPROM.h>

const char* ssid = "**********";
const char* password = "*********";
const char* mqttServer = "192.***.*.***";
const int mqttPort = 1883;
const char* mqttUser = "*****";
const char* mqttPassword = "*****";
const char* clientID = "******";  // MQTT client ID
char auth[] = "**************";

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

BlynkTimer timer;

void sendSensor();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  pinMode(26, OUTPUT);  // ROOD = Verwarming
  pinMode(27, OUTPUT);  // BLAUW = Airco
  pinMode(25, OUTPUT);  // ORANJE = connectie
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.subscribe("raspi/#");
  Blynk.begin(auth, ssid, password, "*******", ****);
  timer.setInterval(1000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();

  client.loop();

  float t = dht.readTemperature();

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    digitalWrite(25, HIGH);
    delay(500);
    digitalWrite(25, LOW);
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      digitalWrite(25, HIGH);
    }
  }
  String payload = String(t);
  client.publish("esp32/kamer2/temp", payload.c_str());

  if (digitalRead(27) == HIGH) {
    client.publish("esp32/kamer1/set", "airco2-AAN");
    Serial.println("A2");
  }

  if (digitalRead(26) == HIGH) {
    client.publish("esp32/kamer1/set", "verwarming2-AAN");
    Serial.println("V2");
  }

  delay(250);

  client.disconnect();  // disconnect from the MQTT broker
}

void sendSensor() {
  float t = dht.readTemperature();
  Blynk.virtualWrite(V1, t);
}
BLYNK_WRITE(V3) {
  int getal = param.asInt();

  if (dht.readTemperature() < getal - 0.5) {
    digitalWrite(26, HIGH);
  } else if (dht.readTemperature() > getal + 0.5) {
    digitalWrite(26, LOW);
  }
  if (dht.readTemperature() > getal - 0.5) {
    digitalWrite(27, HIGH);
  } else if (dht.readTemperature() < getal + 0.5) {
    digitalWrite(27, LOW);
  }
}