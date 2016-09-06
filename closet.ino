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
#define ADD_TIME 1.0045
#define USE_DATA_GAP_TIME 1000 /*IN MILLIS*/

#define BOTTON_1 11 
#define BOTTON_2 10 /* ON/OFF Switch */
#define BOTTON_3 9 /*JIANGE BLUE*/
#define BOTTON_4 8 /*CHAOSHENGBO RED*/
#define BOTTON_5 1

float distance[SENSOR_NUMBER];
float tempDistance = 0;
float minDistance = MAX_DISTANCE;

unsigned long timer;
unsigned long lastChangeTime, changeTime, timer_time;
unsigned long last_use_sensor_data_time;

int have_to_change_time = 0;
int use_sensor = 0;
int motor_move_state = 1; /* 1 in, 2 out, */

int wait_change = 0;
int in_shock = 0; /* shock = 1; stop = 0; for mode 4*/
int mode1 = 0, mode4 = 0; /*in mode4 or not*/
int shock_counter = 0, stay_counter = 0;
int shock_top = 7, stay_top = 8;

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

void write_data() {
	for (int i = 0; i < SENSOR_NUMBER; ++i) {
		Serial.print(distance[i]);
		Serial.print("cm ");
	}
	Serial.print("MIN is ~ ");
	Serial.print(getMin(SENSOR_NUMBER, distance));
	Serial.print("~ cm");
	Serial.print(changeTime);
	Serial.println();
}

