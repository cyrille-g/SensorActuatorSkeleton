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

#include "PinAttribution.h"

#include <inttypes.h> /*pins_arduino.h does not have defines for uint8_t etc... */
#include "pins_arduino.h"

#include <LogManagement.h>

PinAttribution allSensorsAndActuators;
   /********************************** PIN SETUP   ****************************************/

  PinAttribution::PinAttribution(void)
  {
    _onboardLedState == LOW;
    
   // _pinToScanActuators.push(D2); /*IR led is on D2 */
   
   /* onboard led ON while starting */
   pinMode(BUILTIN_LED, OUTPUT);
   digitalWrite(BUILTIN_LED,LOW);
   
   _pinToScanSensors.push(D1);
   _pinToScanSensors.push(D2);
   _pinToScanSensors.push(D3);
    /*_pinToScanSensors.push(D4); D4 = BUILTIN_LED */
   _pinToScanSensors.push(D5);
   _pinToScanSensors.push(D6);
   _pinToScanSensors.push(D7);
   _pinToScanSensors.push(D8);
 

    int pinToScan = 0;

    std::list<GenericSensor *> sensorRet;
    while(!_pinToScanSensors.empty())
    {
      pinToScan = _pinToScanSensors.front();
      _pinToScanSensors.pop();
      sensorRet = GenericSensor::FindSensors(pinToScan);
      
      std::map<int,std::list<GenericSensor *>>::iterator it = _sensors.find(pinToScan);
      if (it != _sensors.end())
      { 
        it->second.merge(sensorRet);
      } else {
        _sensors.insert(std::pair<int,std::list<GenericSensor *>>(pinToScan,sensorRet));
      }
    }

    std::list<GenericActuator *> actuatorRet;
    while(!_pinToScanActuators.empty())
    {
      pinToScan = _pinToScanActuators.front();
      _pinToScanActuators.pop();
      actuatorRet = GenericActuator::FindActuators(pinToScan);
      
      std::map<int,std::list<GenericActuator *>>::iterator it = _actuators.find(pinToScan);
      if (it != _actuators.end())
      { 
        it->second.merge(actuatorRet);
      } else {
        _actuators.insert(std::pair<int,std::list<GenericActuator *>>(pinToScan,actuatorRet));
      }
    }
  }
  
  PinAttribution::~PinAttribution(void)
  {

    
  }
    
    
  void PinAttribution::PublishMqttState(PubSubClient &mqttClient)
  {
     std::map<int,std::list<GenericSensor *>>::iterator itSensorPin;
     std::list<GenericSensor *> *pList;
     std::list<GenericSensor *>::iterator itSensor;

     for(itSensorPin = _sensors.begin(); itSensorPin != _sensors.end(); itSensorPin++)
     {
       pList = &(itSensorPin->second);
      for(itSensor = itSensorPin->second.begin();itSensor!= itSensorPin->second.end(); itSensor++)
      {
        (*itSensor)->PublishMqttState(mqttClient);
      }
     }

     std::map<int,std::list<GenericActuator *>>::iterator itActuatorPin;
     std::list<GenericActuator *>::iterator itActuator;
     std::list<GenericActuator *> *pActuatorList;
     for(itActuatorPin = _actuators.begin(); itActuatorPin != _actuators.end(); itActuatorPin++)
     {
       pActuatorList = &(itActuatorPin->second);
       for(itActuator = pActuatorList->begin();itActuator!= pActuatorList->end(); itActuator++)
       {
         (*itActuator)->PublishMqttState(mqttClient);
       }
     }
  }

  
  void PinAttribution::SubscribeToMqttSetTopic(PubSubClient &mqttClient)
  {
     std::map<int,std::list<GenericActuator *>>::iterator itPin;
     std::list<GenericActuator *> *pList;
     std::list<GenericActuator *>::iterator itActuator;
     for(itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
     {
      pList = &(itPin->second);
      for(itActuator = pList->begin();itActuator!= pList->end(); itActuator++)
      {
        (*itActuator)->SubscribeToMqttSetTopic(mqttClient);
      }
     }
  }


void PinAttribution::ProcessMqttRequest(char *topic, char* message)
{
   /* find who is the target */
  char *token = strtok(topic, "/");

  /* does it start with the actuator name ? */
  if (token == NULL)
  {
    return;
  }
  /* extract name and set topic */
  char *actuatorName = token;
  
  token = strtok(topic, "/");
  if (token == NULL )
  {
    return;
  }
  
  char *actuatorSetTopic = token;

  std::map<int,std::list<GenericActuator *>>::iterator itPin;
  std::list<GenericActuator *> *pList;
  std::list<GenericActuator *>::iterator itActuator;
  for(itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
  {
    pList = &(itPin->second);
    for(itActuator = pList->begin();itActuator!= pList->end(); itActuator++)
    {
      if ((*itActuator)->isItMyName(actuatorName))
      {
        LOG("found actuator with name")
        LOG_LN(actuatorName)
        /* found our actuator */
        if ((*itActuator)->isItMySetTopic(actuatorSetTopic))
        {
          /* set topic is correct, process */
          LOG_LN("Set topic matches, processing")
          (*itActuator)->ProcessMqttCommand(message);
        } else {
          /* break; this is an error */
          LOG("Wrong set topic ")
          LOG_LN(actuatorSetTopic)
        }
      } 
    }
  }
}

void PinAttribution::ProcessWebRequest(ESP8266WebServer *pWebServer)
{
  std::map<int,std::list<GenericActuator *>>::iterator itPin;
  std::list<GenericActuator *> *pList;
  std::list<GenericActuator *>::iterator itActuator;
  for(itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
  {
    pList = &(itPin->second);
    for(itActuator = pList->begin();itActuator!= pList->end(); itActuator++)
    {
          (*itActuator)->ProcessWebCommand(pWebServer);
    }
  }
}

void PinAttribution::UpdateAllSensors(void)
{
  std::map<int,std::list<GenericSensor *>>::iterator itPin;
  std::list<GenericSensor *> *pList;
  std::list<GenericSensor *>::iterator itSensor;
  for(itPin = _sensors.begin(); itPin != _sensors.end(); itPin++)
  {
    pList = &(itPin->second);
    for(itSensor = pList->begin();itSensor!= pList->end(); itSensor++)
    {
          (*itSensor)->UpdateSensor();
    }
  }
  
}


std::string *PinAttribution::GenerateWebData(void)
{
  std::string *pRet = new std::string("");

  std::string *pWebData = NULL;
  
  std::map<int,std::list<GenericSensor *>>::iterator itPin;
  std::list<GenericSensor *> *pList;
  std::list<GenericSensor *>::iterator itSensor;
  for(itPin = _sensors.begin(); itPin != _sensors.end(); itPin++)
  {
    pList = &(itPin->second);
    for(itSensor = pList->begin();itSensor!= pList->end(); itSensor++)
    {
          pWebData = (*itSensor)->GenerateWebData();
          pRet->append(*pWebData);
          delete pWebData;
    }
  }

  std::map<int,std::list<GenericActuator *>>::iterator itActuatorPin;
  std::list<GenericActuator *> *pActuatorList;
  std::list<GenericActuator *>::iterator itActuator;
  for(itActuatorPin = _actuators.begin(); itActuatorPin != _actuators.end(); itActuatorPin++)
  {
    pActuatorList = &(itActuatorPin->second);
    for(itActuator = pActuatorList->begin();itActuator!= pActuatorList->end(); itActuator++)
    {
          pWebData = (*itActuator)->GenerateWebData();
          pRet->append(*pWebData);
          delete pWebData;
    }
  }
  return pRet;
}

void PinAttribution::ToggleOnboardLed(void)
{
  if (_onboardLedState == LOW)
    _onboardLedState = HIGH;
  else
    _onboardLedState  = LOW;

  digitalWrite(BUILTIN_LED,_onboardLedState);
}
