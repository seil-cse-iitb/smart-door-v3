
/*

     Pin configuration
    ___________________
   | vl53lox | Arduino |
   |_________|_________|
   |   Vcc   |   5V    |
   |   SCL   |   D1    |
   |   SDA   |   D2    |
   |   Gnd   |   GND   |
   |   XSHUT |   3     |
   |_________|_________|
    ___________________
   | vl53lox | Arduino |
   |_________|_________|
   |   Vcc   |   5V    |
   |   SCL   |   A5    |
   |   SDA   |   A4    |
   |   Gnd   |   GND   |
   |   XSHUT |   4     |
   |_________|_________|

   Short all the wires except XSHUT for all S1s
*/
#include "Fsm.h"
#include <Regexp.h>
#include <VL53L0X.h>
#include "grid_eye.h"
#include "mqtt.h"

#define LIGHTS_RELAY D5
#define FANS_RELAY D6
#define S1_XSHUT D4
#define S2_XSHUT D3
#define HIGH_SPEED

bool published = false;
bool recorded = false;
bool calibrated = false;

int record_count = 0;
int byteIndex = 0;

float finalLeftThreshold = 0.0;
float finalRightThreshold = 0.0;

byte byteArray[100][64];

byte* gridEyeData;

byte gridEyeRight[32] = {0};
byte gridEyeLeft[32] = {0};
byte gridEyeCenter[16] = {0};

unsigned int leftSum = 0;
unsigned int centerSum = 0;
unsigned int rightSum = 0;

unsigned int leftSumArray[50] = {0};
unsigned int rightSumArray[50] = {0};

//lox variables
#define DOOR_WIDTH 800
VL53L0X S1;
VL53L0X S2;

float reading1, reading2;
int occupancy_counter = 0;
String transition_string = "";

