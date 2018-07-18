
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

//lox variables
#define DOOR_WIDTH 800
VL53L0X S1;
VL53L0X S2;

float reading1, reading2;
int occupancy_counter=0;
String transition_string="";

//State machine variables
enum Event{
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
char buf[100]={0};

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

int calculate_sum(int* rows)
{
    int sum = 0;

    for (int i=1; i<=24; i++)
    {
        Serial.print(rows[i-1]);
        if(i != 24) Serial.print(",");

        sum += rows[i-1];
    }

    return sum;
}

//State machine callbacks
void on_q0_enter(){
  Serial.println("Entering state Q0");
  transition_string.toCharArray(buf,100);
  ms.Target (buf);
  //Test for entry
  result = ms.Match ("0.*1.*3.*2.*", 0);
  if(result == REGEXP_MATCHED){
      Serial.println("ENTRY");
      occupancy_counter++;
  }
  //Test for exit
  result = ms.Match ("0.*2.*3.*1.*", 0);
  if(result == REGEXP_MATCHED && occupancy_counter>0){
      Serial.println("EXIT");
      occupancy_counter--;
  }
  Serial.print("Occupancy counter:");
  Serial.println(occupancy_counter);
  transition_string = "0";
}
void on_q1_enter(){
  Serial.println("Entering state Q1");
  transition_string += "1";
}
void on_q2_enter(){
  Serial.println("Entering state Q2");
  transition_string += "2";
}
void on_q3_enter(){
  Serial.println("Entering state Q3");
  transition_string += "3";
}


void setup() {
  // put your setup code here, to run once:
  
  // begin wifi and mqtt
  setupWiFi();
  client.setServer(mqtt_server, 1883);

    // default settings
  status = amg.begin(0x68);
  amg.write8(AMG88xx_FPSC, AMG88xx_FPS_10);
  amg.setMovingAverageMode(false);
  
  if (!status) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }

  delay(100); // let grid eye sensor boot up

    pinMode(S1_XSHUT, OUTPUT);
    digitalWrite(S1_XSHUT, LOW);
  
    delay(10);
  
    pinMode(S2_XSHUT, OUTPUT);
    digitalWrite(S2_XSHUT, LOW);
  
    Serial.begin(115200);
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

void loop() {
//    reading1=u1.distanceInCm();
//    reading2=u2.distanceInCm();

    if (!client.connected()) reconnect();
    client.loop();

    // Reading grid eye grid eye data 
    gridEyeData = grid_eye_read();

    memcpy(gridEyeRight, pixels, 24);
    memcpy(gridEyeCenter, &(pixels[24]), 16);
    memcpy(gridEyeLeft, &(pixels[40], 24));

    Serial.print("[");
    rightSum = calculate_sum(gridEyeRight);
    Serial.print("] = ");
    Serial.println(rightSum);
    Serial.println();

    Serial.print("[");
    leftSum = calculate_sum(gridEyeLeft);
    Serial.print("] = ");
    Serial.println(leftSum);
    Serial.println();

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
//    Serial.print("S1 1 | 2 Reading :");
//    Serial.print(reading1);
//    Serial.print(" | ");
//    Serial.print(reading2);
//    Serial.println(" CM");
    delay(100);
}
