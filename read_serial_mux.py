#!/usr/bin/python

# read_serial.py
#
# Daniel Tran - dagtran@ucsc.edu
# Jonathan Yang - jyang37@ucsc.edu
# 
# This program reads in binary data from an XBee S1 using the DigiMesh
# protocol as a serial device. The data is then converted into hexadecimal
# and then into a Unicode string to facilitate extraction of relevant information.
# The data printed to the console, and then sent to a local SQLite3 database, 
# as well as a MySQL database.
#
# If data cannot be sent because the internet access is hampered,
# the unsent entries will be saved to a file ("resend.txt"), and the
# program will try to resend those entries at a later time

import binascii
import datetime
import mysql.connector
import os
import select
import serial
import sqlite3
import sys
import time

#SQLite DATABASE
#-----------------------------------------------------------------------------------------
# Check to see if the database exists, otherwise, create and initialize it
# Code is from the Python documentation

def check_sqlite_db():
    # Execute following instructions if the file "irrigation_data.db"
    # does not exist
    if not os.path.isfile("irrigation_data2.db"):
        # Create database
        conn = sqlite3.connect('irrigation_data2.db')
        c = conn.cursor()

        # Create table in database
        c.execute('''CREATE TABLE entry2
                 (julian_time text, date text, time text, stake_id text, sensor_1 integer, sensor_2 integer, sensor_3 integer, temp integer, temp_2 integer)''')

        # Save (commit) the changes
        conn.commit()   

        # We can also close the connection if we are done with it.
        # Just be sure any changes have been committed or they will be lost.
        conn.close()

# Code is from the Python documentation
def sqlite_insert(julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2):
    # Collecting variables to be used
    values = (julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2)

    # Query that will be used to put data into database
    query = "INSERT INTO entry2 VALUES " + str(values)

    # Connect to database
    conn = sqlite3.connect('irrigation_data2.db')
    c = conn.cursor()

    # Insert a row of data
    c.execute(query)

    # Save (commit) the changes
    conn.commit()

    # We can also close the connection if we are done with it.
    # Just be sure any changes have been committed or they will be lost.
    conn.close()


def check_sqlite_db_mux():
    # Execute following instructions if the file "irrigation_data.db"
    # does not exist
    if not os.path.isfile("irrigation_data_mux.db"):
        # Create database
        conn = sqlite3.connect('irrigation_data_mux.db')
        c = conn.cursor()

        # Create table in database
        c.execute('''CREATE TABLE entrymux
                 (julian_time text, date text, time text, stake_id text, m0p3 integer, m1p3 integer, m2p3 integer, m3p3 integer, m0p2 integer, m1p2 integer, m2p2 integer, m3p2 integer, m0p1 integer, m1p1 integer, m2p1 integer, m3p1 integer, m0p0 integer, m1p0 integer, m2p0 integer, m3p0 integer, pin_5 integer)''')

        # Save (commit) the changes
        conn.commit()   

        # We can also close the connection if we are done with it.
        # Just be sure any changes have been committed or they will be lost.
        conn.close()

# Code is from the Python documentation
def sqlite_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5):
    # Collecting variables to be used
    values = (julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)

    # Query that will be used to put data into database
    query = "INSERT INTO entrymux VALUES " + str(values)

    # Connect to database
    conn = sqlite3.connect('irrigation_data_mux.db')
    c = conn.cursor()

    # Insert a row of data
    c.execute(query)

    # Save (commit) the changes
    conn.commit()

    # We can also close the connection if we are done with it.
    # Just be sure any changes have been committed or they will be lost.
    conn.close()




#MYSQL DATABASE with 5 entries
#-----------------------------------------------------------------------------------------
# Code is from the MySQL documentation
def mysql_insert(julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2):
    # We collect all the variables to be used by the query
    entry = (julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2)

    # This is the SQL query we will use to add entries into the "entry" table
    add_entry = ("INSERT INTO entry2 "
                 "(julian_time, date, time, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2) "
                 "VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)")

    # Connect to db
    #
    # It would probably be better to have this information in a config
    # file and then parse the config file for the information
    cnx = mysql.connector.connect(host = 'smartirrigation.cxiuxo3jrzjd.us-west-1.rds.amazonaws.com',
                                  user='sdp', 
                                  password='software',
                                  database='smartirrigation')
    cursor = cnx.cursor()

    # Insert new entry
    cursor.execute(add_entry, entry)

    # Make sure data is committed to the database
    cnx.commit()

    # Close connection
    cursor.close()
    cnx.close()

