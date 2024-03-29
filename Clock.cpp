/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/

#include <Arduino.h> 

#include "Clock.h"

static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated

void refreshCache(time_t t) {
  if (t != cacheTime) {
    breakTime(t, tm); 
    cacheTime = t; 
  }
}

int hour() { // the hour now 
  return hour(now()); 
}

int hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;  
}

int hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now()); 
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(now()); 
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);  
}

uint8_t isPM() { // returns true if PM
  return isPM(now()); 
}

uint8_t isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12); 
}

int minute() {
  return minute(now()); 
}

int minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;  
}

int second() {
  return second(now()); 
}

int second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day(){
  return(day(now())); 
}

int day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday() {   // Sunday is day 1
  return  weekday(now()); 
}

int weekday(time_t t) {
  refreshCache(t);
  return tm.Wday;
}
   
int month(){
  return month(now()); 
}

int month(time_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  return year(now()); 
}

int year(time_t t) { // the year for the given time
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void breakTime(time_t timeInput, tmElements_t &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1  
  tm.Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t &tm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;
  return (time_t)seconds; 
}
/*=====================================================*/	
/* Low level system time functions  */

static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;
static uint32_t NextClockAdjustS = 0;
static uint16_t ClockAdjustStep = 0;
static int8_t AddRemoveMS = 0;

void now_ms(struct timems *tms) {
  uint32_t milliSecondsPassed;
  uint32_t secondsPassed;
  uint32_t nowMillis;

  // millis() and prevMillis are both unsigned ints thus the subtraction will
  // always be the absolute value of the difference
  nowMillis = millis();
  milliSecondsPassed = nowMillis - prevMillis;
  secondsPassed = milliSecondsPassed / 1000;
  if(secondsPassed > 1) {
    secondsPassed = secondsPassed - 1;
    sysTime += secondsPassed;
    prevMillis += secondsPassed * 1000;
    secondsPassed = 1;
  }

  tms->tv_sec = sysTime + secondsPassed;
  tms->tv_msec = milliSecondsPassed % 1000; // prevMillis is always at the top of the second
  tms->raw_millis = nowMillis;

  if(AddRemoveMS != 0 && sysTime >= NextClockAdjustS) {
    // adjust the local clock in case it is running fast or slow
    prevMillis += AddRemoveMS;
    NextClockAdjustS += ClockAdjustStep;
  }
}

// jump to a specific time
void setTime_ms(const struct timems *tms) {
  prevMillis = millis() - tms->tv_msec;
  sysTime = tms->tv_sec;
  Status = timeSet;
  NextClockAdjustS = sysTime + ClockAdjustStep;
}

// jump the time by ms
void adjustTime_ms(int16_t ms) {
  int32_t seconds;
  seconds = ms / 1000;
  sysTime += seconds;
  prevMillis += ms % 1000;
}

// new_AddRemoveMS: positive - local clock is fast, negative - local clock is slow
int adjustClockSpeed(uint16_t StepSeconds, int8_t new_AddRemoveMS) {
  if(Status != timeSet) {
    return -2; // time must be set
  }
  if(new_AddRemoveMS != 0 && new_AddRemoveMS != -1 && new_AddRemoveMS != 1) {
    return -1; // invalid value
  }
  if(StepSeconds < 2) { // max +/- 500ppm
    return -1; // invalid value
  }
  if(StepSeconds > 1000) { // min +/- 1ppm
    return -1; // invalid value
  }

  ClockAdjustStep = StepSeconds;
  NextClockAdjustS = sysTime + StepSeconds;
  AddRemoveMS = new_AddRemoveMS;
  return 0; // ok
}

int adjustClockSpeed_ppm(float clock_error) {
  int8_t posOrNeg = -1;
  uint16_t StepSeconds;

  if(clock_error > 0) {
    posOrNeg = -1;
    StepSeconds = (0.001/clock_error)+0.5;
  } else if(clock_error == 0) {
    return adjustClockSpeed(2, 0);
  } else {
    posOrNeg = 1;
    StepSeconds = (-0.001/clock_error)+0.5;
  }
  if(StepSeconds > 1000) {
    StepSeconds = 1000;
  }
  return adjustClockSpeed(StepSeconds, posOrNeg);
}

int32_t ts_interval(const struct timems *start, const struct timems *end) {
  return (end->tv_sec - start->tv_sec) * 1000 + (end->tv_msec - start->tv_msec);
}

time_t now() {
  struct timems nowTS;
  now_ms(&nowTS);
  return nowTS.tv_sec;
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  return Status;
}
