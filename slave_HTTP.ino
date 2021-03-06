/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h> 

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
#define AGENT 2

int numOfNeighbours = 0;
//String URL = "http://172.28.61.16/position?status=3&";
String URL = "http://172.28.61.16/";
int status = 0; //a variable to give instructions to move or stop
float t = 0;
float oldTime = 0;
int ownPosition[2] = {0,0};
int neighbourPosition[4]; //the first two entries are the robot's own position, while the rest are of his neighbours
ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

void setup() 
{
    USE_SERIAL.begin(115200);
    WiFiMulti.addAP("AeroStaff-4", "stewart2");
    #if AGENT == 1
    numOfNeighbours = 1;
//    URL += "p2=1&index=1&up=";
    URL += "pos1?up=";
    randomSeed(12);
    #elif AGENT == 2
    numOfNeighbours = 2;
//    URL += "p1=1&p3=1&index=2&up=";
    URL += "pos2?up=";
    randomSeed(34);
    #elif AGENT == 3
    numOfNeighbours = 1;
//    URL += "p2=1&index=3&up=";
    URL += "pos3?up=";
    randomSeed(102);
    #endif
}

void loop() {
    t = (float)millis()/1000.0;
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) 
    {

        //check the serial port for any requests from master
        updateMasterUnit();
        if(t-oldTime > 0.1)
        {
//              ownPosition[0] = random(100,200);
//              ownPosition[1] = random(100,200);
//              for(int j = 0; j < 2; j++){Serial.print(ownPosition[j]);Serial.print("\t");}
//              for(int j = 0; j < 4; j++){Serial.print(neighbourPosition[j]);Serial.print("\t");}
//              Serial.print(status);
//              Serial.println();
              // configure traged server and url
              String URL_target = URL + String(ownPosition[0])+","+String(ownPosition[1]);
      //        Serial.println(URL_target);
              http.begin(URL_target); //HTTP
              
              // start connection and send HTTP header
              int httpCode = http.GET();
      
              // httpCode will be negative on error
              if(httpCode == HTTP_CODE_OK) 
              {
                  String payload = http.getString();
                  parseData(payload);
      //            USE_SERIAL.println(payload);
              }
              else 
              {
                  USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
              }
      
              http.end();
              oldTime = t;
        }
    }
}

void updateMasterUnit()
{
  String xVal;
  String yVal;
  String outStr;
  boolean fillX = true;
  boolean fillY = false; 
  char a = 0;
   
  if(USE_SERIAL.available()) //note that every agent supplies its position only
  {
    while(USE_SERIAL.available())
    {
      a = USE_SERIAL.read();
      if(a == ',')
      {
        fillX = false;
        fillY = true;
      }
      else
      {
        if(fillX)xVal += a;
        else if(fillY) yVal += a;
      }      
    }
    ownPosition[0] = xVal.toInt();
    ownPosition[1] = yVal.toInt();
//    Serial.print(ownPosition[0]);
//    Serial.print("\t");
//    Serial.println(ownPosition[1]);
    outStr = "$";
    for(int i = 1; i <= numOfNeighbours; i++)
    {
      outStr = outStr + String(neighbourPosition[2*(i-1)]) + "," + String(neighbourPosition[2*(i-1)+1]);
      if(numOfNeighbours > 1)
      {
        outStr = outStr + ",";
      }
    }
    outStr = outStr +"@"+String(status)+"#";
    USE_SERIAL.print(outStr);//sending to the master
  }
}

void parseData(String msg)
{
  const int bufferSize = 20;
  int index = 1;
  int i = 0;
  int L = msg.length();
  char buffer[bufferSize];
  String auxBuffer = "";
  boolean parsingStarted = false;
  
  msg.toCharArray(buffer,bufferSize);
  for (i = 0; i < L; i++)
  {
    if(buffer[i] == '#')
    {
      if(parsingStarted)
      {
        neighbourPosition[2*(index-1)+1] = auxBuffer.toInt();
        auxBuffer = "";
        index++;
      }
      else
      {
        auxBuffer = "";
        parsingStarted = true;
      }
    }
    else if(buffer[i] == ',')
    {
      neighbourPosition[2*(index-1)] = auxBuffer.toInt();
      auxBuffer = "";
    }
    else if(buffer[i] == '$')
    {
      status = auxBuffer.toInt();
      auxBuffer = "";
    }
    else
    {
      auxBuffer += buffer[i];
    }
  }
}

