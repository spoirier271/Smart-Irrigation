#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define WATER_TIMES 1

#define TIME_LONG_MOISTURE 2000 //long watering time
#define TIME_MEDIUM_MOISTURE 1000 //medium watering time
#define TIME_SHORT_MOISTURE 500 //short watering time
#define TIME_NONE_MOISTURE 0 //minimal watering time

#define THRESH_MOISTURE_HIGH 600 //maximum sensor value threshold
#define THRESH_MOISTURE_LOW 400 //low sensor value threshold
#define THRESH_MOISTURE_NONE 200 //minimal sensor value threshold

#define TIME_LONG_TEMPERATURE 200 //long watering time
#define TIME_MEDIUM_TEMPERATURE 100 //medium watering time
#define TIME_SHORT_TEMPERATURE 50 //short watering time
#define TIME_NONE_TEMPERATURE 0 //minimal watering time

#define THRESH_TEMPERATURE_HIGH 600 //maximum sensor value threshold
#define THRESH_TEMPERATURE_LOW 400 //low sensor value threshold
#define THRESH_TEMPERATURE_NONE 200 //minimal sensor value threshold

#define WATERING_TIME_HOUR_1 3 //time of day to water
#define WATERING_TIME_HOUR_2 4 
#define WATERING_TIME_HOUR_3 5 

#define WATERING_TIME_MIN 0 //minute adjustment to watering period

#define WATERING_TIME_SEC_1 30 //second adjustment to watering period
#define WATERING_TIME_SEC_2 35 //second adjustment to watering period
#define WATERING_TIME_SEC_3 40 //second adjustment to watering period

#define WATERING_TIME_HOUR_RETRY 6 //retry time of day for watering
#define WATERING_TIME_MIN_RETRY 30 //retry minute adjustment for watering
#define WATERING_TIME_SEC_RETRY 45 //retry second adjustment for watering

#define PAUSE_TIME 1 //todo: change to something legit

void get_time(int *, int *, int *);
int decide_watering_time(struct frame frame_average);
int moisture_watering_time(struct frame);
int temperature_watering_time(struct frame);