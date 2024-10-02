/*
 * Temperature and Humidity sensor, 
 * Based on SHT85_demo_async.ino in https://github.com/RobTillaart/SHT85
 *
 * Mitra Ardron: Sept 2024
 * 
 * Tested on Lolin SHT30 shield on ESP8266 - not yet tested on other devices or processors
 * 
 * Configuration options example - these are all in _configuration.h
 * Required:
 * SENSOR_SHT85_DEVICE SHT30                   // Which kind of device, for now it presumes they are all the same.
 * SENSOR_SHT85_ADDRESS 0x45                   // A single device address in hex
 * or SENSOR_SHT85_ADDRESS_ARRAY 0x45,0x44     // A list of device addresses
 * SENSOR_SHT85_MS                             // How often to poll each sensor, for now we presume we poll them all this often
 * Optional: 
 * SENSOR_SHT85_DEBUG                          // Debugging output
 *
 * TODO Support multiple I2C Wires - so for example can use two sensors on each wire. See Issue#16
 * TODO Pull the Wire support into a seperate module so that a single Wire can be used for alternate sensors. See Issue#16
 * TODO Support I2C multiplexors - see sample code at https://github.com/RobTillaart/SHT85/issues/26#issuecomment-2367448245
*/

#include "_settings.h"  // Settings for what to include etc

#ifdef SENSOR_SHT85_WANT

#include <Arduino.h>
#include "_common.h"    // Main include file for Framework
#include <SHT85.h>
#include "sensor_sht85.h"

#if defined(SENSOR_SHT85_TOPIC_TEMPERATURE) || defined(SENSOR_SHT85_TOPIC_HUMIDITY)
  #include "system_mqtt.h"
#endif // SENSOR_SHT85_TOPIC_TEMPERATURE || SENSOR_SHT85_TOPIC_HUMIDITY

