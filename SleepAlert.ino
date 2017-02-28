
#define sec5 5000
#define sec15 15000
#define sec30 30000
#define sec60 60000

#include "LedControl.h"
#include <ESP8266WiFi.h>
#include "TimeClient.h"
#include <WiFiUdp.h>
extern void slop_wdt_feed();
/*
   pin D8 is connected to the DataIn
   pin D7 is connected to the CLK
   pin D6 is connected to LOAD
   We have only a single MAX72XX.
 */
LedControl lc=LedControl(D8,D7,D6,1);

boolean tick = false;
unsigned long lastUpdate = millis();
unsigned long lastSecond = millis();
unsigned long flashPeriod;
unsigned long blink;
boolean flash=1;
long flashForHowLong;

int alarms[] = {
        22, 00, sec5,
        22, 30, sec5,
        22, 45, sec15,
        23, 00, sec15,
        23, 10, sec15,
        23, 20, sec15,
        23, 30, sec15,
        23, 35, sec30,
        23, 40, sec30,
        23, 45, sec30,
        23, 50, sec30,
        23, 55, sec30,
        23, 56, sec30,
        23, 57, sec30,
        23, 58, sec30,
        23, 59, sec60
};

boolean alert = false;

String hours, minutes, seconds;
int currentSecond, currentMinute, currentHour;

char ssid[] = "xxxxx";  //  your network SSID (name)
char pass[] = "xxxxx";       // your network password

const float UTC_OFFSET = 0;
TimeClient timeClient(UTC_OFFSET);

void setup() {
        //Serial.begin(9600);

        //Serial.println();
        //Serial.println();

        pinMode(D3, OUTPUT);
        lc.shutdown(0,true);
        /* Set the brightness to a medium values */
        lc.setIntensity(0,2);
        /* and clear the display */
        lc.clearDisplay(0);
        // We start by connecting to a WiFi network
        //Serial.print("Connecting to ");
        //Serial.println(ssid);
        WiFi.begin(ssid, pass);
        WiFi.begin(ssid, pass);

        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                //Serial.print(".");
        }
        //Serial.println();
        //Serial.println("WiFi connected");
        //Serial.println("IP address: ");
        //Serial.println(WiFi.localIP());
        timeClient.updateTime();
        getTimeFromServer();
        lastUpdate = millis();
        lastSecond = millis();
        //Serial.println("SETUP");
//void pp_soft_wdt_stop();    // close software watchdog
}

void getTimeFromServer() {
        //void pp_soft_wdt_stop();    // close software watchdog
        hours = timeClient.getHours();
        minutes = timeClient.getMinutes();
        seconds = timeClient.getSeconds();
        currentHour = hours.toInt();
        //if (currentHour>12) currentHour -= 12;
        currentMinute = minutes.toInt();
        currentSecond = seconds.toInt();
        lastUpdate = millis();
}

void updateTimeEverySecond()
{
        if ((millis() - lastSecond) > 1000)
        {
                tick = !tick;
                //lc.clearDisplay(0);
                lastSecond = millis();
                currentSecond++;
                if (currentSecond > 59)
                { currentSecond = 0;
                  currentMinute++;
                  if (currentMinute > 59) {
                          currentMinute = 0;
                          currentHour++;
                          //if (currentHour > 12) currentHour = 0;
                  }}
                String currentTime = "  " + String(currentHour) + ':' + String(currentMinute) + ':' + (currentSecond > 9 ? "" : "0") + String(currentSecond);
                //Serial.println(currentTime);

                if (currentHour>12) lc.setDigit(0,0,int(currentHour/10),false);
                lc.setDigit(0,1,int(currentHour%10),(tick ? true : false));
                lc.setDigit(0,2,int(currentMinute/10),false);
                lc.setDigit(0,3,int(currentMinute%10),false);
        }
}

void checkForAlarm()
{
        for (int index=0; index< (sizeof(alarms)/sizeof(int)); index=index+3)
        {
                if ((alarms[index]==currentHour) && alarms[index+1]==currentMinute && (currentSecond<5))
                {
                        alert=true;
                        flashForHowLong=alarms[index+2];
                        //yield();
                }
        }
}

void loop()
{
        if ((millis()-lastSecond)>1000)
        {
          updateTimeEverySecond();
          checkForAlarm();
        }
        if ((millis() - lastUpdate) > 1800000) getTimeFromServer();

        while (alert)
        {
                blink=millis();
                flashPeriod=millis();
                //Serial.println("Entering while loop");

                while ((millis()-flashPeriod)<flashForHowLong)
                {
                        if (flash)
                        {
                          lc.shutdown(0,false);
                          updateTimeEverySecond();
                        }
                        else lc.shutdown(0,true);

                        if ((millis()-blink)>200)
                        {
                                flash = !flash;
                                digitalWrite(D3, flash);
                                blink=millis();
                                yield();
                        }
                }
                lc.shutdown(0,true);
                alert=false;
                digitalWrite(D3, LOW);
        }
}
