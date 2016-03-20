/***************************************
This file contains the functions used to
parse the soil data file.
****************************************/

#include "frames.h"


//fill frame with default attributes
void frame_init(struct frame frame) {
	int i;
	
	strcpy(frame.julian, "empty");
	strcpy(frame.date, "empty");
	strcpy(frame.node_id, "empty");
	
	for(i = 0; i < NUM_OF_MOISTURE_SENSORS; i++)
		frame.data.moisture_sensor_value[i] = 0;
		
	frame.data.moisture_sensor_average_value = 0;
	frame.data.temperature_sensor_value = 0;
}

//return number of frames in file
int get_frame_count(char * filename) {
	int frame_count = 0;
	char nextChar;
	FILE *fp = fopen(filename,"r");
	
	if(fp == NULL) {
		printf("unable to read file\n");
		return 0;
	}
	
	if(fseek( fp, 109, SEEK_SET ) != 0)
		return 0;

	//count lines in file
	while (1) {
		if(nextChar == '\n' || feof(fp)) {
			frame_count++;
			if(feof(fp))
				break;
		}
		nextChar = getc(fp);
		if(feof(fp))
			break;
	}
	fclose(fp);

	return frame_count;
}

//fill array with requested number of frames from file
int get_last_frames(char * filename, int requested_num_of_frames, struct frame *frames) {
	int total_frame_count, frame_count;

	total_frame_count = get_frame_count(filename);
	if( (frame_count = get_all_frames(filename, &frames[0], requested_num_of_frames, total_frame_count) ) == 0)
		return 0;
	
	return frame_count;
}


//fill array with all frames in file
int get_all_frames(char * filename, struct frame *frames, int requested_num_of_frames, int total_frame_count) {

	char ch;
	int j = 0, line_count = 0, frame_count = 0;
	char line[1000];

	//open data file for reading
	FILE *fp = fopen(filename,"r");

	if(fp == NULL) {
		printf("unable to read file\n");
		return 0;
	}
	
	if(fseek( fp, 109, SEEK_SET ) != 0)
		return 0;

	//parse lines in file
	do {
		ch = fgetc(fp);
		if(feof(fp)) {
			line[j] = 0;
			if(j > 0) {
				get_frame(line, line_count, &frames[line_count]);
			}
			break;
		}
		else if(ch == '\n') {
			line[j] = 0;
			j = 0;
			line_count++;
			if(line_count > (total_frame_count - requested_num_of_frames)) {
				get_frame(line, line_count, &frames[frame_count]);
				frame_count++;
				
			}

		}
		else {
			line[j] = ch;
			j++;
		}
	} while(1);

	fclose(fp);
	return frame_count;
}


//fill single frame with given value from lin in file
struct frame get_frame(char * line, int frame_number, struct frame *frame) {
	char temp[1000];

	//check for valid frame
	if(line_check(line) == 0) {
	
		//fill empty frame
		strcpy(frame->julian, "empty");
		strcpy(frame->date, "empty");
		strcpy(frame->node_id, "empty");
		
		frame->data.moisture_sensor_value[0] = 0;
		frame->data.moisture_sensor_value[1] = 0;
		frame->data.moisture_sensor_value[2] = 0;
		
		frame->data.moisture_sensor_average_value = moisture_average(frame->data.moisture_sensor_value,
			NUM_OF_MOISTURE_SENSORS);
			
		frame->data.temperature_sensor_value = 0;

		return empty_frame;
	}

	strcpy(temp, line);

	//fill frame struct header info
	strcpy(frame->julian, strtok(temp, DELIM));
	strcpy(frame->date, strtok(NULL, DELIM));
	strcpy(frame->node_id, strtok(NULL, DELIM));
	
	
	//fill frame struct moisture sensor info
	strcpy(temp, strtok(NULL, DELIM));
	frame->data.moisture_sensor_value[0] = atoi(temp);
	strcpy(temp, strtok(NULL, DELIM));
	frame->data.moisture_sensor_value[1] = atoi(temp);
	strcpy(temp, strtok(NULL, DELIM));
	frame->data.moisture_sensor_value[2] = atoi(temp);
	
	//take temperature sensor average
	frame->data.moisture_sensor_average_value = moisture_average(frame->data.moisture_sensor_value,
		NUM_OF_MOISTURE_SENSORS);
	
	//fill frame struct temperature sensor info
	strcpy(temp, strtok(NULL, DELIM));
	frame->data.temperature_sensor_value = atoi(temp);
	
	return *frame;
}

//check if line contains valid frame
int line_check(char * line) {
	if(line[0] != '1')
	
		return 0;
		
	return 1;
}


//returns a frame filled with its corresponding average data average values
struct frame get_average(struct frame frames[], int frame_count) {
	int i, moisture_sum = 0, temp_sum = 0;
	struct frame frame_average;
	char avg[100];
	
	frame_init(frame_average);
	strcpy(avg, "average");
	avg[strlen(avg)] = '\0';
	
	for(i = 0; i < frame_count; i++) {
		moisture_sum += frames[i].data.moisture_sensor_average_value;
		temp_sum += frames[i].data.temperature_sensor_value;
	}
	frame_average.data.moisture_sensor_average_value = 	moisture_sum / frame_count;
	for(i = 0; i < NUM_OF_MOISTURE_SENSORS; i++)
		frame_average.data.moisture_sensor_value[i] = frame_average.data.moisture_sensor_average_value;
	frame_average.data.temperature_sensor_value = temp_sum / frame_count;

	strcpy(frame_average.julian, avg);
	strcpy(frame_average.date, avg);
	strcpy(frame_average.node_id, avg);
	
	return frame_average;
}

//returns the frame's average moisture sensor value
int moisture_average(int moisture[], int num_of_moisture_sensors) {
	int i, sum = 0;
	for(i = 0; i < num_of_moisture_sensors; i++)
		sum += moisture[i];
		
	return sum / num_of_moisture_sensors;
}
	
//prints all attributes of the given frame
void print_frame(int frame_number, struct frame frames[]) {
	printf("julian: \t\t\t%s\n", frames[frame_number].julian);
	printf("date: \t\t\t\t%s\n", frames[frame_number].date);
	printf("node id: \t\t\t%s\n", frames[frame_number].node_id);
	printf("moisture sensor 1: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[0]);
	printf("moisture sensor 2: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[1]);
	printf("moisture sensor 3: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[2]);
	printf("average moisture: \t\t%d\n",  frames[frame_number].data.moisture_sensor_average_value);
	printf("temperature sensor: \t\t%d\n", frames[frame_number].data.temperature_sensor_value);
}