#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// ====== USER CONFIGURATION ======
#define WIFI_SSID     "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define SERVER_URL    "http://your-server.com/api/data" // Change to your server endpoint

#define DHTPIN 4       // GPIO pin where DHT is connected
#define DHTTYPE DHT22  // Change to DHT11 if using that sensor

// ====== OBJECTS ======
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start DHT sensor
  dht.begin();

  // Connect to Wi-Fi
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n Failed to connect to WiFi");
  } else {
    Serial.println("\n Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float temperature = dht.readTemperature(); // Celsius
    float humidity = dht.readHumidity();

    // Validate sensor readings
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println(" Failed to read from DHT sensor!");
    } else {
      Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%\n", temperature, humidity);

      // Prepare JSON payload
      String jsonPayload = String("{\"temperature\":") + temperature +
                           ",\"humidity\":" + humidity + "}";

      // Send HTTP POST request
      HTTPClient http;
      http.begin(SERVER_URL);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
        Serial.printf(" POST Response code: %d\n", httpResponseCode);
        Serial.println("Response: " + http.getString());
      } else {
        Serial.printf(" POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
      }

      http.end();
    }
  } else {
    Serial.println(" WiFi disconnected, retrying...");
    WiFi.reconnect();
  }

  delay(10000); // Send every 10 seconds
}
