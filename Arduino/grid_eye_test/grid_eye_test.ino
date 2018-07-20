/*
 *   
 *   Pin configuration
 *  ___________________
 * | vl53lox | Arduino | 
 * |_________|_________|
 * |   Vcc   |   5V    |     
 * |   SCL   |   D1    |
 * |   SDA   |   D2    | 
 * |   Gnd   |   GND   |
 * |   XSHUT |   3     |
 * |_________|_________|
 *  ___________________
 * | vl53lox | Arduino | 
 * |_________|_________|
 * |   Vcc   |   5V    |     
 * |   SCL   |   A5    |
 * |   SDA   |   A4    | 
 * |   Gnd   |   GND   |
 * |   XSHUT |   4     |
 * |_________|_________|
 * 
 * Short all the wires except XSHUT for all S1s
 */

#include "Fsm.h"
#include <Regexp.h>
#include <Wire.h>
#include <VL53L0X.h>

#include <Adafruit_AMG88xx.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

using namespace std;

const char *ssid = "SEIL";
const char *password = "deadlock123";

const char *mqtt_server = "10.129.149.9";
//const char* mqtt_server = "10.129.28.158";
const char *mqtt_username = "<MQTT_BROKER_USERNAME>";
const char *mqtt_password = "<MQTT_BROKER_PASSWORD>";

const char *mqtt_topic = "test/nodemcu/grid_eye_data";
const char *mqtt_topic_publish = "nodemcu/pub";
const char *client_id = "grid_eye_data";

Adafruit_AMG88xx amg;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t pixels[AMG88xx_PIXEL_ARRAY_SIZE];
//uint8_t* pixels;

char readGridEye = 'f';

vector<byte *> grid_eye_vector;

int row_count = 0;
int col_count = 0;

bool published = false;
bool recorded = false;

int record_count = 0;

int byteIndex = 0;

byte byteArray[100][64];

#define LIGHTS_RELAY D5
#define FANS_RELAY D6
#define S1_XSHUT D4
#define S2_XSHUT D3
#define HIGH_SPEED

//lox variables
#define DOOR_WIDTH 800
VL53L0X S1;
VL53L0X S2;

float reading1, reading2;
int occupancy_counter = 0;
String transition_string = "";

//State machine variables
enum Event
{
  U1_TRIGGERED,
  U1_HALTED,
  U2_TRIGGERED,
  U2_HALTED
};
State q0(&on_q0_enter, NULL, NULL);
State q1(&on_q1_enter, NULL, NULL);
State q2(&on_q2_enter, NULL, NULL);
State q3(&on_q3_enter, NULL, NULL);
Fsm fsm(&q0);
MatchState ms;
char result;
char buf[100] = {0};

//State machine callbacks
void on_q0_enter()
{
  Serial.println("Entering state Q0");
  transition_string.toCharArray(buf, 100);
  ms.Target(buf);
  //Test for entry
  result = ms.Match("0.*1.*3.*2.*", 0);
  if (result == REGEXP_MATCHED)
  {
    Serial.println("ENTRY");
    occupancy_counter++;
    grid_eye_publish();
  }
  //Test for exit
  result = ms.Match("0.*2.*3.*1.*", 0);
  if (result == REGEXP_MATCHED && occupancy_counter > 0)
  {
    Serial.println("EXIT");
    occupancy_counter--;
    grid_eye_publish();
  }
  Serial.print("Occupancy counter:");
  Serial.println(occupancy_counter);
  transition_string = "0";
}
void on_q1_enter()
{
  Serial.println("Entering state Q1");
  transition_string += "1";
  if (!recorded)
    grid_eye_read();
}
void on_q2_enter()
{
  Serial.println("Entering state Q2");
  transition_string += "2";
  if (!recorded)
    grid_eye_read();
}
void on_q3_enter()
{
  Serial.println("Entering state Q3");
  transition_string += "3";
}

void grid_eye_read()
{
  amg.readPixels(pixels, (uint8_t)AMG88xx_PIXEL_ARRAY_SIZE);

  String mqtt_data_string = "";

  for (int i = 0; i < AMG88xx_PIXEL_ARRAY_SIZE; i++)
  {
    mqtt_data_string += String(pixels[i]) + ",";
  }
  Serial.println();

  char mqtt_data[mqtt_data_string.length()];

  mqtt_data_string.toCharArray(mqtt_data, mqtt_data_string.length());
  //  mqtt_data_string.toCharArray(mqtt_data, 80);

  memcpy(byteArray[byteIndex], pixels, 64);

  //    byte* byteArray[byteIndex];
  //    byteArray[byteIndex] = (byte*)pixels;

  //    grid_eye_vector.push_back(byteArray[byteIndex]);

  unsigned int arraySize = sizeof(byteArray[byteIndex]);
  bool published = false;

  Serial.print("Recording Data with size ");
  Serial.print(sizeof(byteArray));
  Serial.print(" : ");
  Serial.println(mqtt_data_string.length());

  Serial.print("[");
  Serial.print(grid_eye_vector.size());
  Serial.println("]");
  //    published  = client.publish(mqtt_topic, byteArray, sizeof(pixels));

  //    if (published) Serial.println("Data Published");
  //    else Serial.println("Unable to publish the data");

  //  Serial.println(mqtt_data);
  //delay a 100 mili-second
  recorded = true;
  record_count++;
  byteIndex++;
}

