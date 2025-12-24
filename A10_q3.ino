#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define WIFI_SSID       "SUNBEAM"
#define WIFI_PASSWORD   "1234567890"

#define MQTT_BROKER     "broker.hivemq.com" 
#define MQTT_PORT       1883
#define MQTT_USER       ""  
#define MQTT_PASSWORD   ""  

#define MQTT_TOPIC_TEMP "esp32/temperature"
#define MQTT_TOPIC_HUM  "esp32/humidity"

#define DHTPIN  4 
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi();
void connectMQTT();
void publishData();

void setup() {
  Serial.begin(115200);
  dht.begin();

  connectWiFi();
  client.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  publishData();
  delay(5000); 
}

void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startAttemptTime > 15000) {
      Serial.println("\nFailed to connect to WiFi. Restarting...");
      ESP.restart();
    }
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (client.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void publishData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char tempStr[8];
  dtostrf(temperature, 1, 2, tempStr);
  client.publish(MQTT_TOPIC_TEMP, tempStr);

  char humStr[8];
  dtostrf(humidity, 1, 2, humStr);
  client.publish(MQTT_TOPIC_HUM, humStr);

  Serial.printf("Published -> Temp: %s°C, Hum: %s%%\n", tempStr, humStr);
}