# If we cannot send an entry to the database, we write it to "resend.txt"
# and try sending it later
def resend_insert(julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2):
    # Collect values for the entry
    entry = "%s %s %s %s %s %s %s %s %s" % (julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2)

    # Open "resend.txt", write the entry, then close the file
    f = open("resend.txt", "a")
    f.write(entry + "\n")
    f.close

# Attempts to send information to MySQL database from "resend.txt".
# If successful, "resend.txt" is cleared. If not, nothing is done.
def resend_entries():
    try:
        # If there is nothing in "resend.txt", we do nothing and exit
        # the function
        if os.stat("resend.txt").st_size == 0:
            return

    # If "resend.txt" does not exist, do nothing and exit the function
    except OSError:
        return

    # Open file for reading and (later) writing
    f = open("resend.txt", "r+")

    # Iterate through the file, line by line, and send the data to
    # the database
    for line in f:
        julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2 = line.split()

        try:
            mysql_insert(julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2)
            print "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % (julian_time, date, timestamp, stake_id, sensor_1, sensor_2, sensor_3, temp, temp_2, "RE-SENT")
        # If we cannot resend the data, do nothing and try again later
        except mysql.connector.errors.InterfaceError:
            return

    # Delete file contents after all entries have been sent
    f.seek(0)
    f.truncate()

    # Close file
    f.close()
    
    
    
    

#MYSQL MUX DATABASE ENTRY 
#-----------------------------------------------------------------------------------------
def mysql_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5):
    # We collect all the variables to be used by the query
    entry = (julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)

    # This is the SQL query we will use to add entries into the "entry" table
    add_entry = ("INSERT INTO entrymux "
                 "(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5) "
                 "VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)")

    # Connect to db
    #
    # It would probably be better to have this information in a config
    # file and then parse the config file for the information
    cnx = mysql.connector.connect(host = 'smartirrigation.cxiuxo3jrzjd.us-west-1.rds.amazonaws.com',
                                  user='sdp', 
                                  password='software',
                                  database='smartirrigation')
    cursor = cnx.cursor()

    # Insert new entry
    cursor.execute(add_entry, entry)

    # Make sure data is committed to the database
    cnx.commit()

    # Close connection
    cursor.close()
    cnx.close()

# If we cannot send an entry to the database, we write it to "resend.txt"
# and try sending it later
def resend_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5):
    # Collect values for the entry
    entry = "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s" % (julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)

    # Open "resend.txt", write the entry, then close the file
    f = open("resend_mux.txt", "a")
    f.write(entry + "\n")
    f.close

# Attempts to send information to MySQL database from "resend.txt".
# If successful, "resend.txt" is cleared. If not, nothing is done.
def resend_entries_mux():
    try:
        # If there is nothing in "resend.txt", we do nothing and exit
        # the function
        if os.stat("resend_mux.txt").st_size == 0:
            return

    # If "resend.txt" does not exist, do nothing and exit the function
    except OSError:
        return

    # Open file for reading and (later) writing
    f = open("resend_mux.txt", "r+")

    # Iterate through the file, line by line, and send the data to
    # the database
    for line in f:
        julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5 = line.split()

        try:
            mysql_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)
            print "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % (julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5, "RE-SENT")
        # If we cannot resend the data, do nothing and try again later
        except mysql.connector.errors.InterfaceError:
            return

    # Delete file contents after all entries have been sent
    f.seek(0)
    f.truncate()

    # Close file
    f.close()




