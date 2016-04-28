/******************************************
This file contains the main logic used for 
processing values received from the sensor 
readings and using it to formulate the time 
needed to open the valves.

4/28/16: Code now uses the sqlite database to
process frames.
******************************************/

#include "db_feedback.h"
#include "my_gpio.h"

int main() {
	int hour, min, sec, frames_received, i, pin = WORK_PIN;
	double watering_time = 0;
	struct frame frames[FRAMES_TO_GET], frame_average;
	bool no_frames, watered;
	time_t start_t, end_t;
    
	//prepare gpio
	gpio_ready(&pin);
	
	//set frames
	set_frames(&frames);

	//set default frame to empty
	frame_init(empty_frame);
	
	//initialize frame array
	for(i = 0; i < FRAMES_TO_GET; i++)
		frame_init(frames[i]);
		
	//initialize average frame
	frame_init(frame_average);
	
	//waiting loop
	int z = 0;
	watered = false;
	while(1) {
	
		//get the time
		get_time(&hour, &min, &sec);
		
		if( (hour == (WATERING_TIME_HOUR_1 + 1)) && (watered == 1) ) {
			watered = false;
		}

		//check if it's time to water the plants
// 		if( (hour == WATERING_TIME_HOUR_1) && (!watered) ) {
		if(min == 30) {
			printf("Starting feedback control process %d at time %d:%d:%d\n", z, hour, min, sec);
			z++;

			//get all frames from FRAMES_TO_GET to the most recent frame
			printf("Processing %d frames...\n", FRAMES_TO_GET);
			
			//start counting how long it takes to process frames
			time(&start_t);
			
			//check if no frames received
			if( (frames_received = get_frames(frames)) == 0 ) {
				printf("No frames received\n");
				sleep(PAUSE_TIME);
				no_frames = true;
				
				continue;
			} else {
				
				//stop frame processing timer and print out how long it took to process frames
				time(&end_t);
				printf("Successfully processed %d frames in %g second(s)\n", frames_received, difftime(end_t, start_t));
				
				no_frames = false;
				
				//get average of all frame data
				frame_average = get_average(frames, FRAMES_TO_GET);
				
				//calculate how much time should be spent watering based on frame data
				watering_time = decide_watering_time(frame_average);
				
				//display calculated watering time
				print_watering_time(watering_time);
				
				//open valve
				write_to_pin(pin, watering_time);
				printf("\n");
				
				//set watered to true
// 				watered = true;
			}

		}
		
		//if no frames again, then assume system failure and turn off all valves
		if(no_frames && (hour == WATERING_TIME_HOUR_RETRY) ) {
			if( (frames_received = get_frames(frames)) == 0 ) {
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

