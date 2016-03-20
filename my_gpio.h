#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define WORK_PIN 7

void gpio_ready();
void pin_init();
void map(int *);
int reverse_map(int);
void write_to_pin(int, int);
void turn_off_all_pins();