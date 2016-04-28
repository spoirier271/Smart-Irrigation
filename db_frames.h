#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sqlite3.h"

#define FRAMES_TO_GET 96
#define NUM_OF_MOISTURE_SENSORS 3
#define DATABASE_NAME "../irrigation_data.db"
#define TABLE_NAME "entry"
#define TOTAL_COLUMNS 8

#define DELIM "\t"

struct data {
	int moisture_sensor_value[NUM_OF_MOISTURE_SENSORS];
	int moisture_sensor_average_value;
	int temperature_sensor_value;
};

struct frame {
	struct data data;
	char node_id[100];
	char time[100];
	char date[100];
	char julian[100];
};

static const struct frame empty_frame;

void frame_init(struct frame frame);
struct frame get_frame(char *, int, struct frame*);
int line_check(char * );
struct frame get_average(struct frame[], int);
int moisture_average(int[], int);
void print_frame(int, struct frame[]);

void print_all_database_entries(char *, char *);
int get_last_frames(char *, char *, int, char*[]);
int get_all_frames(char *, char *, char **);

extern int callback(void *, int, char **, char **);
// char *[][] get_frame_table();
int get_frame_table_row_count();

int get_frames(struct frame *);
void set_frames(struct frame (*)[]);

