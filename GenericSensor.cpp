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
#include <LogManagement.h>

std::list<GenericSensor *> GenericSensor::FindSensors(int pin)
{
  std::list<GenericSensor *> ret ;
  LOG("Start scanning pin ")
  LOG_LN(pin)
  LOG_LN("Looking for DHT sensor")
 
  ret = FindDhtSensors(pin);
  if (ret.empty())
  {
    LOG_LN("Found no DHT sensor, scanning for DS18")
    ret.merge(FindDs18Sensors(pin));
  } else {
    LOG_LN ("Found one")
  }
  return ret;
}

  
std::list<GenericSensor *> GenericSensor::FindDhtSensors(int pin)
{
  std::list<GenericSensor *> ret;
  DHTSensor *pPotentialSensor = new DHTSensor();
  
  if (!pPotentialSensor->begin(pin))
  {
    delete pPotentialSensor;
  } else {
    ret.push_front((GenericSensor *)pPotentialSensor);
  }
  return ret;
}


std::list<GenericSensor *> GenericSensor::FindDs18Sensors(int pin)
{
  std::list<GenericSensor *> ret;
  DS18Sensor *pPotentialSensor = new DS18Sensor(pin);
   LOG_LN("sensor created, use scan")
   bool bRet = pPotentialSensor->scan();
  if (!bRet)
  {
    delete pPotentialSensor;
  } else {  
    ret.push_front((GenericSensor *)pPotentialSensor);
  }
  return ret;
}
