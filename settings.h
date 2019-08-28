/******************************************************************************
MIT License

Copyright (c) 2019 Cyrille Gaillardet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef _CGE_SETTINGS_H
#define _CGE_SETTINGS_H

/* DEFINES */
/* remove this line to be able to compile. Please do it once 
 *  mac address is set */

#error configure settings.h before compiling, then remove this line

/**************************** device nae, used for OTA, MQTT, mDNS,html ***********/
#define DEVICENAME "defaultDeviceNameNoSpace" 

/**************************** OTA **************************************************/
#define OTAPASSWORD "otapwd" //the password you will need to enter to upload remotely via the ArduinoIDE
#define OTAPORT 8800

/******************************* MQTT **********************************************/
#define MQTT_MAX_PACKET_SIZE 512

/******************************** DEBUG ********************************************/
/* comment these to disable debug. Note: files that do not include this header will not output logs */
#define NEED_LOG
#define NEED_QUEUE_LOG
/******************************** HARDWARE *******************************************/

/* PWM maximum value is defined as PWMRANGE on esp8266*/
#define MAX_PWM_COMMAND PWMRANGE
#define PWM_COMMANDED_ZERO_UNDER_COMMAND 5

/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) **************/

#define WIFISSID1 "0" 
#define WIFIPWD1 "0"
#define WIFISSID2 "0"
#define WIFIPWD2 "0"

#define WIFI_CONFIGURATION_SSID "confssid"
#define WIFI_CONFIGURATION_PWD "localconfssid"
#define WIFI_CONNECTION_TRIES_BEFORE_ERROR 30
#define WIFI_CONNECTION_TIMER_BETWEEN_TRIES_MS 60000
#define WIFI_FAILED_THRESHOLD 3

#define MQTT_SERVER_IP "0.1.2.3"
#define MQTT_USERNAME  "toto"
#define MQTT_PWD "pwd123"
#define MQTT_PORT 0
#define MQTT_WAIT_RECONNECT_COUNTER 50000




/************* MQTT TOPICS (change these topics as you wish)  **********************/
#define MQTT_PREFIX "aircon"

/**************************************** NETWORK **********************************/

// the media access control (ethernet hardware) address for the shield:
#define MAC_ADDRESS { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }

/**************************************** TIME **********************************/
#define NTP_SERVER "pool.ntp.org"
#define NTP_SERVER_PORT 123

/****************************** AIRCON SETTINGS *********************************/
#define DEFAULT_TEMP 25
#define DEFAULT_FAN_SPEED DAIKIN_FAN_AUTO
#define DEFAULT_POWER 0
#define DEFAULT_POWERFUL 0
#define DEFAULT_QUIET 0
#define DEFAULT_SWING_H 0
#define DEFAULT_SWING_V 0
#define DEFAULT_MODE DAIKIN_COOL

/********************************** WEBSERVERS **********************************/
#define WEBSERVER_PORT 80
#define CLOSE_SETUP_WEBSERVER_AFTER_MS 300000 /*5mn */

/********************************** EEPROM **************************************/
#define EEPROM_MAGIC_NUMBER_ADDR 0x0F
#define EEPROM_MAGIC_NUMBER_VALUE 0xDEADBEEF
#define EEPROM_MAGIC_NUMBER_SIZE 4
#define EEPROM_MIN_TIME_BETWEEN_SAVE_MS 30000 /* 30 seconds */



#endif 
