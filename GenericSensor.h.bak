/******************************************************************************
MIT License

Copyright (c) 2019 Cyrille Gaillardet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef _CGE_GENERICSENSOR_H
#define _CGE_GENERICSENSOR_H

#include <string>
#include <list>
#include <DHT.h>
#include <PubSubClient.h>

typedef enum 
{
  SENSOR_DHT11 = DHT11, /* 11 */
  SENSOR_DHT21 = DHT21, /* 21 */
  SENSOR_AM2301 = DHT21, /* 21 */
  SENSOR_DHT22 = DHT22, /* 22 */
  
  SENSOR_DS18S20,
  SENSOR_DS18B20,
  SENSOR_DS18B22,

  SENSOR_UNDEF = 0xFF
} sensorType_t;


class GenericSensor {
  public:
    GenericSensor(void) { }
    ~GenericSensor(void){ }
    
    virtual void PublishMqttState(PubSubClient &mqttClient) = 0;
    virtual std::string *GenerateWebData(void) = 0;

    virtual void UpdateSensor(void) = 0;
    
    static std::list<GenericSensor *> FindSensors(int pin);
    
  protected:
    sensorType_t _sensorType; 

    std::string _sensorName;
    std::string _stateTopic;  

    private:
    static std::list<GenericSensor *> FindDs18Sensors(int pin);
    static std::list<GenericSensor *> FindDhtSensors(int pin);
};




  
#endif