#DATA PROCESSING
#-----------------------------------------------------------------------------------------
def process_data(device):
    count2 = 0
    try:
        # Reads 1024 bytes from the XBee, converts the data from the XBee 
        # into hex, then casts it as a string so string manipulation operations 
        # can be done on the received data
        string = str(binascii.b2a_hex(device.read(1024)))
    # If the XBee is suddenly disconnected from the Raspberry Pi, 
    # the program exits
    except IOError as e:
        error_string = "%s: %s %s" % (sys.argv[0], e.strerror, "(XBee may have been disconnected)")
        sys.exit(error_string)

    # Break string into chunks of 60 chars and put them into a list
    chunks = [string[i:i + 60] for i in range(0, len(string), 60)]

    # Process each chunk in the list
    for chunk in chunks:
        # We make sure that each chunk is a IO Data Sample
        # IO Data Sample frames are 30 bytes (60 hex chars) 
        # because all 5 ADC pins are active. If not all the ADC
        # pins are active, the frame size will be smaller.
        # Each reading from an ADC pin is 2 bytes in size
        print count
        if len(chunk) == 60:
            # Obtain time for log
            julian_time = time.time()
            
            if count == 0:
                if count2 % 2 != 0:
             	    count2 += 1
            	    continue
                count2 +=1	

            #Create a counter that starts at 0 and will go up to 4. At 0 break the code
            #At 1-4 continue and get the data.
            
            # Convert Julian time to standard time and date format
            date = datetime.datetime.fromtimestamp(julian_time).strftime('%Y-%m-%d')
            timestamp = datetime.datetime.fromtimestamp(julian_time).strftime('%H:%M:%S')

            # We take parts of the chunk at specific offsets for 
            # relevant data
            stake_id = chunk[8:24].upper() # stake_id, capitalized

            # We use a base of 16 to interpret the string slice as a 
            # base 16 (hex) value, and then convert to a decimal
            # integer
            #THIS IS USED IF PIN 5 IS A DIGITAL OUTPUT HIGH
            #pin1 = int(chunk[38:42], 16) # pin 1
            #pin2 = int(chunk[42:46], 16) # pin 2
            #pin3 = int(chunk[46:50], 16) # pin 3
            #pin4 = int(chunk[50:54], 16) # pin 4
            #pin_5 = int(chunk[54:58], 16) # pin 5
            
            pin1 = int(chunk[38:42], 16) # pin 5
            pin2 = int(chunk[42:46], 16) # pin 1
            pin3 = int(chunk[46:50], 16) # pin 2
            pin4 = int(chunk[50:54], 16) # pin 3
            pin5 = int(chunk[54:58], 16) # pin 4
            
            # If the readings are less than 0 or greater than 1023, then
            # something has gone wrong with the frame because the ADC
            # readings from the XBee should never be below 0 or greater
            # than 1023. If that is the case, we move onto the next chunk
            if (pin1 or pin2 or pin3 or pin4 or pin5) > 1023 or\
               (pin1 or pin2 or pin3 or pin4 or pin5) < 0:
                continue

            pin_1st = (((float(pin1)/1023)*3.3)) #moisture volt
            pin_2nd = float((float(pin2)/1023)*3.3) #temperature volt                  
            pin_3rd = float((float(pin3)/1023)*3.3) #temperature volt (will be humidity) 
            pin_4th = float((float(pin4)/1023)*3.3) # temperature volt (any sensor)
            pin_5th = float((float(pin5)/1023)*3.3) # temperature volt (not used)            
            
            pin_1ohm = (((.85/(pin_1st))*4700)-4700) #pin 1 ohms
            #pin_2ohm = (((.85/(pin_2nd))*4700)-4700) #pin 2 ohm if needed
            #pin_3ohm = (((.81/(pin_3rd))*4700)-4700) #pin 3 ohm if needed
            

            if pin_1ohm >= 0 and pin_1ohm < 1068: #pin 1 conversions to kPA
                 pin_1 = (pin_1ohm*.019) - 10.305
                 #pin_1 = 100 - ((pin_11/199) * 100)
            elif pin_1ohm >= 1068 and pin_1ohm < 6944:
                 pin_1 = (pin_1ohm*.0051) + 4.581
                 #pin_1 = 100 - ((pin_11/199) * 100)
            elif pin_1ohm >= 6944 and pin_1ohm < 9127:
                 pin_1 = (pin_1ohm*.0063) - 2.5
                 #pin_1 = 100 - ((pin_11/199) * 100)
            elif pin_1ohm >= 9127 and pin_1ohm < 12139:
                 pin_1 = (pin_1ohm*.0067) - 6.33
                 #pin_1 = 100 - ((pin_11/199) * 100)
            elif pin_1ohm >= 12139 and pin_1ohm < 32000:
                 pin_1 = (pin_1ohm*.0079) - 23.055
                 #pin_1 = 100 - ((pin_11/199) * 100)
                 
            elif pin_1ohm > 32000:
                 print "Extreme Dry Case P1"
                 pin_1 = 199
                 #continue 
            elif pin_1ohm < 0:
                 print "Extreme Wet Case P1"
                 pin_1 = 0
                 #continue
                       
            #RAN IF PIN 2 IS USED FOR MOISTURE             
            #if pin_2ohm >= 0 and pin_2ohm < 1068: #pin 2 conversions to kPA
            #     pin_2 = (pin_2ohm*.019) - 10.305
            #     #pin_2 = 100 - ((pin_22/199) * 100)
            #elif pin_2ohm >= 1068 and pin_2ohm < 6944:
            #     pin_2 = (pin_2ohm*.0051) + 4.581
            #     #pin_2 = 100 - ((pin_22/199) * 100)
            #elif pin_2ohm >= 6944 and pin_2ohm < 9127:
            #     pin_2 = (pin_2ohm*.0063) - 2.5
            #     #pin_2 = 100 - ((pin_22/199) * 100)
            #elif pin_2ohm >= 9127 and pin_2ohm < 12139:
            #     pin_2 = (pin_2ohm*.0067) - 6.33
            #     #pin_2 = 100 - ((pin_22/199) * 100)
            #elif pin_2ohm >= 12139 and pin_2ohm < 32000:
            #     pin_2 = (pin_2ohm*.0079) - 23.055  
            #     #pin_2 = 100 - ((pin_22/199) * 100)  
                 
            #elif pin_2ohm > 32000:
            #     print "Extreme Dry Case P2"
            #     pin_2 = 199
                 #continue 
            #elif pin_2ohm < 0:
            #     print "Extreme Wet Case P2"
            #     pin_2 = 0
                 #continue 
                      
            # RAN IF PIN 3 IS USED FOR MOISTURE
            #if pin_3ohm >= 0 and pin_3ohm < 1068: #pin 3 conversions to kPA
            #     pin_3 = (pin_3ohm*.019) - 10.305
            #     #pin_3 = 100 - ((pin_33/199) * 100)   
            #elif pin_3ohm >= 1068 and pin_3ohm < 6944:
            #     pin_3 = (pin_3ohm*.0051) + 4.581
            #     #pin_3 = 100 - ((pin_33/199) * 100)   
            #elif pin_3ohm >= 6944 and pin_3ohm < 9127:
            #     pin_3 = (pin_3ohm*.0063) - 2.5
            #     #pin_3 = 100 - ((pin_33/199) * 100)   
            #elif pin_3ohm >= 9127 and pin_3ohm < 12139:
            #     pin_3 = (pin_3ohm*.0067) - 6.33
            #     #pin_3 = 100 - ((pin_33/199) * 100)   
            #elif pin_3ohm >= 12139 and pin_3ohm < 32000:
            #     pin_3 = (pin_3ohm*.0079) - 23.055
            #     #pin_3 = 100 - ((pin_33/199) * 100)    
            
            #elif pin_3ohm > 32000 or pin_3ohm < 0:
            #     pin_3 = 0     
            
            #Extreme Cases
            if pin_1 < 0:
                 print "Extreme Wet Case P1"
                 pin_1 = 0
                 #continue   
                             
            #if pin_2 < 0:
            #     print "Extreme Wet Case P2"
            #     pin_2 = 0
                 #continue   
                 
            pin_2 = float((pin_2nd* 100) - 273.15) #temperature kelvin to celcius                                                                        
            pin_4 = float((pin_4th* 100) - 273.15) #temperature kelvin to celcius
            #pin_3 = float((pin_3rd* 100) - 273.15)
            pin_3 = float(pin_3rd/.03)	
            pin_5 = float((pin_5th* 100) - 273.15)		
            # We format the obtained data as a string, delimited by tabs
            data = "%s\t%s\t%s\t%s\t%s kPa \t%s kPa \t%s C \t%s C \t%s C" % (julian_time, date, timestamp, stake_id, round(pin_1, 2), round(pin_2, 2), round(pin_3, 2), round(pin_4, 2), round(pin_5, 2))
                        
            #Format data to be printed
            pin_1p = 100 - ((pin_1/199) * 100) 
            #pin_2p = 100 - ((pin_2/199) * 100) if you want percentage with pin2  
            #pin_3p = 100 - ((pin_3/199) * 100)  
            #printdata = "%s\t%s\t%s\t%s\t%s kPa %s %% \t%s kPa %s %% \t%s kPa %s %% \t%s C" % (julian_time, date, timestamp, stake_id, round(pin_1, 2), round(pin_1p, 2),round(pin_2, 2), round(pin_2p, 2),round(pin_3, 2), round(pin_3p, 2), round(pin_4, 2))
            
            printdata = "%s\t%s\t%s\t%s\t%s kPa %s %% \t%s C \t\t%s RH \t%s C      %s C" % (julian_time, date, timestamp, stake_id, round(pin_1, 2), round(pin_1p, 2),round(pin_2, 2),round(pin_3, 2), round(pin_4, 2), round(pin_5, 2))

            f = open("soildata.txt", "a")
            f.write(data + "\n")
            f.close()

			#4 different if statements depending on the counter that was produced at the beginning of the code.
            if count == 0:
                print "Julian Time\tDate\t\tTime\t\tID\t\t\tM1P3\t\t\tM2P3\t\t\tM3P3\t\tM4P4\t    Pin 5"
                global m0p3
                m0p3 = pin_1
                global m1p3
                m1p3 = pin_2
                global m2p3
                m2p3 = pin_3
                global m3p3
                m3p3 = pin_4		
            if count == 1:
                print "Julian Time\tDate\t\tTime\t\tID\t\t\tM1P2\t\t\tM2P2\t\t\tM3P2\t\tM4P2\t    Pin 5"
                global m0p2
                m0p2 = pin_1
                global m1p2
                m1p2 = pin_2
                global m2p2
                m2p2 = pin_3
                global m3p2
                m3p2 = pin_4	
            if count == 2:
                print "Julian Time\tDate\t\tTime\t\tID\t\t\tM1P1\t\t\tM2P1\t\t\tM3P1\t\tM4P1\t    Pin 5"
                global m0p1
                m0p1 = pin_1
                global m1p1
                m1p1 = pin_2
                global m2p1
                m2p1 = pin_3
                global m3p1
                m3p1 = pin_4
            if count == 3:
                print "Julian Time\tDate\t\tTime\t\tID\t\t\tM1P0\t\t\tM2P0\t\t\tM3P0\t\tM4P0\t    Pin 5"
                global m0p0
                m0p0 = pin_1
                global m1p0
                m1p0 = pin_2
                global m2p0
                m2p0 = pin_3
                global m3p0
                m3p0 = pin_4
                sqlite_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)
                try:
                    mysql_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)
                    data += "\tOK"
                except mysql.connector.errors.InterfaceError:
                    resend_insert_mux(julian_time, date, timestamp, stake_id, m0p3, m1p3, m2p3, m3p3, m0p2, m1p2, m2p2, m3p2, m0p1, m1p1, m2p1, m3p1, m0p0, m1p0, m2p0, m3p0, pin_5)
                    data += "\tRESEND REQUIRED"
                           
            # We push the data we obtained to the local database
            sqlite_insert(julian_time, date, timestamp, stake_id, pin_1, pin_2, pin_3, pin_4, pin_5)

            try:
                # We push the data we obtained to the database in the web
                mysql_insert(julian_time, date, timestamp, stake_id, pin_1, pin_2, pin_3, pin_4, pin_5)

                # We add the status "OK" if the data was sent to the db
                # without problems
                data += "\tOK"

            # If we cannot connect to the MySQL database, we insert the entry that could not
            # be sent into "resend.txt", so we can try again later
            except mysql.connector.errors.InterfaceError:
                resend_insert(julian_time, date, timestamp, stake_id, pin_1, pin_2, pin_3, pin_4, pin_5)

                # We add the status "RESEND REQUIRED" if we could not
                # connect to the MySQL database
                data += "\tRESEND REQUIRED"

            # Print data and status
            print printdata

