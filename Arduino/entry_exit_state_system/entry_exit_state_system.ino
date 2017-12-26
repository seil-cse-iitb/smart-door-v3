/*
Finite state machine to detect entry and exit using HC-SR04
Author:  Steve Garratt 
Date:    19/10/13
 
Interrupts from timer 1 are used to schedule and deliver the sensor
trigger pulse. The same interrupt is used to control the flash rate
of the onboard LED indicating distance.

The duration of the sensor echo signal is measured by generating an
external interrupt ervery time the echo signal changes state.

This sketch uses the TimerOne library downloadable from here:
http://code.google.com/p/arduino-timerone/downloads/detail?name=TimerOne-v9.zip&can=2&q=

Install the library using the following guide:
http://arduino.cc/en/Guide/Libraries
*/

#include <TimerOne.h>                                 // Header file for TimerOne library

#define LED 13                                 // Pin 13 onboard LED

#define TIMER_US 50                                   // 50 uS timer duration 
#define TICK_COUNTS 4000                              // 200 mS worth of timer ticks


class Ultrasonic{
  public:
    int triggerPin;
    int echoPin;
    volatile long echo_start = 0;                         // Records start of echo pulse 
    volatile long echo_end = 0;                           // Records end of echo pulse
    volatile long echo_duration = 0;                      // Duration - difference between end and start
    volatile int trigger_time_count = 0;                  // Count down counter to trigger pulse time

    Ultrasonic(){
      pinMode(triggerPin,OUTPUT);
      pinMode(echoPin,INPUT);
    }
    Ultrasonic(int tp, int ep){
      triggerPin = tp;
      echoPin = ep;
      pinMode(triggerPin,OUTPUT);
      pinMode(echoPin,INPUT);

    }
    boolean isTriggered();
    // --------------------------
    // echo_interrupt() External interrupt from HC-SR04 echo signal. 
    // Called every time the echo signal changes state.
    //
    // Note: this routine does not handle the case where the timer
    //       counter overflows which will result in the occassional error.
    // --------------------------
    void echo_interrupt()
    {
      switch (digitalRead(echoPin))                     // Test to see if the signal is high or low
      {
        case HIGH:                                      // High so must be the start of the echo pulse
          echo_end = 0;                                 // Clear the end time
          echo_start = micros();                        // Save the start time
          break;
          
        case LOW:                                       // Low so must be the end of hte echo pulse
          echo_end = micros();                          // Save the end time
          echo_duration = echo_end - echo_start;        // Calculate the pulse duration
          break;
      }
    }

    // --------------------------
  // trigger_pulse() called every 50 uS to schedule trigger pulses.
  // Generates a pulse one timer tick long.
  // Minimum trigger pulse width for the HC-SR04 is 10 us. This system
  // delivers a 50 uS pulse.
  // --------------------------
  void trigger_pulse()
  {
        static volatile int state = 0;                 // State machine variable
  
        if (!(--trigger_time_count))                   // Count to 200mS
        {                                              // Time out - Initiate trigger pulse
           trigger_time_count = TICK_COUNTS;           // Reload
           state = 1;                                  // Changing to state 1 initiates a pulse
        }
      
        switch(state)                                  // State machine handles delivery of trigger pulse
        {
          case 0:                                      // Normal state does nothing
              break;
          
          case 1:                                      // Initiate pulse
             digitalWrite(this->triggerPin, HIGH);              // Set the trigger output high
             state = 2;                                // and set state to 2
             break;
          
          case 2:                                      // Complete the pulse
          default:      
             digitalWrite(this->triggerPin, LOW);               // Set the trigger output low
             state = 0;                                // and return state to normal 0
             break;
       }
  }
  private:

};

Ultrasonic US1(4,2);
//Wrapper function to enable a member function to be passed as ISR. One needs to be created for each sensor
void US1_interrupt_wrapper(){
  US1.echo_interrupt();
}

Ultrasonic US2(5,3);
//Wrapper function to enable a member function to be passed as ISR. One needs to be created for each sensor
void US2_interrupt_wrapper(){
  US2.echo_interrupt();
}
// ----------------------------------
// setup() routine called first.
// A one time routine executed at power up or reset time.
// Used to initialise hardware.
// ----------------------------------
void setup() 
{

  pinMode(LED, OUTPUT);                        // Onboard LED pin set to output
  
  Timer1.initialize(TIMER_US);                        // Initialise timer 1
  Timer1.attachInterrupt( timerIsr );                 // Attach interrupt to the timer service routine 
  
  attachInterrupt(digitalPinToInterrupt(US1.echoPin), US1_interrupt_wrapper, CHANGE);  // Attach interrupt to the sensor echo input

  attachInterrupt(digitalPinToInterrupt(US2.echoPin), US2_interrupt_wrapper, CHANGE);  // Attach interrupt to the sensor echo input
  
  Serial.begin (9600);                                // Initialise the serial monitor output

}

// ----------------------------------
// loop() Runs continuously in a loop.
// This is the background routine where most of the processing usualy takes place.
// Non time critical tasks should be run from here.
// ----------------------------------
void loop()
{
    int distance = US2.echo_duration/58;
    Serial.println(distance);               // Print the distance in centimeters
    if(distance <70)
      Serial.println("Entry");
    delay(100);                                       // every 100 mS
}

// --------------------------
// timerIsr() 50uS second interrupt ISR()
// Called every time the hardware timer 1 times out.
// --------------------------
void timerIsr()
{
  
    US2.trigger_pulse();                                 // Schedule the trigger pulses
}





