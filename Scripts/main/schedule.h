#ifndef schedule_h
#define schedule_h
#include <Arduino.h>
#include <TimeLib.h>

#define MAX_ALARMS 10


typedef void (*OnTick_t)(uint8_t relayFunction);

struct Alarm {
  uint8_t id = 255;
  char name[10];
  time_t startDay;
  time_t endDay;
  time_t startTime;
  time_t endTime;
  time_t lastActivate;
  time_t lastDeactivate;
  uint8_t weekdays;
  uint8_t functionAtStart;
  uint8_t functionAtEnd;
};



class ScheduleManager {
private:
  Alarm alarms[MAX_ALARMS];
  int numAlarms;

  OnTick_t _onTickHandler = NULL;

  time_t currentTime;
  time_t seconds;
  uint8_t dow;
  bool updated = false;

  uint8_t getFreeIndex() {
    for(uint8_t i=0; i<MAX_ALARMS; i++) {
      if(alarms[i].id == 255) {
        return i;
      }
    }
    return 255;
  }

  int convert_to_minutes(int hours, int minutes, int seconds) {
    return (hours * 3600) + (minutes * 60) + seconds;
  }

public:
  bool add(const char *name, time_t startDay, time_t endDay, time_t startTime, time_t endTime, uint8_t w_days, uint8_t functionAtStart, uint8_t functionAtEnd, bool doSave = true) {
    if (numAlarms < MAX_ALARMS) {
      uint8_t index = getFreeIndex();
      if(index != 255) {
        if(strlen(name) > 0)
          strcpy(alarms[index].name, name);
        alarms[index].startDay = startDay;
        alarms[index].endDay = endDay;
        alarms[index].startTime = startTime;
        alarms[index].endTime = endTime;
        alarms[index].weekdays = w_days;
        alarms[index].id = index;
        alarms[index].functionAtStart = functionAtStart;
        alarms[index].functionAtEnd = functionAtEnd;
        numAlarms++;
        if(doSave)
          updated = true;
        Serial.printf("[Schedule] Added new at Index %d\n", index);
        return true;
      }
    }
    return false;
  }

  bool update(uint8_t id, const char *name, time_t startDay, time_t endDay, time_t startTime, time_t endTime, uint8_t w_days, uint8_t functionAtStart, uint8_t functionAtEnd) {
    if(id != 255) {
      if(strlen(name) > 0)
        strcpy(alarms[id].name, name);
      alarms[id].startDay = startDay;
      alarms[id].endDay = endDay;
      alarms[id].startTime = startTime;
      alarms[id].endTime = endTime;
      alarms[id].weekdays = w_days;
      alarms[id].functionAtStart = functionAtStart;
      alarms[id].functionAtEnd = functionAtEnd;
      updated = true;
      return true;
    }
    else {
      return false;
    }
  }


  bool remove(uint8_t id) {
    if(id != 255) {
      if(alarms[id].id == 255) {
        return false;
      }
      else {
        alarms[id].id = 255;
        updated = true;
        numAlarms--;
        return true;
      }
    }
    return false;
  }

  void getAll(char *buf) {
    strcpy(buf, "[");
    for(uint8_t i=0; i<MAX_ALARMS; i++) {
      if(alarms[i].id != 255) {
        Alarm a = alarms[i];
        char localBuf[150];
        sprintf(localBuf, "{\"id\":%d,\"n\":\"%s\",\"sd\":%lu,\"ed\":%lu,\"st\":%lu,\"et\":%lu,\"wd\":%d,\"sf\":%d,\"ef\":%d},", a.id, a.name, a.startDay, a.endDay, a.startTime, a.endTime, a.weekdays, a.functionAtStart, a.functionAtEnd);
        strcat(buf, localBuf);
      }
    }
    int len = strlen(buf);
    if(len > 10) {
      buf[len-1] = '\0';
    }
    strcat(buf, "]");
  }

  void updateCurrentTimeVariables() {
    currentTime = now(); // time in unix in seconds
    seconds = convert_to_minutes(hour(), minute(), second());
    dow = checkWeekday();
  }

  bool isUserInLimits(time_t startDay, time_t endDay, time_t startTime, time_t endTime, uint8_t w_days) {
    updateCurrentTimeVariables();
    if(currentTime >= startDay) { // run always
      if(currentTime <= endDay || endDay == 0) {
        if(dow & w_days) {
          if(seconds >= startTime && seconds <= endTime) {
            return true;
          }
        }
      }
    }
    return false;
  }

  void service() {
    updateCurrentTimeVariables();
    uint8_t runFunction = 255; // 255 is used as invalid function
    for(uint8_t i=0; i<MAX_ALARMS; i++) {
      if(alarms[i].id != 255) {
        if(currentTime >= alarms[i].startDay) { // run always
          if(currentTime <= alarms[i].endDay || alarms[i].endDay == 0) {
            if(dow & alarms[i].weekdays) {
              if(alarms[i].startTime > 0) {
                if(seconds >= alarms[i].startTime && seconds <= alarms[i].startTime+10) {
                  if(currentTime - alarms[i].lastActivate >= 20) {
                    alarms[i].lastActivate = currentTime;
                    runFunction = alarms[i].functionAtStart;
                    continue;
                  }
                }
              }
              if(alarms[i].endTime > 0) {
                if(seconds >= alarms[i].endTime && seconds <= alarms[i].endTime+10) {
                  if(currentTime - alarms[i].lastDeactivate >= 20) {
                    alarms[i].lastDeactivate = currentTime;
                    runFunction = alarms[i].functionAtEnd;
                  }
                }
              }
            }
          }
        }
      }
    }
    if(runFunction != 255) {
      if(_onTickHandler != NULL) {
        _onTickHandler(runFunction); // only run last active schedule
      }
    }
  }

  void printTime() {
    Serial.printf("%d-%d-%d %d:%d:%d %d\n", year(), month(), day(), hour(), minute(), second(), weekday());
  }

  // Get weekday bitmask (Monday = 2, Tuesday = 3, Wednesday = 4, etc.)
  uint8_t checkWeekday() {
    switch (weekday()) {
      case 1: return 1; // Sunday
      case 2: return 2; // Monday
      case 3: return 4; // Tuesday
      case 4: return 8; // Wednesday
      case 5: return 16; // Thursday
      case 6: return 32; // Friday
      case 7: return 64; // Saturday
    }
    return 0; // Should never reach here
  }


  void setCallbackFunction(OnTick_t onTickHandler) {
    _onTickHandler = onTickHandler;
  }

  bool scheduleChanged() {
    if(updated) {
      updated = false;
      return true;
    }
    return false;
  }
};


#endif
