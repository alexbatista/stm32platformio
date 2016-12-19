/*
Library commands for ESP8266
*/
#include <stdio.h>
#include <stdarg.h>

/*Base AT commands*/
#define AT = "AT" //Startup test
#define RST = "AT+RST" //restart esp
#define INFO_VERSION = "AT+GMR" //See information about the version of the esp
#define ALT_ECHO = "ATE" //Alternate echo
#define ENABLE_ECHO = "ATE1" //enable echo
#define DISABLE_ECHO = "ATE0" //disable echo
#define RESTORE = "AT+RESTORE" //Restore factory settings

char command[512];
//variable arguments example
// char* mountCommand(int num,...){
//   va_list valist;
//   char* base[256];
//   int i;
//
//    /* initialize valist for num number of arguments */
//    va_start(valist, num);
//
//    /* access all the arguments assigned to valist */
//    for (i = 0; i < num; i++) {
//       base = va_arg(valist, int);
//       i++;
//    }
//
//    /* clean memory reserved for valist */
//    va_end(valist);
//
//    return sum/num;
// }
//mountCommand(RESTORE,40,40);

//Initialize deep sleep function
char* deepSleep(int time){
  sprintf(command,"AT+GSLP=%d",time);
  return command;
}

//Serial Port configurations
char* configUART(unsigned int baudrate, int databits,float stopbits, unsigned int parity, unsigned int flowControl){
  sprintf(command,"AT+UART=%u,%d,%f,%u,%u",baudrate,databits,stopbits,parity,flowControl);
  return command;
}


/********
* WiFI AT CONFIGURATIONS
*********/

/*WiFi Mode Options
* 1 - STA
* 2 - AP
* 3 - BOTH
* response command: The mode chosen. ex: STA
*/
char* WiFiMode(int mode){
  sprintf(command,"AT+CWMODE=%d",mode);
  return command;
}

char* joinAccessPoint(char *ssid, char *password){
  sprintf(command,"AT+CWJAP=%s,%s",ssid,password);
  return command;
}

char* listAllAccessPoint(){
  sprintf(command,"AT+CWLAP");
  return command;
}

char* listAccessPoint(char* ssid, char* mac, char* channel){
  sprintf(command,"AT+CWLAP=%s,%s,%s",ssid,mac,channel);
  return command;
}

char* quitAccessPoint(){
  sprintf(command,"AT+CWQAP");
  return command;
}

char* setParametersAccessPoint(char* ssid, char* pwd, char* channel, char* encryption){
  sprintf(command,"AT+CWSAP=%s,%s,%s,%s",ssid,pwd,channel,encryption);
  return command;
}

/*
*return the ip address of stations which are connected to ESP8266 softAP
*/
char* ipStationsConnected(){
  sprintf(command,"AT+CWLIF");
  return command;
}
