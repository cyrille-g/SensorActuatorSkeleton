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

#ifndef _CGE_PIN_ATTRIBUTION_H
#define _CGE_PIN_ATTRIBUTION_H

#include <queue>
#include <map>
#include <list>
#include <string>

#include "settings.h"
#include "GenericSensor.h"
#include "GenericActuator.h"
#include <ESP8266WebServer.h>


class PinAttribution 
{

  public:
  PinAttribution(void);
  ~PinAttribution(void);
  void begin(void);
  void ToggleOnboardLed(void);
  
  void PublishMqttState(PubSubClient &mqttClient);
  void SubscribeToMqttSetTopic(PubSubClient &mqttClient);

  void ProcessMqttRequest(char *topic, char* message);
  void ProcessWebRequest(ESP8266WebServer *pWebServer);

  void HandlePinsConfiguration(ESP8266WebServer &webServer);
  void AppendWebData(std::string &str);
  void AppendConfigureData(std::string &str);
  
  void UpdateAllSensors(void);

  static uint8_t PinToDPin(uint8_t pin);
  static uint8_t DPinToPin(uint8_t dPin);
  
  private:
  char _localConvertBuffer[10];

  std::vector<char *> _supportedActuators;
  std::vector<char *> _supportedSensors;
  
  std::queue<std::pair<uint8_t,uint8_t>*> _pinToScanActuators; 
  std::queue<std::pair<uint8_t,uint8_t>*> _pinToScanSensors;

  int _onboardLedState;
  
  std::map<int,std::list<GenericSensor *>> _sensors;
  std::map<int,std::list<GenericActuator *>> _actuators;


  void AppendSensorSelect(std::string &str, std::string selectName, int pin);
  void AppendActuatorSelect(std::string &str, std::string selectName, int pin);


};


extern PinAttribution allocatedPins;
#endif