void grid_eye_publish()
{

  Serial.println("-------------------------");
  Serial.print("Number of records: ");
  Serial.println(record_count);
  Serial.println("-------------------------");
  delay(500);

  int valid_count = 0;
  Serial.println("Sending Data over mqtt");

  //      for (int vector_index = 0; vector_index < grid_eye_vector.size(); vector_index++)
  for (int i = 0; i < record_count; i++)
  {
    //        published  = client.publish(mqtt_topic, grid_eye_vector[vector_index], sizeof(pixels));
    published = client.publish(mqtt_topic, byteArray[i], sizeof(pixels));
    if (published)
      Serial.println("Published");
    else
      Serial.println("Unable to publish");

    valid_count++;
  }

  if (record_count == valid_count)
    Serial.println("BullsEye Behbee");
  else
    Serial.println("Some serious issues with count");

  delay(500);
  record_count = 0;
  recorded = false;
  grid_eye_vector.clear();
  byteIndex = 0;
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

void setup()
{
  Serial.begin(115200);
  //  Serial.println(F("AMG88xx pixels"));

  setupWiFi();

  client.setServer(mqtt_server, 1883);

  bool status;

  // default settings
  status = amg.begin(0x68);
  if (!status)
  {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1)
      ;
  }

  amg.write8(AMG88xx_FPSC, AMG88xx_FPS_10);
  amg.setMovingAverageMode(false);
  //  Serial.println("-- Pixels Test --");

  //  Serial.println();

  delay(100); // let sensor boot up

  pinMode(S1_XSHUT, OUTPUT);
  digitalWrite(S1_XSHUT, LOW);

  delay(10);

  pinMode(S2_XSHUT, OUTPUT);
  digitalWrite(S2_XSHUT, LOW);

  Wire.begin();

  digitalWrite(S1_XSHUT, HIGH);
  delay(10);
  S1.init();
  S1.setTimeout(200);
  S1.setAddress((uint8_t)48);
  delay(10);

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  S1.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  S1.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  S1.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  S1.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  S1.setMeasurementTimingBudget(200000);
#endif

  digitalWrite(S2_XSHUT, HIGH);
  delay(10);
  S2.init();
  S2.setTimeout(200);
  S2.setAddress((uint8_t)49);
  delay(10);

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  S2.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  S2.setMeasurementTimingBudget(200000);
#endif
  // power
  Serial.println(F("SEIL occupancy counter using VL53lox rangefinder\n\n"));
  fsm.add_transition(&q0, &q1, U1_TRIGGERED, NULL);
  fsm.add_transition(&q1, &q3, U2_TRIGGERED, NULL);
  fsm.add_transition(&q3, &q2, U1_HALTED, NULL);
  fsm.add_transition(&q2, &q0, U2_HALTED, NULL);
  fsm.add_transition(&q0, &q2, U2_TRIGGERED, NULL);
  fsm.add_transition(&q2, &q3, U1_TRIGGERED, NULL);
  fsm.add_transition(&q3, &q1, U2_HALTED, NULL);
  fsm.add_transition(&q1, &q0, U1_HALTED, NULL);
}

void loop()
{

  if (!client.connected())
    reconnect();
  client.loop();

  // while (Serial.available() > 0)
  // {
  //   readGridEye = Serial.read();
  // }

  // if (readGridEye == 't')
  // {
  //   grid_eye_read();
  // }
  // else
  // {
  //   Serial.println("Stop Reading[put 't' for the read");
  //   if (recorded)
  //   {
  //     grid_eye_publish();
  //   }
  // }
  // delay(100);

  //    Serial.println("----------/--------------------------------------------------");
  reading1 = S1.readRangeSingleMillimeters();   // Reading
  //    Serial.print("Distance from S1 1 : ");/
  //    Serial.print(reading1);/
  if (S1.timeoutOccurred())
  {
    Serial.print(" TIMEOUT");
  }
  //    Serial.println();/

  //    Serial.print("Distan/ce from S1 2 : ");
  reading2 = S2.readRangeSingleMillimeters();
  //    Serial.print(reading2);/
  if (S2.timeoutOccurred())
  {
    Serial.print(" TIMEOUT");
  }
  //    Serial.println();/
  //    Serial.println("----/--------------------------------------------------------");
  //    Serial.println();/
  if (reading1 == 0 || reading2 == 0)
    return;
  if (reading1 < DOOR_WIDTH)
    fsm.trigger(U1_TRIGGERED);
  else
    fsm.trigger(U1_HALTED);

  if (reading2 < DOOR_WIDTH)
    fsm.trigger(U2_TRIGGERED);
  else
    fsm.trigger(U2_HALTED);
  fsm.run_machine();
  //    Serial.print("S1 1 | 2 Reading :");
  //    Serial.print(reading1);
  //    Serial.print(" | ");
  //    Serial.print(reading2);
  //    Serial.println(" CM");
  delay(100);
}
