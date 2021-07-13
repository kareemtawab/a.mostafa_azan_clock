#include <UTFT.h>
#include <UTFT_SdRaw.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "Timer.h"
#include <stdio.h>
#include "PrayerTimes.h"

Timer t;
UTFT myGLCD(R61581, 38, 39, 40, 41); // 480x320 pixels
RTC_DS3231 rtc;
DateTime now;

extern unsigned int side[16000];
extern uint8_t Sinclair_M[];
extern uint8_t Sinclair_S[];
extern uint8_t Grotesk32x64[];
char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
int n = 0;
String dateday;
String datemonth;
float salatLatitude = 30.612543724263904; //Al-Alamein
float salatLongitude = 28.739135955260206; //Al-Alamein
int salatTimezone = 2;  //New Cairo
double salatTimes[sizeof(TimeName) / sizeof(char*)];
int salatHours;
int salatMinutes;
int currentSalatID;
int previousSalatID;
int line = 110;
int hoursIn12hr;

#define button 13

void setup()
{
  set_calc_method(Egypt);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(19.5);
  set_isha_angle(17.5);

  Serial.begin(115200);

  myGLCD.InitLCD();
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(Sinclair_M);
  myGLCD.print("PRAYER TIMES CLOCK", CENTER, 5);
  myGLCD.setFont(Sinclair_S);
  myGLCD.print("made for A.M.Mokhtar", CENTER, 24);
  myGLCD.drawLine(68, 35, 420, 35);
  myGLCD.print("Adruino Mega 2560 + DS3231 RTC +", CENTER, 40);
  myGLCD.print("R61581 3.5inch LCD + LM2590 DC/DC Converter", CENTER, 51);
  myGLCD.print("Design & Execution: KareemTawab", CENTER, 70);
  myGLCD.print("Build Date: JUL 15, 2021", CENTER, 81);
  if (! rtc.begin()) {
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("> RTC initialization failed! System can not continue!", LEFT, 5);
    while (1);
  }
  else {
    myGLCD.print("> RTC initialized!", LEFT, line);
  }
  if (rtc.lostPower()) {
    line += 11;
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("> RTC lost time/date! System can not continue!", LEFT, line);
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    while (1);
  }
  line += 11;
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("> Starting...", LEFT, line);
  delay(2000);
  myGLCD.fillScr(0, 0, 0);
  myGLCD.drawBitmap (0, 0, 50, 320, side);
  myGLCD.drawBitmap (430, 0, 50, 320, side);
  t.every(200, lcdupdatetime);
}


void loop()
{
  t.update();
  now = rtc.now();
}