namespace sSHT85 {

#ifndef SENSOR_SHT85_ADDRESS_ARRAY
  SENSOR_SHT85_DEVICE *sht;
#else
  SENSOR_SHT85_DEVICE *sht_array[SENSOR_SHT85_COUNT];
  int sht_address_array[] = {SENSOR_SHT85_ADDRESS_ARRAY};
  // Defining SENSOR_SHT85_ADDRESS simplifies some code as always used in context of 'i' being defined
  #define SENSOR_SHT85_ADDRESS sht_address_array[i]
#endif // SENSOR_SHT85_ADDRESS_ARRAY

unsigned long nextLoopTime = 0;
#ifndef SENSOR_SHT85_ADDRESS_ARRAY
float temperature; 
float humidity; 
#else
float temperature[SENSOR_SHT85_COUNT];
float humidity[SENSOR_SHT85_COUNT];
#endif // SENSOR_SHT85_ADDRESS_ARRAY

#ifdef SENSOR_SHT85_TOPIC_TEMPERATURE
  String *topicT = new String(SENSOR_SHT85_TOPIC_TEMPERATURE);
  float lastTemperature;
#endif
#ifdef SENSOR_SHT85_TOPIC_HUMIDITY
  String *topicH = new String(SENSOR_SHT85_TOPIC_HUMIDITY);
  float lastHumidity;
#endif

SENSOR_SHT85_DEVICE *setup_sensor(unsigned int addr) {
  SENSOR_SHT85_DEVICE *sht = new SENSOR_SHT85_DEVICE(addr);
  sht->begin();
#ifdef SENSOR_SHT85_DEBUG
  uint16_t stat = sht->readStatus();
  Serial.print("addr: ");
  Serial.print(addr);
  Serial.print(" status: ");
  Serial.print(stat, HEX);
  Serial.println();
  sht->requestData(); // Initial request queued up 
#endif
  return sht;
}
void setup()
{
#ifdef SENSOR_SHT85_DEBUG
  Serial.println(__FILE__);
  Serial.print("SHT_LIB_VERSION: \t");
  Serial.println(SHT_LIB_VERSION);
#endif

  //TODO It might be that we have to be careful to only setup the Wire once if there are multiple sensors. 
  Wire.begin();
  Wire.setClock(100000);

#ifndef SENSOR_SHT85_ADDRESS_ARRAY // TODO could merge the if and else with clever ifdef around the looping part
  sht = setup_sensor(SENSOR_SHT85_ADDRESS);
#else // SENSOR_SHT85_ADDRESS_ARRAY
  for(int i = 0 ; i < SENSOR_SHT85_COUNT; i++) { //TODO can use length of sht_address_array and dont think need _COUNT 
    sht_array[i] = setup_sensor(sht_address_array[i]);
  }
#endif // SENSOR_SHT85_ADDRESS_ARRAY
}

void readSensor(SENSOR_SHT85_DEVICE *sht, unsigned int i) {
  #ifdef SENSOR_SHT85_DEBUG
    Serial.print(SENSOR_SHT85_ADDRESS);
    Serial.print("   ");
  #endif
  if (sht->dataReady())
  {
    if (sht->readData()) {
      #ifndef SENSOR_SHT85_ADDRESS_ARRAY
        temperature = sht->getTemperature(); // TODO use raw version https://github.com/RobTillaart/SHT85
        humidity = sht->getHumidity(); // TODO use raw version https://github.com/RobTillaart/SHT85
      #else
        temperature[i] = sht->getTemperature(); // TODO use raw version https://github.com/RobTillaart/SHT85
        humidity[i] = sht->getHumidity(); // TODO use raw version https://github.com/RobTillaart/SHT85
      #endif
      // Note, not smoothing the data as it seems fairly stable and is float rather than bits anyway
      #ifdef SENSOR_SHT85_DEBUG
        #ifndef SENSOR_SHT85_ADDRESS_ARRAY
          Serial.print(temperature, 1);
        #else
          Serial.print(temperature[i], 1);
        #endif
        Serial.print("°C\t");
        #ifndef SENSOR_SHT85_ADDRESS_ARRAY
          Serial.print(humidity, 1);
        #else
          Serial.print(humidity[i], 1);
        #endif
        Serial.println("%");
      #endif
      // Note only request more Data if was dataReady
      sht->requestData(); // Request next one
#ifdef SENSOR_SHT85_DEBUG
  } else {
    Serial.println("SHT sensor did not return data");
#endif
    }
#ifdef SENSOR_SHT85_DEBUG
  } else {
    Serial.println("SHT sensor not ready");
#endif
  }
}

void loop() {
  if (nextLoopTime <= millis() ) {
    #ifndef SENSOR_SHT85_ADDRESS_ARRAY
        readSensor(sht,0);
    #else
      for(int i = 0 ; i < SENSOR_SHT85_COUNT; i++) {
        readSensor(sht_array[i],i);
      }
    #endif
    #ifndef SENSOR_SHT85_ADDRESS_ARRAY
      #ifdef SENSOR_SHT85_TOPIC_TEMPERATURE
        if (temperature != lastTemperature) { // TODO may want to add some bounds (e.g a percentage)
          xMqtt::messageSend(*topicT, temperature, 1);
          lastTemperature = temperature;
        }
      #endif
      #ifdef SENSOR_SHT85_TOPIC_HUMIDITY
        if (humidity != lastHumidity) { // TODO may want to add some bounds (e.g a percentage)
          xMqtt::messageSend(*topicH, humidity, 1);
          lastHumidity = humidity;
        }
      #endif
    #else
      // TODO expand to do MQTT with array 
      #if defined(SENSOR_SHT85_TOPIC_TEMPERATURE) || defined(SENSOR_SHT85_TOPIC_HUMIDITY)
        ERROR - undefined // intentionally wont compile !! 
      #endif
    #endif
    nextLoopTime = millis() + SENSOR_SHT85_MS;

// Now convert to string using dtostrf 


  }
}

} // namespace sSHT85

#endif SENSOR_SHT85_WANT

//  -- END OF FILE --

