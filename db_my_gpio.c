/************************************
This file contains functions used to
operate the GPIO pins on the raspberry
pi
**************************************/

#include "db_my_gpio.h"
#include "db_feedback.h"

//initialize GPIO functionality
void gpio_ready(int * pin) {
	
	//initialize wiringPi library
	if (wiringPiSetup() == -1) {
		fprintf(stderr, "error initializing wiringPiSetup\n");
    	exit(1);
    }
    
    //initialize pins
	pin_init();
	
	//map working pin
	map(pin);
}

//set pins to output mode
void pin_init() {
	
	//set pins 11,12,13,15,16,18,22,7,3 to OUTPUT
    pinMode(3, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(15, OUTPUT);
    pinMode(16, OUTPUT);
    pinMode(18, OUTPUT);
    pinMode(22, OUTPUT);
}

//map pin to GPIO number
void map(int *pin) {
	switch(*pin) {
		case 0:
			*pin = 11;
			break;
		case 1:
			*pin = 12;
			break;
		case 2:
			*pin = 13;
			break;
		case 3:
			*pin = 15;
			break;
		case 4:
			*pin = 16;
			break;
		case 5:
			*pin = 18;
			break;
		case 6:
			*pin = 22;
			break;
		case 7:
			*pin = 7;
			break;
		case 8:
			*pin = 3;
			break;
	}
}

//map GPIO number to pin number
int reverse_map(int pin) {

	if(pin == 12)
		return 1;
	
	if(pin == 11)
		return 0;
		
	if(pin == 13)
		return 2;

	if(pin == 15)
		return 3;

	if(pin == 16)
		return 4;

	if(pin == 18)
		return 5;

	if(pin == 22)
		return 6;
		
	if(pin == 7)
		return 7;

	if(pin == 3)
		return 8;
		
	return -1;


}

//write to pin for given time
double write_to_pin(int pin, double watering_time) {
	time_t start_t, end_t, total_time;
	int hour, min, sec;

	//get time of day  before opening valve
	time(&start_t);
	get_time(&hour, &min, &sec);

	//open valve
	digitalWrite(pin, 1);
	printf("Opened valves on pin %d at time %d:%d:%d\n", pin, hour, min, sec);

	//wait watering_time (in mili-seconds)
	delay(watering_time);

	//close valve
	digitalWrite(pin, 0);

	//get time of day after closing valve
	time(&end_t);
	get_time(&hour, &min, &sec);
	printf("Closed valves on pin %d at time %d:%d:%d\n", pin, hour, min, sec);
	
	//get time spent watering
	total_time = difftime(end_t, start_t);
	
	//print out the amount of time that pin was high
	printf("Time spent watering: %.1f\n", (float)total_time);
	
	//return the amount of time that the valve was open
	return total_time;
}

//write to pin for given time
void write_to_alarm_pin(int alarm_pin, double alarm_time) {
	map(&alarm_pin);

	//sound the alarm
	printf("Sounding alarm on pin %d\n", alarm_pin);
	digitalWrite(alarm_pin, 1);

	//let alarm ring
	delay(alarm_time);

	//turn off alarm
	digitalWrite(alarm_pin, 0);
}

//write low to all pins
void turn_off_all_pins() {
	int pin[8], i;
	
	for(i = 0; i < 8; i++) {
		map(&pin[i]);
		digitalWrite(pin[i], 0);
	}
}
	