//State machine variables
enum Event {
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

float grid_eye_calibration_left()
{
  Serial.println("Left calibration");
  for (int j = 0; j < 50; j++)
  {
    //    Serial.println("Read start");
    gridEyeData = grid_eye_read();

    memcpy(gridEyeRight, pixels, 32);
    memcpy(gridEyeLeft, &(pixels[32]), 24);

    for (int i = 1; i <= 24; i++)
    {
      //    Serial.print(gridEyeLeft[i - 1]);
      //    if (i != 24) Serial.print(",");

      leftSum += gridEyeLeft[i - 1];
    }
    //    Serial.print("Left sum: ");
    //    Serial.println(j);
    leftSumArray[j] = leftSum;
    //    Serial.println("Sum stored");
    unsigned long currentTime = millis();
    //    while ((millis() - currentTime) < 100);
    delay(100);
    leftSum = 0;
  }

  float leftThreshold = 0.0;
  long totalSum = 0;
  for (int i = 0; i < 50; i++)
  {
    totalSum += leftSumArray[i];
  }

  leftThreshold = totalSum / 50;

  Serial.println("Retruning from left");
  return leftThreshold;
}

float grid_eye_calibration_right()
{
  Serial.println("Right calibration");
  for (int j = 0; j < 50; j++)
  {
    gridEyeData = grid_eye_read();

    memcpy(gridEyeRight, pixels, 32);
    memcpy(gridEyeLeft, &(pixels[32]), 32);

    for (int i = 1; i <= 32; i++)
    {
      //    Serial.print(gridEyeLeft[i - 1]);
      //    if (i != 24) Serial.print(",");

      rightSum += gridEyeRight[i - 1];
    }
    rightSumArray[j] = rightSum;

    double currentTime = millis();
    //    while((millis() - currentTime) < 100);
    delay(100);
    rightSum = 0;
  }

  float rightThreshold = 0.0;
  long totalSum = 0;
  for (int i = 0; i < 50; i++)
  {
    totalSum += rightSumArray[i];
  }
  rightThreshold = totalSum / 50;

  Serial.println("Return from right");
  return rightThreshold;
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

int calculate_sum(byte* rows)
{
  int sum = 0;

  for (int i = 1; i <= 24; i++)
  {
    Serial.print(rows[i - 1]);
    if (i != 24) Serial.print(",");

    sum += rows[i - 1];
  }

  return sum;
}

//State machine callbacks
void on_q0_enter() {
  Serial.println("Entering state Q0");
  transition_string.toCharArray(buf, 100);
  ms.Target (buf);
  //Test for entry
  result = ms.Match ("0.*1.*3.*2.*", 0);
  String mqtt_string;
  if (result == REGEXP_MATCHED) {
    Serial.println("ENTRY");
    mqtt_string = "entry: ";
    occupancy_counter++;
  }
  //Test for exit
  result = ms.Match ("0.*2.*3.*1.*", 0);
  if (result == REGEXP_MATCHED && occupancy_counter > 0) {
    Serial.println("EXIT");
    mqtt_string = "exit: ";
    occupancy_counter--;
  }
  Serial.print("Occupancy counter:");
  Serial.println(occupancy_counter);
  mqtt_string += String(occupancy_counter);
  char mqtt_char[10];
  mqtt_string.toCharArray(mqtt_char, 10);
  client.publish(mqtt_topic, mqtt_char);
  transition_string = "0";
}
void on_q1_enter() {
  Serial.println("Entering state Q1");
  transition_string += "1";
}
void on_q2_enter() {
  Serial.println("Entering state Q2");
  transition_string += "2";
}
void on_q3_enter() {
  Serial.println("Entering state Q3");
  transition_string += "3";
}


void setup() {
  // put your setup code here, to run once:

  // begin wifi and mqtt
  Serial.begin(115200);
  Serial.println("Setting up wifi and mqtt");
  setupWiFi();
  client.setServer(mqtt_server, 1883);

  // default settings
  Serial.println("Setting up grid eye sensor");
  bool status;
  status = amg.begin(0x68);
  amg.write8(AMG88xx_FPSC, AMG88xx_FPS_10);
  amg.setMovingAverageMode(false);

  if (!status) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }

  delay(100); // let grid eye sensor boot up

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

void loop() {
  //    reading1=u1.distanceInCm();
  //    reading2=u2.distanceInCm();

  if (!client.connected()) reconnect();
  client.loop();

  // Calibrating the sensor
  if (!calibrated)
  {
    Serial.println("Calibrating..!!");
    finalLeftThreshold = grid_eye_calibration_left() + 180;
    finalRightThreshold = grid_eye_calibration_right() + 150;
    Serial.print("Left Threshold: ");
    Serial.println(finalLeftThreshold);
    Serial.print("Right Threshold: ");
    Serial.println(finalRightThreshold);

    calibrated = true;
  }

  // Reading grid eye grid eye data
  gridEyeData = grid_eye_read();

  memcpy(gridEyeRight, pixels, 32);
  //  memcpy(gridEyeCenter, &(pixels[24]), 16);
  //  memcpy(gridEyeLeft, &(pixels[32]), 32);
  memcpy(gridEyeLeft, &(pixels[32]), 24);

  //  Serial.print("Sum for right = [");
  //  rightSum = calculate_sum(gridEyeRight);
  for (int i = 1; i <= 32; i++)
  {
    //    Serial.print(gridEyeRight[i - 1]);
    //    if (i != 24) Serial.print(",");

    rightSum += gridEyeRight[i - 1];
  }
  //  Serial.print("] = ");
  //  Serial.print("Right sum : ");
  //  Serial.println(rightSum);
  //  Serial.println();

  //  Serial.print("Sum for left = [");
  //  leftSum = calculate_sum(gridEyeLeft);
  for (int i = 1; i <= 24; i++)
  {
    //    Serial.print(gridEyeLeft[i - 1]);
    //    if (i != 24) Serial.print(",");

    leftSum += gridEyeLeft[i - 1];
  }
  //  Serial.print("] = ");
  //  Serial.print("Left sum : ");
  //  Serial.println(leftSum);
  //  Serial.println();
  if (rightSum < 2000 || leftSum < 1500) {
    Serial.println("Error: please restart!!");
    delay(2000);
    //    ESP.reset();
  }

  //  if (rightSum > 3600 && leftSum > 3600) Serial.println("U1 and U2 triggered");
  //  else
  {
    if (rightSum > finalRightThreshold)
    {
      Serial.println("U2 triggered");
      fsm.trigger(U2_TRIGGERED);
    }
    else
    {
      //      Serial.println("U1 halted");
      //      Serial.println(rightSum);
      fsm.trigger(U2_HALTED);
    }

    if (leftSum > finalLeftThreshold)
    {
      Serial.println("U1 triggered");
      fsm.trigger(U1_TRIGGERED);

    }
    else
    {
      //      Serial.println("U2 halted");
      //      Serial.println(leftSum);
      fsm.trigger(U1_HALTED);
    }
  }
  fsm.run_machine();

  rightSum = 0;
  centerSum = 0;
  leftSum = 0;

  //    Serial.println("----------/--------------------------------------------------");
  // reading1 = S1.readRangeSingleMillimeters();
  //    Serial.print("Distance from S1 1 : ");/
  //    Serial.print(reading1);/
  // if (S1.timeoutOccurred()) {
  //   Serial.print(" TIMEOUT");
  // }
  //    Serial.println();/

  //    Serial.print("Distan/ce from S1 2 : ");
  // reading2 = S2.readRangeSingleMillimeters();
  //    Serial.print(reading2);/
  // if (S2.timeoutOccurred()) {
  //   Serial.print(" TIMEOUT");
  // }
  //    Serial.println();/
  //    Serial.println("----/--------------------------------------------------------");
  //    Serial.println();/
  /*
      if(reading1 ==0 || reading2==0)
        return;
      if(reading1 <DOOR_WIDTH)
        fsm.trigger(U1_TRIGGERED);
      else
        fsm.trigger(U1_HALTED);

      if(reading2 <DOOR_WIDTH)
        fsm.trigger(U2_TRIGGERED);
      else
        fsm.trigger(U2_HALTED);
      fsm.run_machine();
  */
  //    Serial.print("S1 1 | 2 Reading :");
  //    Serial.print(reading1);
  //    Serial.print(" | ");
  //    Serial.print(reading2);
  //    Serial.println(" CM");
  delay(100);
}