void getSensorData(boolean user_write_data) {
	for (int i = 0; i < SENSOR_NUMBER; ++i) {
		digitalWrite(TRIG_PIN, LOW);
		timer = millis();
		while (millis() - timer < 2) {}
		digitalWrite(TRIG_PIN, HIGH);
		timer = millis();
		while (millis() - timer < 10) {}
		digitalWrite(TRIG_PIN, LOW);
		tempDistance = pulseIn(ECHO_START_PIN + i, HIGH, 250000);
		tempDistance /= 58;
		if (tempDistance == 0) {
			distance[i] = MAX_DISTANCE;
		}
		else {
			distance[i] = tempDistance;
		}
	}
	if (user_write_data) {
		write_data();
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

void go_stop() {
	digitalWrite(IN_PIN, LOW);
	digitalWrite(OUT_PIN, LOW);
	return;
}

void motorShake() {
	//if (wait_change) {
	//	go_stop();
	//	wait_change = 0;
	//	return;
	//} else {
	//	wait_change = 1;
	//}

	MsTimer2::stop();

	/*
	make sure to change move state at points
	*/
	if (have_to_change_time == 1) {
		if (motor_move_state == 1) {
			if (mode1) {
				go_stop();
				MsTimer2::start();
				return;
			}
			timer_time = changeTime;
			//MsTimer2::set((unsigned long)(changeTime), motorShake);
			have_to_change_time = 0;
		}
	}

	if (mode4 == 1) {
		if (in_shock) {
			++shock_counter;
			if (shock_counter > shock_top) {
				in_shock = 0;
				shock_counter = 0;
			}
		}
		else {
			go_stop();
			++stay_counter;
			if (stay_counter > stay_top) {
				in_shock = 1;
				stay_counter = 0;
				motor_move_state = 1;
			}
			MsTimer2::start();
			return;
		}
	}

	if (motor_move_state == 1) {
		motor_move_state = 2;
		go_stop();
		goOut();
		MsTimer2::set((unsigned long)(timer_time), motorShake);
	}
	else {
		motor_move_state = 1;
		go_stop();
		goIn();
		MsTimer2::set((unsigned long)(timer_time * ADD_TIME), motorShake);
	}

	MsTimer2::start();

	return;
}

void check_botton() {
	/*
	botton1: goOut a little and stop shake
	*/
	if (digitalRead(BOTTON_1) == HIGH) {
		delay(10);
		if (digitalRead(BOTTON_1) == HIGH) {
			MsTimer2::stop();
			use_sensor = 0;
			goOut();
			delay(50);
			go_stop();
		}
		while (digitalRead(BOTTON_1) == HIGH);
	}
	/*
	botton5: go in a little and stop shake
	*/
	if (digitalRead(BOTTON_5) == HIGH) {
		delay(10);
		if (digitalRead(BOTTON_5) == HIGH) {
			MsTimer2::stop();
			use_sensor = 0;
			goIn();
			delay(50);
			go_stop();
		}
		while (digitalRead(BOTTON_5) == HIGH);
	}
	/*
	botton 2
	start shake
	*/
	if (digitalRead(BOTTON_2) == HIGH) {
		delay(10);
		if (digitalRead(BOTTON_2) == HIGH) {
			MsTimer2::start();
		}
		while (digitalRead(BOTTON_2) == HIGH);
	}
	/*
	botton3 change change time for test
	*/
	if (digitalRead(BOTTON_3) == HIGH) {
		delay(10);
		if (digitalRead(BOTTON_3) == HIGH) {
			if (changeTime == CHANGE_TIME1) {
				mode4 = 0;
				shock_counter = 0;
				stay_counter = 0;
				changeTime = CHANGE_TIME2;
			}
			else if (changeTime == CHANGE_TIME2) {
				changeTime = CHANGE_TIME3;
				mode4 = 0;
				shock_counter = 0;
				stay_counter = 0;
			}
			else if (changeTime == CHANGE_TIME3) {
				mode4 = 1;
				in_shock = 1;
				changeTime = CHANGE_TIME4;
			} else {
				mode4 = 0;
				shock_counter = 0;
				stay_counter = 0;
				changeTime = CHANGE_TIME1;
			}
			have_to_change_time = 1;
		}
		while (digitalRead(BOTTON_3) == HIGH);
	}
	/*
	botton4: click to use sensor data or not
	*/
	if (digitalRead(BOTTON_4) == HIGH) {
		delay(10);
		if (digitalRead(BOTTON_4) == HIGH) {
			if (use_sensor == 0) {
				use_sensor = 1;
			} else {
				use_sensor = 0;
			}
		}
		while (digitalRead(BOTTON_4) == HIGH);
	}
	return;
}

void change_time_by_distance() {
	if (millis() - last_use_sensor_data_time < USE_DATA_GAP_TIME) {
		return;
	}
	else {
		last_use_sensor_data_time = millis();
	}

	if (minDistance < CHANGE_DISTANCE1) {
		if (timer_time != CHANGE_TIME1) {
			have_to_change_time = 1;
			changeTime = CHANGE_TIME1;
			mode1 = 1;
			mode4 = 0;
			shock_counter = 0;
			stay_counter = 0;
		}
	}
	else if (minDistance < CHANGE_DISTANCE2) {
		if (timer_time != CHANGE_TIME2) {
			have_to_change_time = 1;
			changeTime = CHANGE_TIME2;
			mode1 = 0;
			mode4 = 0;
			shock_counter = 0;
			stay_counter = 0;
		}
	}
	else if (minDistance < CHANGE_DISTANCE3) {
		if (timer_time != CHANGE_TIME3) {
			have_to_change_time = 3;
			changeTime = CHANGE_TIME3;
			mode4 = 0;
			mode1 = 0;
			shock_counter = 0;
			stay_counter = 0;
		}
	}
	else {
		if (timer_time != CHANGE_TIME4) {
			mode4 = 1;
			mode1 = 0;
			in_shock = 1;
			have_to_change_time = 1;
			changeTime = CHANGE_TIME4;
		}
	}

	return;
}

void setup()
{

	/* add setup code here */
	for (int i = 0; i < SENSOR_NUMBER; ++i) {
		pinMode(ECHO_START_PIN + i, INPUT);
	}
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(IN_PIN, OUTPUT);
	pinMode(OUT_PIN, OUTPUT);

	pinMode(BOTTON_1, INPUT);
	pinMode(BOTTON_2, INPUT);
	pinMode(BOTTON_3, INPUT);
	pinMode(BOTTON_4, INPUT);
	pinMode(BOTTON_5, INPUT);

	changeTime = CHANGE_TIME1;
	timer_time = changeTime;
	MsTimer2::set(timer_time, motorShake);
	//MsTimer2::start();
	return;
}

void loop()
{
  /* add main program code here */
	check_botton();

	getSensorData(true);

	if (use_sensor == 1) {
		minDistance = getMin(SENSOR_NUMBER, distance);
		change_time_by_distance();
	}

	//if (Serial.available()) { 
	//	char in_char = Serial.read();
	//	if (in_char == 'a') {
	//		digitalWrite(13, HIGH);
	//		delay(500);
	//		digitalWrite(13, LOW);
	//	}
	//}

	return;
}
