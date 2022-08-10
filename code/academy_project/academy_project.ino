#include "ServoSmooth.h"
#include <SoftwareSerial.h>

#define START_ANGLE 90
#define MIN_ANGLE 5
#define MAX_ANGLE 175
#define PIN_SERVO_LEFT 11
#define PIN_SERVO_RIGHT_1 9
#define PIN_SERVO_RIGHT_2 10
#define RX_PIN 0
#define TX_PIN 1
#define LED_PIN 13
#define BPS 9600
#define INTERVAL_GET_DATA 20
#define TIME_TICK_SERVO 100
#define LED_BLINK 1000
#define MAX_SPEED 100
#define ANGLE_DELTA 10
#define STRENTH_DELTA 5
#define ACCEL 0.7

// servo init
ServoSmooth servo_left(MAX_ANGLE);
ServoSmooth servo_right_1(MAX_ANGLE);
ServoSmooth servo_right_2(MAX_ANGLE);

// bluetooth module init
SoftwareSerial bluetooth(RX_PIN, TX_PIN);

// variables for parsing bluetooth
String data = "";
String angle_s = "";
String strength_s = "";
String button_s = "";

// variables for timing
uint32_t servoTimer = 0;
uint32_t times = 0;
uint32_t times_led = 0;

// main data variables
unsigned angle = 0;
unsigned strength = 0;
unsigned prev_angle = 0;
unsigned prev_strength = 0;
unsigned button = 0;

// variables for speed
int speed_X = 0;
int speed_Y = 0;
int speed_left = 0;
int speed_right_1 = 0;
int speed_right_2 = 0;

// flag for blinking LED
bool led_state = 0;


// func for printing data in serial port
void print_data(String angle, String strength, String button);
// func for set speed for each servo
void move_wheels(unsigned angle, unsigned strength);



void setup() {

  // start up all servos in middle position
  servo_right_1.attach(PIN_SERVO_RIGHT_1, START_ANGLE);
  servo_right_2.attach(PIN_SERVO_RIGHT_2, START_ANGLE);
  servo_left.attach(PIN_SERVO_LEFT, START_ANGLE);

  // setting up the acceleration
  servo_right_1.setAccel(ACCEL);
  servo_right_2.setAccel(ACCEL);
  servo_left.setAccel(ACCEL);

  // move servos in start position
  servo_right_1.smoothStart();
  servo_right_2.smoothStart();
  servo_left.smoothStart();

  // open serial ports
  Serial.begin(BPS);
  bluetooth.begin(BPS);
}



void loop() {

  // checking up for new data from bluetooth by some interval
  if (millis() - times >= INTERVAL_GET_DATA and bluetooth.available() > 0) {
    data = bluetooth.readStringUntil('#');

    // parsing data
    if (data.length() == 7) {
      angle_s = data.substring(0, 3);
      strength_s = data.substring(3, 6);
      button_s = data.substring(6, 8);

      print_data(angle_s, strength_s, button_s);

      // save previous positions for removing useless movement
      prev_angle = angle;
      prev_strength = strength;

      angle = angle_s.toInt();
      strength = strength_s.toInt();
      button = button_s.toInt();

      if (abs(prev_angle - angle) >= ANGLE_DELTA or abs(prev_strength - strength) >= STRENTH_DELTA)
        move_wheels(angle, strength);

      // clear bluetooth port buffer
      bluetooth.flush();
      data = "";
    }
    times = millis();
  }

  // moving each servo
  boolean state_left = servo_left.tick();
  boolean state_right_1 = servo_right_1.tick();
  boolean state_right_2 = servo_right_2.tick();

  // blinking buildin LED
  if ((millis() - times_led) >= LED_BLINK or button != 0) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    times_led = millis();
    button = 0;
  }
}



void print_data(String angle, String strength, String button) {
  Serial.print("angle: ");
  Serial.print(angle);
  Serial.print('\t');
  Serial.print("strength: ");
  Serial.print(strength);
  Serial.print('\t');
  Serial.print("button: ");
  Serial.print(button);
  Serial.println("");
}



void move_wheels(unsigned angle, unsigned strength) {

  // calc speed_X and speed_Y in project's coordinate system
  speed_X = (int) (MAX_SPEED * strength / 100 * cos(angle * DEG_TO_RAD));
  speed_Y = (int) (MAX_SPEED * strength / 100 * sin(angle * DEG_TO_RAD));

  // calc speed for each servo
  speed_left = speed_X;
  speed_right_1 = (int) ((-1) * speed_X / 2 - sqrt(3) * speed_Y / 2);
  speed_right_2 = (int) ((-1) * speed_X / 2 + sqrt(3) * speed_Y / 2) ;

  // handel situation with negative speed for each servo
  if (speed_left < 0) {
    speed_left *= -1;
    servo_left.setSpeed(speed_left);
    servo_left.setTargetDeg(MIN_ANGLE);
  } else {
    servo_left.setSpeed(speed_left);
    servo_left.setTargetDeg(MAX_ANGLE);
  }

  if (speed_right_1 < 0) {
    speed_right_1 *= -1;
    servo_right_1.setSpeed(speed_left);
    servo_right_1.setTargetDeg(MIN_ANGLE);
  } else {
    servo_right_1.setSpeed(speed_left);
    servo_right_1.setTargetDeg(MAX_ANGLE);
  }

  if (speed_right_2 < 0) {
    speed_right_2 *= -1;
    servo_right_2.setSpeed(speed_left);
    servo_right_2.setTargetDeg(MIN_ANGLE);
  } else {
    servo_right_2.setSpeed(speed_left);
    servo_right_2.setTargetDeg(MAX_ANGLE);
  }

}
