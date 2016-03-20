#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define FRAMES_TO_GET 5
#define NUM_OF_MOISTURE_SENSORS 3

#define DELIM "\t"
#define FILE_NAME "soildata.txt"

struct data {
	int moisture_sensor_value[NUM_OF_MOISTURE_SENSORS];
	int moisture_sensor_average_value;
	int temperature_sensor_value;
};

struct frame {
	struct data data;
	char node_id[100];
	char date[100];
	char julian[100];
};

static const struct frame empty_frame;

void frame_init(struct frame frame);
int get_frame_count(char * filename);
int get_last_frames(char *, int, struct frame*);
int get_all_frames(char * , struct frame *, int , int);
struct frame get_frame(char *, int, struct frame*);
int line_check(char * );
struct frame get_average(struct frame[], int);
int moisture_average(int[], int);
void print_frame(int, struct frame[]);

