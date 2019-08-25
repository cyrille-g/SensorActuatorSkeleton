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


bool GenericActuator::SubscribeToMqttSetTopic(PubSubClient &mqttClient)
{
  if (_setTopic != "")
  {
    return mqttClient.subscribe(_setTopic.c_str());
  } else {
    return false;
  }
}

/* define your actuator finders here. example:
std::list<GenericActuator *> GenericActuator::FindAircon(int pin)
{
  std::list<GenericActuator *> ret;
  if (pin == D2)
  {
    DaikinAirConditioning *pAircon = new DaikinAirConditioning(D2);
    ret.push_front((GenericActuator *)pAircon);
  }
  return ret;
}
*/
std::list<GenericActuator *> GenericActuator::FindActuators(int pin)
{
  std::list<GenericActuator *> ret;
  
/* call your actuator finders here. example:
  
  ret.merge(FindAircon(pin);
*/

  return ret;
}
