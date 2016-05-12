#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define WORK_PIN 7
#define ALARM_PIN 0

void gpio_ready();
void pin_init();
void map(int *);
int reverse_map(int);
double write_to_pin(int, double);
void write_to_alarm_pin(int, double);
void turn_off_all_pins();