void lcdupdatetime() {
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(Sinclair_S);
  myGLCD.print("INTERNAL TEMPERATURE: ", 119, 260);
  myGLCD.printNumF(rtc.getTemperature(), 1, 293, 260);
  myGLCD.print("degC", 328, 260);
  myGLCD.print("LONG 30.6125437, LAT 28.7391359, SHAAFI", CENTER, 285);
  myGLCD.print("EGYPTIAN GENERAL AUTHORITY OF SURVEY", CENTER, 296);
  myGLCD.print("FAJR ANGLE 19.5, ISHA ANGLE 17.5", CENTER, 307);

  //--------------------------Time/Date-----------------------------

  if (now.hour() > 0 && now.hour() <= 12) {
    hoursIn12hr = now.hour();
  }
  if (now.hour() > 12 && now.hour() <= 23) {
    hoursIn12hr = now.hour() - 12;
  }
  if (now.hour() == 00) {
    hoursIn12hr = 12;
  }
  myGLCD.setFont(Sinclair_M);
  myGLCD.print(daysOfTheWeek[now.dayOfTheWeek()], 125, 10);
  myGLCD.printNumI(now.day(), 193, 10, 2, '0');
  myGLCD.print("/", 225, 10);
  myGLCD.printNumI(now.month(), 243, 10, 2, '0');
  myGLCD.print("/", 275, 10);
  myGLCD.printNumI(now.year(), 293, 10, 2, '0');
  myGLCD.setFont(Grotesk32x64);
  myGLCD.printNumI(hoursIn12hr, 70, 35, 2, '0');
  myGLCD.print(":", 135, 35);
  myGLCD.printNumI(now.minute(), 170, 35, 2, '0');
  myGLCD.print(":", 235, 35);
  myGLCD.printNumI(now.second(), 270, 35, 2, '0');
  if (now.hour() >= 0 && now.hour() < 12) {
    myGLCD.print("AM", 347, 35);
  }
  else {
    myGLCD.print("PM", 347, 35);
  }
  myGLCD.setFont(Sinclair_S);
  myGLCD.print("TIMEZONE: AL-ALAMEIN (UTC+02:00)", CENTER, 102);

  //---------------------------Prayer Times----------------------------

  myGLCD.setFont(Sinclair_M);
  previousSalatID = currentSalatID;
  get_prayer_times(now.year(), now.month(), now.day(), salatLatitude, salatLongitude, salatTimezone, salatTimes);
  for (int i = 0; i < sizeof(salatTimes); i++) {
    if (i == 4) {
      continue;
    }
    if ((now.hour() + now.minute() / 60.00) > salatTimes[i] && (now.hour() + now.minute() / 60.00) <= salatTimes[i + 1]) {
      currentSalatID = i + 1;
      if (currentSalatID == 4) {
        currentSalatID = 5;
      }
      break;
    }
  }

  //Serial.println(currentSalatID);

  if (currentSalatID != previousSalatID) {
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(80, 126, 400, 250); // blank screen for prayer times
  }
  myGLCD.setColor(192, 173, 124);
  switch (currentSalatID) {
    case 0:
      myGLCD.drawRoundRect(80, 128, 400, 148);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 128, 400, 148);
      break;
    case 1:
      myGLCD.drawRoundRect(80, 148, 400, 168);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 148, 400, 168);
      break;
    case 2:
      myGLCD.drawRoundRect(80, 168, 400, 188);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 168, 400, 188);
      break;
    case 3:
      myGLCD.drawRoundRect(80, 188, 400, 208);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 188, 400, 208);
      break;
    case 5:
      myGLCD.drawRoundRect(80, 208, 400, 228);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 208, 400, 228);
      break;
    case 6:
      myGLCD.drawRoundRect(80, 228, 400, 248);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 228, 400, 248);
      break;
    default:
      myGLCD.drawRoundRect(80, 128, 400, 148);
      delay(400);
      myGLCD.setColor(10, 10, 10);
      myGLCD.drawRoundRect(80, 128, 400, 148);
      break;
  }

  myGLCD.setColor(255, 255, 255);
  if ((now.hour() + now.minute() / 60.00) > salatTimes[6]) {
    get_prayer_times(now.year(), now.month(), now.day() + 1, salatLatitude, salatLongitude, salatTimezone, salatTimes);
  }

  //---------------FAJR----------------
  get_float_time_parts(salatTimes[0], salatHours, salatMinutes);
  myGLCD.print(TimeName[0], 87, 130);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 130);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 130);
    }
    else {
      myGLCD.print("AM", 365, 130);
    }
  }
  myGLCD.printNumI(salatHours, 275, 130, 2, '0');
  myGLCD.print(":", 305, 130);
  myGLCD.printNumI(salatMinutes, 325, 130, 2, '0');

  //---------------SUNRISE----------------
  get_float_time_parts(salatTimes[1], salatHours, salatMinutes);
  myGLCD.print(TimeName[1], 87, 150);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 150);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 150);
    }
    else {
      myGLCD.print("AM", 365, 150);
    }
  }
  myGLCD.printNumI(salatHours, 275, 150, 2, '0');
  myGLCD.print(":", 305, 150);
  myGLCD.printNumI(salatMinutes, 325, 150, 2, '0');

  //---------------DHUHR----------------
  get_float_time_parts(salatTimes[2], salatHours, salatMinutes);
  myGLCD.print(TimeName[2], 87, 170);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 170);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 170);
    }
    else {
      myGLCD.print("AM", 365, 170);
    }
  }
  myGLCD.printNumI(salatHours, 275, 170, 2, '0');
  myGLCD.print(":", 305, 170);
  myGLCD.printNumI(salatMinutes, 325, 170, 2, '0');

  //---------------ASR----------------
  get_float_time_parts(salatTimes[3], salatHours, salatMinutes);
  myGLCD.print(TimeName[3], 87, 190);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 190);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 190);
    }
    else {
      myGLCD.print("AM", 365, 190);
    }
  }
  myGLCD.printNumI(salatHours, 275, 190, 2, '0');
  myGLCD.print(":", 305, 190);
  myGLCD.printNumI(salatMinutes, 325, 190, 2, '0');

  //---------------MAGHRIB----------------
  get_float_time_parts(salatTimes[5], salatHours, salatMinutes);
  myGLCD.print(TimeName[5], 87, 210);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 210);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 210);
    }
    else {
      myGLCD.print("AM", 365, 210);
    }
  }
  myGLCD.printNumI(salatHours, 275, 210, 2, '0');
  myGLCD.print(":", 305, 210);
  myGLCD.printNumI(salatMinutes, 325, 210, 2, '0');

  //---------------ISHA----------------
  get_float_time_parts(salatTimes[6], salatHours, salatMinutes);
  myGLCD.print(TimeName[6], 87, 230);
  if (salatHours > 12) {
    salatHours = salatHours - 12;
    myGLCD.print("PM", 365, 230);
  }
  else {
    if (salatHours == 12) {
      myGLCD.print("PM", 365, 230);
    }
    else {
      myGLCD.print("AM", 365, 230);
    }
  }
  myGLCD.printNumI(salatHours, 275, 230, 2, '0');
  myGLCD.print(":", 305, 230);
  myGLCD.printNumI(salatMinutes, 325, 230, 2, '0');
}
