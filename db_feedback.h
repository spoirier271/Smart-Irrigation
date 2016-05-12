#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "db_frames.h"

//this is for 1 inch of water per week

#define WATER_TIMES 1

#define ALARM_TIME 10000

#define BASE_WATERING_INCHES_PER_WEEK 1
#define BASE_WATERING_TIME_PER_DAY 1

#define TIME_LONG_MOISTURE 2000 //long increase to watering time
#define TIME_MEDIUM_MOISTURE 1000 //medium increase to watering time
#define TIME_GOOD_MOISTURE 0 //no adjustment to watering time
#define TIME_SHORT_MOISTURE 500 //short decrease to watering time
#define TIME_NONE_MOISTURE 0 //minimal decrease to watering time

#define THRESH_MOISTURE_TOO_HIGH 100 //maximum sensor value threshold (centibars)
#define THRESH_MOISTURE_GOOD_HIGH 60 //low sensor value threshold (centibars)
#define THRESH_MOISTURE_GOOD_LOW 30 //very low sensor value threshold (centibars)
#define THRESH_MOISTURE_NONE 10 //minimal sensor value threshold (centibars)

#define TIME_LONG_TEMPERATURE 200 //long watering time
#define TIME_MEDIUM_TEMPERATURE 100 //medium watering time
#define TIME_SHORT_TEMPERATURE 50 //short watering time
#define TIME_VERY_SHORT_MOISTURE
#define TIME_NONE_TEMPERATURE 0 //minimal watering time

#define THRESH_TEMPERATURE_HIGH 600 //maximum sensor value threshold
#define THRESH_TEMPERATURE_LOW 400 //low sensor value threshold
#define THRESH_TEMPERATURE_NONE 200 //minimal sensor value threshold

#define WATERING_TIME_HOUR_1 6 //time of day to water
#define WATERING_TIME_HOUR_2 7 
#define WATERING_TIME_HOUR_3 8 

#define WATERING_TIME_MIN 0 //minute adjustment to watering period

#define WATERING_TIME_SEC_1 30 //second adjustment to watering period
#define WATERING_TIME_SEC_2 35 //second adjustment to watering period
#define WATERING_TIME_SEC_3 40 //second adjustment to watering period

#define WATERING_TIME_HOUR_RETRY 7 //retry time of day for watering
#define WATERING_TIME_MIN_RETRY 30 //retry minute adjustment for watering
#define WATERING_TIME_SEC_RETRY 45 //retry second adjustment for watering

#define PAUSE_TIME 1 //todo: change to something legit

void get_time(int *, int *, int *);
double decide_watering_time(struct frame);
double get_moisture_watering_inches(struct frame);
double get_temperature_watering_inches(struct frame);
double convert_to_miliseconds(double);
void print_watering_time(double);
