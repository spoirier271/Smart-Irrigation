/******************************************
This file contains the main logic used for 
processing values received from the sensor 
readings and using it to formulate the time 
needed to open the valves.
******************************************/

#include "frames.h"
#include "feedback.h"
#include "my_gpio.h"

int main() {
	int hour, min, sec, frames_received, i, watering_time = 0, pin = WORK_PIN;
	struct frame frames[FRAMES_TO_GET], frame_average;
	bool no_frames;
    
	//prepare gpio
	gpio_ready(&pin);

	//set default frame to empty
	frame_init(empty_frame);
	
	//initialize frame array
	for(i = 0; i < FRAMES_TO_GET; i++)
		frame_init(frames[i]);
		
	//initialize average frame
	frame_init(frame_average);
	
	//waiting loop
	while(1) {
	
		//get the time
		get_time(&hour, &min, &sec);

		//check if it's time to water the plants
		if(sec == WATERING_TIME_SEC_1) {
			
			//get all frames from FRAMES_TO_GET to the most recent frame
			if((frames_received = get_last_frames(FILE_NAME, FRAMES_TO_GET, frames)) == 0) {
				printf("No frames received\n");
				sleep(PAUSE_TIME);
				no_frames = true;
				
				continue;
			} else {
				no_frames = false;
				
				//get average of all frame data
				frame_average = get_average(frames, FRAMES_TO_GET);
				
				//calculate how much time should be spent watering based on frame data
				watering_time = decide_watering_time(frame_average);
				
				//open valve
				write_to_pin(pin, watering_time);
				sleep(PAUSE_TIME);
			}

		}
		
		//if no frames again, then assume system failure and turn off all valves
		if(no_frames && (sec == WATERING_TIME_SEC_RETRY) ) {
			if((frames_received = get_last_frames(FILE_NAME, FRAMES_TO_GET, frames)) == 0) {
				printf("NO FRAMES AGAIN PLEASE CHECK IF XBEES ARE DEAD!!!\n");
				turn_off_all_pins();
				
				continue;

			} else {
				no_frames = false;
				frame_average = get_average(frames, FRAMES_TO_GET);
				watering_time = decide_watering_time(frame_average);
				write_to_pin(pin, watering_time);
				sleep(PAUSE_TIME);
			}
		}
	}
		
}

//get the current time of day
void get_time(int * hour, int * min, int * sec) {
	char t[100], buff[100], hour_buff[100], min_buff[100], sec_buff[100];
	time_t ticks;
	
	ticks = time(NULL);
	sprintf(t, "%s", ctime(&ticks));
	
	strcpy(buff, strtok(t, ":"));
	strcpy(hour_buff, buff + 11);
	*hour = atoi(hour_buff);
	
	strcpy(buff, strtok(NULL, ":"));
	strcpy(min_buff, buff);
	*min = atoi(min_buff);
	
	strcpy(buff, strtok(NULL, ":"));
	strcpy(sec_buff, buff);
	*sec = atoi(sec_buff);
}

int decide_watering_time(struct frame frame) {

	return moisture_watering_time(frame) + temperature_watering_time(frame);
		
}

//get watering time adjustment from moisture data
int moisture_watering_time(struct frame frame_average) {
	int m;
	
	m = frame_average.data.moisture_sensor_average_value;
	
	if(m > THRESH_MOISTURE_HIGH) {
	
		return TIME_LONG_MOISTURE;	
	} else if( (m < THRESH_MOISTURE_HIGH) && (m > THRESH_MOISTURE_LOW) ) {
		
		return TIME_MEDIUM_MOISTURE;
	} else if( (m < THRESH_MOISTURE_LOW) && (m > THRESH_MOISTURE_NONE) ) {
		
		return TIME_SHORT_MOISTURE;
	} else {
		
		return TIME_NONE_MOISTURE;
	}	
}

//get watering time adjustment from temperature data
int temperature_watering_time(struct frame frame_average) {
	int t;
	
	t = frame_average.data.temperature_sensor_value;
	
	if(t > THRESH_TEMPERATURE_HIGH) {
	
		return TIME_LONG_TEMPERATURE;
	} else if( (t < THRESH_TEMPERATURE_HIGH) && (t > THRESH_TEMPERATURE_LOW) ) {
	
		return TIME_MEDIUM_TEMPERATURE;
	} else if( (t < THRESH_TEMPERATURE_LOW) && (t > THRESH_TEMPERATURE_NONE) ){
	
		return TIME_SHORT_TEMPERATURE;
	} else {
	
		return  TIME_NONE_TEMPERATURE;
	}
}