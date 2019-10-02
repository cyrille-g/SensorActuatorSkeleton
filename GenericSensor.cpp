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


#include "GenericSensor.h"
#include "settings.h" 
#include "DHTSensor.h"
#include "DS18Sensor.h"

#include "settings.h"
#include <LogManagement.h>
#include "PinAttribution.h"

std::list<GenericSensor *> GenericSensor::FindSensors(std::pair<uint8_t,uint8_t> *pPinDefinition)
{
  std::list<GenericSensor *> ret ;
  if (pPinDefinition == NULL)
  {
    LOG_LN("pPinDefinition parameter null")
    return ret;
  }
  
  LOG("Start scanning pin D")
  LOG_LN(PinAttribution::DPinToPin(pPinDefinition->first))
  
  if (pPinDefinition->second == SENSOR_DHT22_ID)
  {
    LOG_LN("Looking for DHT sensor")
    ret = FindDhtSensors(pPinDefinition->first);
  } 
  else if (pPinDefinition->second == SENSOR_DS18_ID)
  {
    ret = FindDs18Sensors(pPinDefinition->first);
  } else {
    LOG_LN ("No known sensor type on that pin")
  }
  
  return ret;
}

  
std::list<GenericSensor *> GenericSensor::FindDhtSensors(int pin)
{
  std::list<GenericSensor *> ret;
  DHTSensor *pPotentialSensor = new DHTSensor();
  
  if (!pPotentialSensor->begin(pin))
  {
    LOG_LN("No DHT sensor found on that pin")
    delete pPotentialSensor;
  } else {
    LOG_LN("Found sensor")
    ret.push_front((GenericSensor *)pPotentialSensor);
  }
  return ret;
}


std::list<GenericSensor *> GenericSensor::FindDs18Sensors(int pin)
{
  std::list<GenericSensor *> ret;
  DS18Sensor *pPotentialSensor = new DS18Sensor(pin);

  bool bRet = pPotentialSensor->scan();
  if (!bRet)
  {
    LOG_LN("No DS18 sensor found on that pin")
    delete pPotentialSensor;
  } else {  
    LOG_LN("Found sensor")
    ret.push_front((GenericSensor *)pPotentialSensor);
  }
  return ret;
}
