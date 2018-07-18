#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "SEIL";
const char* password = "deadlock123";

const char* mqtt_server = "10.129.149.9";
//const char* mqtt_server = "10.129.28.158";
const char* mqtt_username = "<MQTT_BROKER_USERNAME>";
const char* mqtt_password = "<MQTT_BROKER_PASSWORD>";

const char* mqtt_topic = "test/nodemcu/grid_eye_data";
const char* mqtt_topic_publish = "nodemcu/pub";
const char* client_id = "grid_eye_data";

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi with IP");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT...");

    if (client.connect(client_id))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("Failed to connect ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}