
/*
 * This program reads the distance from multiple hc-sr04 sensors 
 * and produce output in the form of Centimeters. 
 * 
 * UltraDistSensor is library designed to Interface 
 * HC-SR04 Ultrasonics sensor to Arduino microcontrollers.
 * 
 * Designed and developed by
 * 
 * Shubham Trivedi on Oct 2017
 * 
 * No license required to access this library it is release into open source 
 * license
 * for any query you can write me at shubhamtrivedi95@gmail.com
 * follow me on facebook https://www.facebook.com/shubhamtrivedi95
 * on github https://www.github.com/shubhamtrivedi95
 * for letest updates subscribe at http://electro-passionindia.blogspot.in
 *   
 *   Pin configuration
 *  ___________________
 * | HC-SC04 | Arduino | 
 * |_________|_________|
 * |   Vcc   |   5V    |     
 * |   Trig  |    2    |
 * |   Echo  |    3    | 
 * |   Gnd   |   GND   |
 * |_________|_________|
 *  ___________________
 * | HC-SC04 | Arduino | 
 * |_________|_________|
 * |   Vcc   |   5V    |     
 * |   Trig  |    4    |
 * |   Echo  |    5    | 
 * |   Gnd   |   GND   |
 * |_________|_________|
 *  ___________________
 * | HC-SC04 | Arduino | 
 * |_________|_________|
 * |   Vcc   |   5V    |     
 * |   Trig  |    6    |
 * |   Echo  |    7    | 
 * |   Gnd   |   GND   |
 * |_________|_________|
 * 
 * attach()         This function configure sensor with arduino
 * 
 * distanceInCm()   This function produce distance in centimeter
 * 
 * distanceInInch() This function produce distance in Inches 
 * 
 * ChangeTimeout()  This functionis used to change echo timeout of receiveing pulse
 *                  by default the timeout is 20000 microseconds
 * Please note that you will get Approximated distance from HC-SR04 module, 
 * which will be having plus or minus tolernace upto 1cm or inch. 
 */
#include "Fsm.h"
#include <Regexp.h>

#include<UltraDistSensor.h>
//Ultrasonic variables
#define DOOR_WIDTH 70
UltraDistSensor u1;
UltraDistSensor u2;
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
  if(result == REGEXP_MATCHED){
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
    Serial.begin(9600);
    u2.attach(2,3);//Trigger pin , Echo pin
    u1.attach(4,5);//Trigger pin , Echo pin

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
    reading1=u1.distanceInCm();
    reading2=u2.distanceInCm();
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
//    Serial.print("Sensor 1 | 2 Reading :");
//    Serial.print(reading1);
//    Serial.print(" | ");
//    Serial.print(reading2);
//    Serial.println(" CM");
    delay(100);
}
