// Sleep alert clock
// by Mike McRoberts

#include "LedControl.h"
#include <ESP8266WiFi.h>
#include "TimeClient.h"
#include <WiFiUdp.h>

LedControl lc=LedControl(D7,D5,D1,1);

boolean tick = false;
unsigned long lastUpdate = millis();
unsigned long lastSecond = millis();
unsigned long flashPeriod;
unsigned long blink;
boolean flash=1;
unsigned long flashForHowLong;

unsigned int alarms[] = {
        22, 00, 5,
        22, 30, 5,
        22, 45, 15,
        23, 00, 15,
        23, 10, 15,
        23, 20, 15,
        23, 30, 15,
        23, 35, 30,
        23, 40, 30,
        23, 45, 30,
        23, 50, 30,
        23, 55, 30,
        23, 56, 30,
        23, 57, 30,
        23, 58, 30,
        23, 59, 60,
};

boolean alert = false;

String hours, minutes, seconds;
int currentSecond, currentMinute, currentHour;

char ssid[] = "xxxxxx";  //  your network SSID (name)
char pass[] = "xxxxxx";       // your network password

const float UTC_OFFSET = 0;
TimeClient timeClient(UTC_OFFSET);

void setup() {
        Serial.begin(9600);

        //Serial.println();
        //Serial.println();

        pinMode(D3, OUTPUT);
        lc.shutdown(0,false);
        /* Set the brightness to a medium values */
        lc.setIntensity(0,0);
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

        tick = !tick;
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

        showTime();
}

void checkForAlarm()
{
        alert=false;
        for (int index=0; index< (sizeof(alarms)/sizeof(int)); index=index+3)
        {
                if ((alarms[index]==currentHour) && alarms[index+1]==currentMinute && (currentSecond<5))
                {
                        alert=true;
                        flashForHowLong=alarms[index+2]*1000;
                        //yield();
                }
        }
}

void showTime()
{

        lc.setDigit(0,3,currentMinute%10,false);
        lc.setDigit(0,2,currentMinute/10,false);
        lc.setDigit(0,1,currentHour%10,(tick ? true : false));
        lc.setDigit(0,0,currentHour/10,false);
}

void loop()
{
        if ((millis() - lastUpdate) > 600000) getTimeFromServer();

        if ((millis()-lastSecond)>1000)
        {
                lastSecond = millis();
                updateTimeEverySecond();
                checkForAlarm();
        }

        while (alert==true)
        {
                blink=millis();
                flashPeriod=millis();

                while ((millis()-flashPeriod)<flashForHowLong)
                {
                        if ((millis()-blink)>100)
                        {
                                flash = !flash;
                                blink=millis();
                                digitalWrite(D3, flash);
                                yield();
                        }

                        if (flash)  lc.setIntensity(0,15);
                        else  lc.setIntensity(0,0);

                        if ((millis()-lastSecond)>1000)
                        {
                                lastSecond = millis();
                                updateTimeEverySecond();
                        }
                }
                digitalWrite(D3, LOW);
                lc.setIntensity(0,0);
                alert=false;
        }
}
