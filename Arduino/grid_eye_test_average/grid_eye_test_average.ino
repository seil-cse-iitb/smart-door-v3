#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

using namespace std;

const char* ssid = "SEIL";
const char* password = "deadlock123";

const char* mqtt_server = "10.129.149.9";
//const char* mqtt_server = "10.129.28.158";
const char* mqtt_username = "<MQTT_BROKER_USERNAME>";
const char* mqtt_password = "<MQTT_BROKER_PASSWORD>";

const char* mqtt_topic = "test/nodemcu/grid_eye_data";
const char* mqtt_topic_publish = "nodemcu/pub";
const char* client_id = "grid_eye_data";

Adafruit_AMG88xx amg;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t pixels[AMG88xx_PIXEL_ARRAY_SIZE];
//uint8_t* pixels;

char readGridEye = 'f';

vector <byte*> grid_eye_vector;

int row_count = 0;
int col_count = 0;

bool published = false;
bool recorded = false;

int record_count = 0;

int byteIndex = 0;

byte byteArray[100][64];

byte* gridEyeData;

byte gridEyeRight[24] = {0};
byte gridEyeLeft[24] = {0};
byte gridEyeCenter[16] = {0};

unsigned int leftSum = 0;
unsigned int centerSum = 0;
unsigned int rightSum = 0;

byte* grid_eye_read()
{
  amg.readPixels(pixels, (uint8_t)AMG88xx_PIXEL_ARRAY_SIZE);

  return pixels;
}

void grid_eye_publish()
{
  for (int i = 0; i < record_count; i++)
  {
    //        published  = client.publish(mqtt_topic, grid_eye_vector[vector_index], sizeof(pixels));
    published = client.publish(mqtt_topic, byteArray[i], sizeof(pixels));
    if (published) Serial.println("Published");
    else Serial.println("Unable to publish");
  }
}

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

void setup() {
  Serial.begin(115200);
  //  Serial.println(F("AMG88xx pixels"));


  setupWiFi();

  client.setServer(mqtt_server, 1883);

  bool status;

  // default settings
  status = amg.begin(0x68);
  if (!status) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }

  amg.write8(AMG88xx_FPSC, AMG88xx_FPS_10);
  amg.setMovingAverageMode(false);
  //  Serial.println("-- Pixels Test --");

  //  Serial.println();

  delay(100); // let sensor boot up
}

void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  gridEyeData = grid_eye_read();

  // Serial.println("---------------------------");
  // Serial.println("Total Array");
  // Serial.print("[");
  // for (int index  = 1; index <= 64; index++) 
  // {
  //   Serial.print(gridEyeData[index-1]);
  //   if(index != 64) Serial.print(",");
  //   if(index % 8 == 0) Serial.println();
  // }
  // Serial.println("]");
  // Serial.println();
  // Serial.println("---------------------------");

  // Storing data in the respetive arrays 
  memcpy(gridEyeRight, pixels, 24);
  memcpy(gridEyeCenter, &(pixels[24]), 16);
  memcpy(gridEyeLeft, &(pixels[40]), 24);
  // std::copy(&gridEyeData + 24, &gridEyeData + 40, gridEyeCenter);
  // std::copy(&gridEyeData + 41, &gridEyeData + 64, gridEyeLeft);

  // Serial.println(gridEyeRight[1]);
  Serial.println("Right to grid eye");
  Serial.print("[");
  for(int i=1; i <= 24; i++)
  {
    Serial.print(gridEyeRight[i-1]);
    if (i != 24) Serial.print(",");

    rightSum += gridEyeRight[i-1];
  }
  Serial.print("] = ");
  Serial.println(rightSum);
  Serial.println();

  Serial.println("Center to grid eye");
  Serial.print("[");
  for(int i=1; i <= 16; i++)
  {
    Serial.print(gridEyeCenter[i-1]);
    if (i != 16) Serial.print(",");

    centerSum += gridEyeCenter[i -1];
  }
  Serial.print("] = ");
  Serial.println(centerSum);
  Serial.println();

  Serial.println("Left to grid eye");
  Serial.print("[");
  for(int i=1; i <= 24; i++)
  {
    Serial.print(gridEyeLeft[i-1]);
    if (i != 24) Serial.print(",");

    leftSum += gridEyeLeft[i-1];
  }
  Serial.print("] = ");
  Serial.println(leftSum);
  Serial.println();

  rightSum = 0;
  centerSum = 0;
  leftSum = 0;
  
  delay(100);
}
