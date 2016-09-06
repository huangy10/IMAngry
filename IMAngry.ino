#include <MsTimer2.h>

#define TRIG_PIN 2
#define ECHO_START_PIN 4
#define IN_PIN 12
#define OUT_PIN 13

#define SENSOR_NUMBER 5

#define MAX_DISTANCE 99999
#define CHANGE_DISTANCE1 80
#define CHANGE_DISTANCE2 150
#define CHANGE_DISTANCE3 250

#define CHANGE_TIME1 199
#define CHANGE_TIME2 500
#define CHANGE_TIME3 700
#define CHANGE_TIME4 200

#define USE_DATA_GAP_TIME 1000 /*IN MILLIS*/

enum UserState {
  NEAR, MIDDLE, FAR
};

UserState userState = FAR;

float distance[SENSOR_NUMBER];
float minDis = MAX_DISTANCE;

unsigned long timer;
unsigned long lastChangeTime, changeTime, timer_time;
unsigned long last_sin;

float timer_duration = 0;

float getMin(int length, float a[]) {
  /*
    give the min float data of an array
  */
  float min_value = MAX_DISTANCE + 1;
  for (int i = 0; i < length; ++i) {
    min_value = min(min_value, a[i]);
  }
  return min_value;
}

void getSensorData() {
  float temp = 0;
  for (int i = 0; i < SENSOR_NUMBER; ++i) {
    digitalWrite(TRIG_PIN, LOW);
    timer = millis();
    while (millis() - timer < 2) {}
    digitalWrite(TRIG_PIN, HIGH);
    timer = millis();
    while (millis() - timer < 10) {}
    digitalWrite(TRIG_PIN, LOW);
    temp = pulseIn(ECHO_START_PIN + i, HIGH, 250000);
    temp /= 58;
    if (temp == 0) {
      distance[i] = MAX_DISTANCE;
    }
    else {
      distance[i] = temp;
    }
  }
}

void motor_loop() {

}

void setup() {
  // PIN Definition
  for (int i = 0; i < SENSOR_NUMBER; ++i) {
    pinMode(ECHO_START_PIN + i, INPUT);
  }
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(IN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);

  Serial.begin(9600);

  // fire the timer
  timer_duration = CHANGE_TIME1;
  MsTimer2::set(USE_DATA_GAP_TIME, state_switch_according_to_sensor_data);
  MsTimer2::start();
}

void state_switch_according_to_sensor_data() {
  getSensorData();
  float minDis = getMin(SENSOR_NUMBER, distance);
  Serial.println(minDis);
  if (minDis < CHANGE_DISTANCE1) {
    userState = NEAR;
  } else if (minDis < CHANGE_DISTANCE2) {
    userState = MIDDLE;
  } else {
    userState = FAR;
  }
}


void goOut() {
  digitalWrite(OUT_PIN, HIGH);
  digitalWrite(IN_PIN, LOW);

  return;
}

void goIn() {
  digitalWrite(IN_PIN, HIGH);
  digitalWrite(OUT_PIN, LOW);

  return;
}

void goStop() {
  digitalWrite(IN_PIN, LOW);
  digitalWrite(OUT_PIN, LOW);
  return;
}

void onUserNear() {
  goStop();
}


void moveMotorWithPeriod(float period) {
  static bool switcher = false;
  goStop();
  if (switcher) {
    goOut();
  } else {
    goIn();
  }
  switcher = !switcher;
  delay(period);
}

void onUserMiddle() {
  moveMotorWithPeriod(CHANGE_TIME2);
}

void onUserFar() {
  moveMotorWithPeriod(CHANGE_TIME3);
}

void loop() {
  switch (userState) {
    case NEAR:
      onUserNear();
      break;
    case MIDDLE:
      onUserMiddle();
      break;
    case FAR:
      onUserFar();
      break;
  }
}

