#include "mbed.h"
#include "states.h"
#include <string.h>


Serial pc(USBTX, USBRX);
Serial esp(A0,A1);
InterruptIn bt(USER_BUTTON);
AnalogIn Ain(A5);


char host [] = "api.thingspeak.com";
char server[] = "GET /update?api_key=06R4I3BAROD02GKV&field1";
//  char server[] = "GET /update?api_key=06R4I3BAROD02GKV&field1=30 HTTP/1.1\r\nHost: api.thingspeak.com";
char ssid[32] = "IC";     // enter WiFi router ssid inside the quotes
char pwd [32] = "icomputacaoufal"; // enter WiFi router password inside the quotes

char port [] = "80";
char tcp [] = "TCP";

int count = 0, ended, timeout = 2;
char buf[1024] = {0};
char snd[255];
char * m;
int flag;
int rsp;
int analog;

int flag_close;
int go = 0;
void mySleep();
void send(char* command, int target){
  if(target == T_ESP){
    esp.printf("%s\r\n",command);
  }else if(target == T_PC){
    pc.printf("%s\r\n",command);
  }
  mySleep();
}
void Reset(){
  // Mexer no whatchdog
  char str[1024];
  //sprintf(str,"AT+RST");
  sprintf(str,"AT");
  //pc.printf("STR = %s\r\n",str);
  send(str,T_ESP); //set wifi-mode client

}

void TypeSingleConnection(){
  char str[1024];
  sprintf(str,"AT+CIPMUX=0");
  send(str,T_ESP);
}

void WiFiMode(){
  char str[1024];
  sprintf(str,"AT+CWMODE=1");
  send(str,T_ESP);
  TypeSingleConnection();
}

void Connect(){
  char str[1024];
  sprintf(str,"AT+CWJAP=\"%s\",\"%s\"",ssid,pwd);
  send(str,T_ESP);

}
void GetIP(){
  char str[1024];
  sprintf(str,"AT+CIFSR");
  send(str,T_ESP);
}


void OpenTCP(){
  char str[1024];
  sprintf(str,"AT+CIPSTART=\"%s\",\"%s\",%s",tcp,host,port);
  send(str,T_ESP);

}
void SendTCP(){
  char str[1024];
  sprintf(str,"%s=%d HTTP/1.0",server,analog);
  sprintf(str,"AT+CIPSEND=%d",strlen(str)+4);
  send(str,T_ESP); //set wifi-mode client
}
void SendDataTCP(){
  //pegar leitura do  analogico
  char str[1024];
  sprintf(str,"%s=%d HTTP/1.0\r\n",server,analog);
  printf("AQUI: %s\n", str);
  send(str,T_ESP);
}

void CloseTCP(){
  char str[1024];
  sprintf(str,"AT+CIPCLOSE");
  send(str,T_ESP);
}


void rxHandler();
void opca();
void IRQButton();
_STATE state = INIT;
int main(){
  esp.baud(115200);
  pc.baud(115200);

  esp.attach(rxHandler,Serial::RxIrq);
  bt.rise(NULL);

  pc.printf("Initi RST\n");
  /*wait(1);
  //Reset();
  esp.printf("AT\r\n");
  STATUS = RST;
  wait(3);
  */

  while(1){

    pc.printf("_______________");
    pc.printf("\nBUF= %s\n",buf);
    pc.printf("RSP = %d\n",rsp);
    pc.printf("STATE = %d\n",state);
    pc.printf("Values: %u\n",Ain*100);
    memset(buf, 0, sizeof(buf));
    count=0;
    //wait(2);
    opca();
    //sleep();

  }


}
void mySleep(){
  printf("sleeping\n");
    while(!go){
      sleep();
    }
    go = 0;
}

void rxHandler(){
  // 0 -ok, 1 - error, 2 - `>` send return
  //rsp
  //wait(0.05);
  while(esp.readable()) {
    buf[count++] = esp.getc();
  }

  flag = 0;
  rsp = ERROR;
  for(int i =0; i < count;i++ ){
    if (buf[i] == '>'){
      rsp = OK_SEND;
      flag = 1;
      break;
    }
  }
  if(flag == 0){
    for(int i =0; i < count-1;i++ ){
      if ( buf[i] == 'O'){
        if(buf[i+1] == 'K'){
          rsp = OK;
          go=1;
          break;
        }
      }
    }
  }
}


void opca(){
  switch (state) {
    case INIT:
    Reset();
    state = W_INIT;
    //sleep();
    break;
    case W_INIT:
    if(rsp == OK){
      state = WIFIMODE;
    }
    else{
      state = INIT;
    }
    //sleep();
    break;
    case WIFIMODE:
    WiFiMode();
    state = W_WIFIMODE;
    //sleep();
    break;
    case W_WIFIMODE:
    if(rsp == OK){
      state = CONNECT;
    }
    else{
      state = INIT;
    }
    //sleep();
    break;
    case CONNECT:
    Connect();
    state = W_CONNECT;
    //sleep();
    break;

    case W_CONNECT:
    //sleep();
    //wait(5);
    if(rsp == OK){
      state = GETIP;
    }
    else{
      state = CONNECT;
    }
    break;
    case GETIP:
    GetIP();
    state = W_GETIP;
    //sleep();
    break;
    case W_GETIP:
    if(rsp == OK){
      state = ENABLE_BUTTON;
    }
    else{
      //.Reset();
      state = GETIP;
    }

    break;
    case ENABLE_BUTTON:
    bt.rise(&IRQButton);
    state = CONNECTED;
    break;
    case CONNECTED:
    //bt.rise(&IRQButton);
      mySleep();
    break;

    case OPENTCP:
    OpenTCP();
    state = W_OPENTCP;
    //sleep();
    break;
    case W_OPENTCP:
    if(rsp == OK){
      //SendTCP();
      state = SENDTCP;
    }
    else{
      //OpenTCP();
      flag_close = CLOSE_ERROR;
      state = CLOSETCP;
    }
    break;
    case SENDTCP:
    SendTCP();
    state = W_SENDTCP;
    //sleep();
    break;
    case W_SENDTCP:
    if(rsp == OK_SEND){ // >
      //SendDataTCP();
      state = SENDDATATCP;
    }
    else if(rsp == ERROR){ // error
      //SendTCP();
      flag_close = CLOSE_ERROR;
      state = CLOSETCP;
    }
    break;
    case SENDDATATCP:
    SendDataTCP();
    state = W_SENDDATATCP;
    //sleep();
    break;
    case W_SENDDATATCP:
    if(rsp == OK){ // >
      flag_close = CLOSE_OK;
      state = CLOSETCP;
    }
    else { // error
      flag_close = CLOSE_ERROR;
      state = CLOSETCP;
    }
    break;

    case CLOSETCP:
    CloseTCP();
    state = W_CLOSETCP;
//sleep();
    break;
    case W_CLOSETCP:
    if(rsp == OK){ // >
      if(flag_close == CLOSE_OK){
        state = ENABLE_BUTTON;
      }else{
        state = OPENTCP;
      }
    }
    else { // error

      state = ENABLE_BUTTON;
    }
    break;

  }
}

void IRQButton(){
  bt.rise(NULL);
  analog = Ain*100;
  //analog = 99;
  state = OPENTCP;
  go = 1;
}
