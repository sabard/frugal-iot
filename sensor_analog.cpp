/*
  Sensor Analog
  Read from a pin and return as sAnalog::Value
*/

#include "_settings.h"  // Settings for what to include etc
#include "_common.h"    // Main include file for Framework
#ifdef WANT_SENSOR_ANALOG
#include <Arduino.h>
#include "sensor_analog.h"
#ifdef SENSOR_ANALOG_MS
#include "system_clock.h"
#endif // SENSOR_ANALOG_MS

// TODO figure out how to handle multiple analog input pins 

namespace sAnalog {

int value = 0;

#ifdef SENSOR_ANALOG_MS
unsigned long lastLoopTime = 0;
#endif // SENSOR_ANALOG_MS


void setup() {      
  // pinMode(SENSOR_ANALOG_PIN, INPUT); // I don't think this is needed
  analogReference(SENSOR_ANALOG_REFERENCE); // TODO see TODO's in the sensor_analog.h
}

void loop() {
#ifdef SENSOR_ANALOG_MS
  if (sClock::hasIntervalPassed(lastLoopTime, SENSOR_ANALOG_MS)) {
#endif // SENSOR_ANALOG_MS
#ifdef SENSOR_ANALOG_SMOOTH // TODO maybe copy this to a system function
        value = value - (value >> SENSOR_ANALOG_SMOOTH) + analogRead(SENSOR_ANALOG_PIN);
#else // !SENSOR_ANALOG_SMOOTH
        value = analogRead(SENSOR_ANALOG_PIN);
#endif // SENSOR_ANALOG_SMOOTH
#ifdef SENSOR_ANALOG_MS
        lastLoopTime = sClock::getTime();
    }
#endif // SENSOR_ANALOG_MS
}
} //namespace sAnalog
#endif // WANT_SENSOR_ANALOG