def main():
    global count
    count = 0
    try:
        # Open XBee board as a serial device
        ser = serial.Serial("/dev/ttyUSB0", 9600, timeout = .2)

    # If the XBee is not connected, print error   
    except OSError as e:
        error_string = "%s: %s %s" % (sys.argv[0], e.strerror, "(Is the XBee plugged in?)")
        sys.exit(error_string)

    # Check if local database exists, create if not
    check_sqlite_db()
    check_sqlite_db_mux()

    # Print niceties
    print "Data Logging Initialized"
    print "Connected to: " + ser.portstr
    #print "Julian Time\tDate\t\tTime\t\tID\t\t\tPin 1\t\t\tPin 2\t\t\tPin 3\t\t\tPin 4\tStatus"
    #below is print data for 2 temp and 2 moisture
    print "Julian Time\tDate\t\tTime\t\tID\t\t\tMoisture Mux\t\tTemp Mux\t\tHumidity Mux\t  Mux 4\t      Pin 5"

    # Continuously loop through the following instructions
    while True:
        # If there are things in "resend.txt", we attempt to resend
        # those entries
        resend_entries()
        resend_entries_mux()
 
        # Block until XBee receives data, return value (readable_device)
        # is a list of devices
        readable_device, _, _ = select.select([ser], [], [])

        # When the XBee is ready, we process the data
        #
        # Note: because we only have one device in our list of things
        # to wait for, the XBee will always be the first in the list,
        # that is why we can call the XBee as readable_device[0]
        process_data(readable_device[0])
        if count == 3:
            count = 0
        else:
            count += 1
if __name__ == '__main__':
    main()
