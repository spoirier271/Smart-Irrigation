/***************************************
This file contains the functions used to
parse the soil data file.
****************************************/

#include "db_frames.h"

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

int get_last_frames(char * db_name, char * table_name, int requested_num_of_frames, char * sql_results[]) {
	int rc, record_count, column_count, total_entries;
	char sql_query[100], * error_msg, temp[100];
	sqlite3 * db;
	
	total_entries = get_all_frames(db_name, table_name, NULL);
	rc = sqlite3_open(db_name, &db);
	
	if(rc != SQLITE_OK) {
		printf("Unable to read from database\n");
	}
	
	int total_frames = total_entries / 8;
	
	int start_frame = total_frames - requested_num_of_frames;
	
	char ** res;
	strcpy(sql_query, "select * from ");
	strcat(sql_query, table_name);
    strcat(sql_query, " where rowid > ");
	sprintf(temp, "%d", start_frame);
	strcat(sql_query, temp);
	strcat(sql_query, ";");
	
	sqlite3_get_table(db, sql_query, &res, &record_count, &column_count, &error_msg);
	
	int count;
	int roww_count = 0;
	for(count = 0; count < record_count; count++) {
		strcpy(&sql_query[count], res[count]);
		if( ((count+1) % 8) == 0) {
			roww_count++;
		}
	}

	return 0;
}

//stores all frames from table into sql_results and returns total number of frames
int get_all_frames(char * db_name, char * table_name, char ** sql_results) {
	char sql_query[100], * error_msg;
	sqlite3 * db;
	int rc, record_count, column_count;
	
	rc = sqlite3_open(db_name, &db);
	
	if(rc != SQLITE_OK) {
		printf("Unable to read from database\n");
	}
	
	strcpy(sql_query, "select * from ");
	strcat(sql_query, table_name);
	strcat(sql_query, ";");
	
	sqlite3_get_table(db, sql_query, &sql_results, &record_count, &column_count, &error_msg);
	
	return record_count;
}

//fill single frame with given value from line in file
struct frame get_frame(char * line, int frame_number, struct frame *frame) {
	char temp[1000];

	//check for valid frame
	if(line_check(line) == 0) {

		//fill empty frame
		strcpy(frame->julian, "empty");
		strcpy(frame->date, "empty");
		strcpy(frame->node_id, "empty");
		strcpy(frame->time, "empty");
		
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
	strcpy(frame->time, strtok(NULL, DELIM));
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
	printf("time: \t\t\t\t%s\n", frames[frame_number].time);
	printf("node id: \t\t\t%s\n", frames[frame_number].node_id);
	printf("moisture sensor 1: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[0]);
	printf("moisture sensor 2: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[1]);
	printf("moisture sensor 3: \t\t%d\n", frames[frame_number].data.moisture_sensor_value[2]);
	printf("average moisture: \t\t%d\n",  frames[frame_number].data.moisture_sensor_average_value);
	printf("temperature sensor: \t\t%d\n", frames[frame_number].data.temperature_sensor_value);
}

void print_all_database_entries(char * db_name, char * table_name) {
	char ** sql_results, sql_query[100], * error_msg;
	sqlite3 * db;
	int rc, record_count, column_count, i, column_index = 1, total_entries, row_count = 0;
	
	rc = sqlite3_open(db_name, &db);
	
	if(rc != SQLITE_OK) {
		printf("Unable to read from database\n");
	}
	
	strcpy(sql_query, "select * from ");
	strcat(sql_query, table_name);
	strcat(sql_query, ";");
	
	sqlite3_get_table(db, sql_query, &sql_results, &record_count, &column_count, &error_msg);
	
	total_entries = record_count * column_count;
	
    for(i = 0; i < (total_entries); i++) {
    	if( (i == 0) || (i == 3) ) {
    		printf("%s\t\t", sql_results[i]);
    	} else if(i == 1){
    		printf("%s\t\t\t", sql_results[i]);
    	} else if(i == 2){
    		printf("%s\t\t\t", sql_results[i]);
    	} else if( (i == 4) || (i == 5) || (i == 6) ){
    		printf("%s\t", sql_results[i]);
    	} else if(i == 7){
    		printf("%s\n\n", sql_results[i]);
    	} else if (strlen(sql_results[i]) < 16) {
    		printf("%s\t\t", sql_results[i]);
    	} else {
    		printf("%s\t", sql_results[i]);
    	}
    	if(column_index == column_count) {
    		column_index = 0;
    		if(row_count > 0)
    			printf("record number: %d\n", row_count);
    		row_count++;
    	}
    	column_index++;
    }
    printf("\n");
}

char * frame_table[FRAMES_TO_GET][TOTAL_COLUMNS];
int row_index = 0;
int rows_read = 0;

struct frame (*frames)[];

void set_frames(struct frame (*fr)[]) {
	frames = fr;
}

int callback(void *data, int argc, char **argv, char **azColName){
	char line[10000];
	
	sprintf(line, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s", argv[0], argv[1], argv[2], argv[3], argv[4], 
		argv[5], argv[6], argv[7]);
			

	get_frame(line, row_index, &((*frames)[row_index]));

	row_index++;
	rows_read++;
	if(row_index >= FRAMES_TO_GET) {
		row_index = 0;
	}

   return 0;
}

int get_frame_table_row_count() {
	if(rows_read > FRAMES_TO_GET) {
		return FRAMES_TO_GET;
	}
	return rows_read;
}

int get_frames(struct frame *frames) {
	sqlite3 *db;
   	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = "Callback function called";

	/* Open database */
	rc = sqlite3_open(DATABASE_NAME, &db);
	if( rc ){
	  fprintf(stdout, "Can't open database: %s\n", sqlite3_errmsg(db));
	}else{
	  fprintf(stdout, "Opened database %s successfully\n", DATABASE_NAME);
	}

	/* Create SQL statement */
	sql = "SELECT * from entry";

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	if( rc != SQLITE_OK ){
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	}else{
	  fprintf(stdout, "Database successfully queried\n");
	}
	sqlite3_close(db);

	return get_frame_table_row_count();
}
