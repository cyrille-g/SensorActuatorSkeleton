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

#ifndef _CGE_DHTSENSOR_H
#define _CGE_DHTSENSOR_H

#include <DHT.h>
#include <PubSubClient.h>
#include "GenericSensor.h"

class DHTSensor : public DHT, public GenericSensor
{
  public:
  DHTSensor (uint8_t pin, uint8_t sensorType = DHT_AUTO, uint16_t maxIntervalRead = READ_INTERVAL_DONT_CARE);

  bool begin(void);
  
  void PublishMqttState(PubSubClient &mqttClient);
  std::string *GenerateWebData(void);

  void UpdateSensor(void);
  
  private:
    float _minValidTemp;
    float _maxValidTemp;
    float _temperature;
    float _minValidHumidity;
    float _maxValidHumidity;
    float _humidity; 
};

#endif
