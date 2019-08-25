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

#include "DHTSensor.h"
#include "DS18Sensor.h"


std::list<GenericSensor *> GenericSensor::FindSensors(int pin)
{
  std::list<GenericSensor *> ret = FindDhtSensors(pin);

  if (ret.empty())
  {
    ret.merge(FindDs18Sensors(pin));
  }

  return ret;
}

  
std::list<GenericSensor *> GenericSensor::FindDhtSensors(int pin)
{
  std::list<GenericSensor *> ret;
  DHTSensor *pPotentialSensor = new DHTSensor(pin);
  
  if (!pPotentialSensor->begin())
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
  if (!pPotentialSensor->begin())
  {
    delete pPotentialSensor;
  } else {  
    ret.push_front((GenericSensor *)pPotentialSensor);
  }
  return ret;
}
