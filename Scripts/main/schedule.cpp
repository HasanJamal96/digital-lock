
#include "schedule.h"

#define IS_ONESHOT  true   // constants used in arguments to create method
#define IS_REPEAT   false


//**************************************************************
//* Alarm Class Constructor

AlarmClass::AlarmClass()
{
  Mode.isEnabled = Mode.isOneShot = 0;
  Mode.alarmType = dtNotAllocated;
  value = nextTrigger = 0;
  onTickHandler = NULL;  // prevent a callback until this pointer is explicitly set
}

//**************************************************************
//* Private Methods


void AlarmClass::updateNextTrigger()
{
  if (Mode.isEnabled) {
    time_t time = now();
    if (dtIsAlarm(Mode.alarmType) && nextTrigger <= time) {
      // update alarm if next trigger is not yet in the future
      if (Mode.alarmType == dtExplicitAlarm) {
        // is the value a specific date and time in the future
        nextTrigger = value;  // yes, trigger on this value
      } else if (Mode.alarmType == dtDailyAlarm) {
        //if this is a daily alarm
        if (value + previousMidnight(now()) <= time) {
          // if time has passed then set for tomorrow
          nextTrigger = value + nextMidnight(time);
        } else {
          // set the date to today and add the time given in value
          nextTrigger = value + previousMidnight(time);
        }
      } else if (Mode.alarmType == dtWeeklyAlarm) {
        // if this is a weekly alarm
        if ((value + previousSunday(now())) <= time) {
          // if day has passed then set for the next week.
          nextTrigger = value + nextSunday(time);
        } else {
          // set the date to this week today and add the time given in value
          nextTrigger = value + previousSunday(time);
        }
      } else {
        // its not a recognized alarm type - this should not happen
        Mode.isEnabled = false;  // Disable the alarm
      }
    }
    if (Mode.alarmType == dtTimer) {
      // its a timer
      nextTrigger = time + value;  // add the value to previous time (this ensures delay always at least Value seconds)
    }
  }
}

//**************************************************************
//* Time Alarms Public Methods

ScheduleClass::ScheduleClass()
{
  isServicing = false;
  for(uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    free(id);   // ensure all Alarms are cleared and available for allocation
  }
}

void ScheduleClass::enable(AlarmID_t ID)
{
  if (isAllocated(ID)) {
    if (( !(dtUseAbsoluteValue(Alarm[ID].Mode.alarmType) && (Alarm[ID].value == 0)) ) && (Alarm[ID].onTickHandler != NULL)) {
      // only enable if value is non zero and a tick handler has been set
      // (is not NULL, value is non zero ONLY for dtTimer & dtExplicitAlarm
      // (the rest can have 0 to account for midnight))
      Alarm[ID].Mode.isEnabled = true;
      Alarm[ID].updateNextTrigger(); // trigger is updated whenever  this is called, even if already enabled
    } else {
      Alarm[ID].Mode.isEnabled = false;
    }
  }
}

void ScheduleClass::disable(AlarmID_t ID)
{
  if (isAllocated(ID)) {
    Alarm[ID].Mode.isEnabled = false;
  }
}

// write the given value to the given alarm
void ScheduleClass::write(AlarmID_t ID, time_t value)
{
  if (isAllocated(ID)) {
    Alarm[ID].value = value;  //note: we don't check value as we do it in enable()
    Alarm[ID].nextTrigger = 0; // clear out previous trigger time (see issue #12)
    enable(ID);  // update trigger time
  }
}

// return the value for the given alarm ID
time_t ScheduleClass::read(AlarmID_t ID) const
{
  if (isAllocated(ID)) {
    return Alarm[ID].value ;
  } else {
    return dtINVALID_TIME;
  }
}

// return the alarm type for the given alarm ID
dtAlarmPeriod_t ScheduleClass::readType(AlarmID_t ID) const
{
  if (isAllocated(ID)) {
    return (dtAlarmPeriod_t)Alarm[ID].Mode.alarmType ;
  } else {
    return dtNotAllocated;
  }
}

void ScheduleClass::free(AlarmID_t ID)
{
  if (isAllocated(ID)) {
    Alarm[ID].Mode.isEnabled = false;
    Alarm[ID].Mode.alarmType = dtNotAllocated;
    Alarm[ID].onTickHandler = NULL;
    Alarm[ID].value = 0;
    Alarm[ID].nextTrigger = 0;
  }
}

// returns the number of allocated timers
uint8_t ScheduleClass::count() const
{
  uint8_t c = 0;
  for(uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (isAllocated(id)) c++;
  }
  return c;
}

// returns true only if id is allocated and the type is a time based alarm, returns false if not allocated or if its a timer
bool ScheduleClass::isAlarm(AlarmID_t ID) const
{
  return( isAllocated(ID) && dtIsAlarm(Alarm[ID].Mode.alarmType) );
}

