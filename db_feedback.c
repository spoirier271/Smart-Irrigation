#include "db_feedback.h"

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

double decide_watering_time(struct frame frame) {
	double moisture_inches, temp_inches, 
		moisture_time, temp_time, final_watering_time, watering_time_adjustment;
		
	//get inches adjustment for base watering time from moisture and temperature readings
	moisture_inches = get_moisture_watering_inches(frame);
	temp_inches = get_temperature_watering_inches(frame);
	
	//convert inches to actual weekly watering time
	moisture_time = convert_to_miliseconds(moisture_inches);
	temp_time = convert_to_miliseconds(temp_inches);

	//add weighted times together to produce time adjustment
	watering_time_adjustment = ( (0.8) * moisture_time) + ( (0.2) * temp_time);
	
	//adjust base watering time by calculated adjustment time
	final_watering_time = convert_to_miliseconds(BASE_WATERING_TIME_PER_DAY) + watering_time_adjustment;
	
	//return final watering duration time for today
	return final_watering_time;
		
}

//get watering inches adjustment from moisture data
double get_moisture_watering_inches(struct frame frame_average) {
	double m;
	
	//m is the actual watering inches that must be later converted to a watering time
	m = frame_average.data.moisture_sensor_average_value;
	
	//decide proper watering inches based on moisture reading
	
	//more than 100
	if(m >= 100) {
	
		return 1;	
	}
	
	//80-100
	else if( (m < 100) && (m >= 80) ) {
		
		return 0.5;
	}  
	
	//60-80
	else if( (m < 80) && (m >= 60) ) {
		
		return 0.25;
	} 
	
	
	/*********		IDEAL RANGE 	*********/
	//30-60
	else if( (m < 60) && (m >= 30) ) {
		
		return 0;
	} 
	/*********		IDEAL RANGE *********/
	
	//10-30
	else if( (m < 30) && (m >= 10) ) {
		
		return -0.25;
	}	
	
	//less than 10
	else {
		
		return -0.5;
	}	
}

//get watering time adjustment from temperature data
double get_temperature_watering_inches(struct frame frame_average) {
	double t;
	
	t = frame_average.data.temperature_sensor_average_value;
	
	//t is the actual water amount that must be converted to a watering time
	
	//above 50
	if(t >= 50) {
	
		return 0.75;
	} 
	
	//38-50
	else if( (t < 50) && (t >= 38) ) {

		return 0.5;
	} 
	
	//26-38
	else if( (t < 38) && (t >= 26) ) {
	
		return 0.25;
	} 
	
	/*********		IDEAL RANGE 	*********/
	//21-26
	else if( (t < 26) && (t >= 21) ) {
	
		return 0;
	} 
	/*********		IDEAL RANGE 	*********/
	
	//10-21
	else if( (t < 21) && (t >= 10) ) {
	
		return -0.25;
	} 
	
	//0-10
	else if( (t < 10) && (t >= 0) ){
	
		return -0.5;
	} 
	
	//below 0 degrees
	else {
	
		return  -0.75;
	}
}

//There is a 1 to 1 ratio between inches per week and hours per day. Need only convert from hours to mili-seconds.
double convert_to_miliseconds(double inches) {
	
	
	//1 hour = 60 minutes * 60 seconds * 1000 mili-seconds
	return inches * 60 * 60 * 1000;
	
}


void print_watering_time(double  t) {
	double hours, mins, secs;
	char time[1000];
	
	secs = t / 1000;
	mins = secs / 60;
	hours = mins / 60;
	
	if(hours >= 1) {
		mins = (int)mins - ((int)hours * 60);
		secs = secs - (hours * 60 * 60);
	}
	
	if(mins >= 1) {
		secs = secs - (mins * 60);
	}
	
	sprintf(time, "%s\t%dh%dm", "Calculated watering time:", (int)hours, (int)mins);
	
	printf("%s\n", time);
}
