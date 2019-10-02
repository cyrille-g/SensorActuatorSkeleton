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

#include "inttypes.h"
#include "pins_arduino.h"

#include "GenericActuator.h"

#include "aircon.h" 

#include "settings.h"
#include <LogManagement.h>

bool GenericActuator::SubscribeToMqttSetTopic(PubSubClient &mqttClient)
{
  if (_setTopic != "")
  {
    return mqttClient.subscribe(_setTopic.c_str());
  } else {
    return false;
  }
}

std::list<GenericActuator *> GenericActuator::FindActuators(std::pair<uint8_t,uint8_t> *pPinDefinition)
{
  std::list<GenericActuator *> ret;

  if (pPinDefinition == NULL)
    return ret;

  if (pPinDefinition->second == ACTUATOR_DAIKIN_AIRCON_ID)
  {
     DaikinAirConditioning *pAircon = new DaikinAirConditioning(pPinDefinition->first);
     ret.push_front((GenericActuator *)pAircon);
  }
  
  return ret;
}
