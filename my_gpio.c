/************************************
This file contains functions used to
operate the GPIO pins on the raspberry
pi
**************************************/

#include "my_gpio.h"

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
	
	//set pins 12,13,15,16,18,22,7,3 to OUTPUT
    pinMode(3, OUTPUT);
    pinMode(7, OUTPUT);
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
		
	return 0;


}

//write to pin for given time
void write_to_pin(int pin, int time) {
	digitalWrite(pin, 1);
    printf("pin %d HIGH\n", reverse_map(pin));
    delay(time);
    digitalWrite(pin, 0);
    printf("pin %d LOW\n", reverse_map(pin));
    delay(time);
}

//write low to all pins
void turn_off_all_pins() {
	int pin[8], i;
	
	for(i = 0; i < 8; i++) {
		map(&pin[i]);
		digitalWrite(pin[i], 0);
	}
}
	