// returns true if this id is allocated
bool ScheduleClass::isAllocated(AlarmID_t ID) const
{
  return (ID < dtNBR_ALARMS && Alarm[ID].Mode.alarmType != dtNotAllocated);
}

// returns the currently triggered alarm id
// returns dtINVALID_ALARM_ID if not invoked from within an alarm handler
AlarmID_t ScheduleClass::getTriggeredAlarmId() const
{
  if (isServicing) {
    return servicedAlarmId;  // new private data member used instead of local loop variable i in serviceAlarms();
  } else {
    return dtINVALID_ALARM_ID; // valid ids only available when servicing a callback
  }
}

// following functions are not Alarm ID specific.
void ScheduleClass::delay(unsigned long ms)
{
  unsigned long start = millis();
  do {
    serviceAlarms();
    yield();
  } while (millis() - start  <= ms);
}

void ScheduleClass::waitForDigits( uint8_t Digits, dtUnits_t Units)
{
  while (Digits != getDigitsNow(Units)) {
    serviceAlarms();
  }
}

void ScheduleClass::waitForRollover( dtUnits_t Units)
{
  // if its just rolled over than wait for another rollover
  while (getDigitsNow(Units) == 0) {
    serviceAlarms();
  }
  waitForDigits(0, Units);
}

uint8_t ScheduleClass::getDigitsNow( dtUnits_t Units) const
{
  time_t time = now();
  if (Units == dtSecond) return numberOfSeconds(time);
  if (Units == dtMinute) return numberOfMinutes(time);
  if (Units == dtHour) return numberOfHours(time);
  if (Units == dtDay) return dayOfWeek(time);
  return 255;  // This should never happen
}

//returns isServicing
bool ScheduleClass::getIsServicing() const
{
  return isServicing;
}

//***********************************************************
//* Private Methods

void ScheduleClass::serviceAlarms()
{
  if (!isServicing) {
    isServicing = true;
    for (servicedAlarmId = 0; servicedAlarmId < dtNBR_ALARMS; servicedAlarmId++) {
      if (Alarm[servicedAlarmId].Mode.isEnabled && (now() >= Alarm[servicedAlarmId].nextTrigger)) {
        OnTick_t TickHandler = Alarm[servicedAlarmId].onTickHandler;
        if (Alarm[servicedAlarmId].Mode.isOneShot) {
          free(servicedAlarmId);  // free the ID if mode is OnShot
        } else {
          Alarm[servicedAlarmId].updateNextTrigger();
        }
        if (TickHandler != NULL) {
          (*TickHandler)(Alarm[servicedAlarmId].myVal);     // call the handler
        }
      }
    }
    isServicing = false;
  }
}

// returns the absolute time of the next scheduled alarm, or 0 if none
time_t ScheduleClass::getNextTrigger() const {
  time_t nextTrigger = 0;

  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (isAllocated(id)) {
      if (nextTrigger == 0) {
        nextTrigger = Alarm[id].nextTrigger;
      }
      else if (Alarm[id].nextTrigger <  nextTrigger) {
        nextTrigger = Alarm[id].nextTrigger;
      }
    }
  }
  return nextTrigger;
}

time_t ScheduleClass::getNextTrigger(AlarmID_t ID) const
{
  if (isAllocated(ID)) {
    return Alarm[ID].nextTrigger;
  } else {
    return 0;
  }
}

// attempt to create an alarm and return true if successful
AlarmID_t ScheduleClass::create(time_t value, OnTick_t onTickHandler, uint8_t isOneShot, dtAlarmPeriod_t alarmType, const char *mv) {
  if ( !  ( (dtIsAlarm(alarmType) && now() < SECS_PER_YEAR) || (dtUseAbsoluteValue(alarmType) && (value == 0)) ) ) {
    // only create alarm ids if the time is at least Jan 1 1971
    for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
      if (Alarm[id].Mode.alarmType == dtNotAllocated) {
        // here if there is an Alarm id that is not allocated
        Alarm[id].onTickHandler = onTickHandler;
        Alarm[id].Mode.isOneShot = isOneShot;
        Alarm[id].Mode.alarmType = alarmType;
        Alarm[id].value = value;
        strcpy(Alarm[id].myVal, mv);
        enable(id);
        return id;  // alarm created ok
      }
    }
  }
  else {
    Serial.println("Creating false");
    Serial.println(dtIsAlarm(alarmType));
    Serial.println(dtUseAbsoluteValue(alarmType));
  }
  return dtINVALID_ALARM_ID; // no IDs available or time is invalid
}

// make one instance for the user to use
ScheduleClass Alarm = ScheduleClass() ;
