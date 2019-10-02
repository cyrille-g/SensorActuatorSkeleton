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
#include "parameters.h"
#include <inttypes.h> /*pins_arduino.h does not have defines for uint8_t etc... */
#include "pins_arduino.h"

#include "settings.h"
#include <LogManagement.h>

PinAttribution allocatedPins;
/********************************** PIN SETUP   ****************************************/

PinAttribution::PinAttribution(void)
{
  _onboardLedState == LOW;
  
   /* onboard led ON while starting */
   pinMode(BUILTIN_LED, OUTPUT);
   digitalWrite(BUILTIN_LED,LOW);
  

  /* these are used for text in webservice */
  _supportedActuators = SUPPORTED_ACTUATORS ;
  _supportedSensors = SUPPORTED_SENSORS ;  
}
  
void PinAttribution::begin(void)
{
  std::pair<uint8_t,uint8_t> *pPinDefinition;

  /* pins used are D1 to D8 . They are converted in the parameters class to a 0->7 index, 
   * but are refered to as D1 to D8 / 1 to 8.  
   * all accessors do the shift 
   */
  for(int i=1;i<=8;i++)
  {
    if (parameters.IsInput(i))
    {
      pinMode(PinAttribution::PinToDPin(i), INPUT);  
      pPinDefinition = (std::pair<uint8_t,uint8_t> *)malloc(sizeof (std::pair<uint8_t,uint8_t>));
      pPinDefinition->first = PinAttribution::PinToDPin(i);
      pPinDefinition->second = parameters.GetSensorIdOnPin(i); 
      _pinToScanSensors.push(pPinDefinition);
      sprintf(allocatedPins._localConvertBuffer,"Pin D%d",i);
      LOG(allocatedPins._localConvertBuffer)
      LOG(" set as input for sensor type ")
      sprintf(allocatedPins._localConvertBuffer,"%d",parameters.GetSensorIdOnPin(i));
      LOG_LN(allocatedPins._localConvertBuffer)
    } 
    else if (parameters.IsOutput(i))
    {
      pinMode(PinAttribution::PinToDPin(i), OUTPUT);  
      pPinDefinition = (std::pair<uint8_t,uint8_t> *)malloc(sizeof (std::pair<uint8_t,uint8_t>));
      pPinDefinition->first = PinAttribution::PinToDPin(i);
      pPinDefinition->second = parameters.GetActuatorIdOnPin(i);
      _pinToScanActuators.push(pPinDefinition);
      sprintf(allocatedPins._localConvertBuffer,"Pin D%d",i);
      LOG(allocatedPins._localConvertBuffer)
      LOG(" set as output for actuator type ")
      sprintf(allocatedPins._localConvertBuffer,"%d",parameters.GetSensorIdOnPin(i));
      LOG_LN(allocatedPins._localConvertBuffer)
    } else {
      /* pin set to none, do nothing */
      sprintf(allocatedPins._localConvertBuffer,"Pin D%d",i);
      LOG(allocatedPins._localConvertBuffer)
      LOG_LN(" set as none")
      }
  }
  
  std::list<GenericSensor *> sensorRet;
  while(!_pinToScanSensors.empty())
  {
    pPinDefinition = _pinToScanSensors.front();
    _pinToScanSensors.pop();
    sensorRet = GenericSensor::FindSensors(pPinDefinition);

    if (!sensorRet.empty())
    {
      std::map<int,std::list<GenericSensor *>>::iterator it = _sensors.find(pPinDefinition->first);
      if (it != _sensors.end())
      { 
        it->second.merge(sensorRet);
        LOG_LN("Sensor(s) already exist on that pin, merging")
      } else {
        _sensors.insert(std::pair<int,std::list<GenericSensor *>>(pPinDefinition->first,sensorRet));
        LOG_LN("Adding first sensor on that pin")
      }
    }
  }
  
  std::list<GenericActuator *> actuatorRet;
  while(!_pinToScanActuators.empty())
  {
    pPinDefinition = _pinToScanActuators.front();
    _pinToScanActuators.pop();
    actuatorRet = GenericActuator::FindActuators(pPinDefinition);

    if(!actuatorRet.empty())
    {
      std::map<int, std::list<GenericActuator *>>::iterator it = _actuators.find(pPinDefinition->first);
      if (it != _actuators.end())
      {
        LOG_LN("Actuator(s) already exist on that pin, merging")
        it->second.merge(actuatorRet);
      } else {
        _actuators.insert(std::pair<int, std::list<GenericActuator *>>(pPinDefinition->first, actuatorRet));
         LOG_LN("Adding first actuator on that pin")
      }
    }
  }
}

PinAttribution::~PinAttribution(void)
{
  /* we should free various pointers but really this function will never be called */
}

void PinAttribution::PublishMqttState(PubSubClient &mqttClient)
{
  std::map<int, std::list<GenericSensor *>>::iterator itSensorPin;
  std::list<GenericSensor *> *pList;
  std::list<GenericSensor *>::iterator itSensor;

  for (itSensorPin = _sensors.begin(); itSensorPin != _sensors.end(); itSensorPin++)
  {
    pList = &(itSensorPin->second);
    for (itSensor = itSensorPin->second.begin(); itSensor != itSensorPin->second.end(); itSensor++)
    {
      (*itSensor)->PublishMqttState(mqttClient);
    }
  }

  std::map<int, std::list<GenericActuator *>>::iterator itActuatorPin;
  std::list<GenericActuator *>::iterator itActuator;
  std::list<GenericActuator *> *pActuatorList;
  for (itActuatorPin = _actuators.begin(); itActuatorPin != _actuators.end(); itActuatorPin++)
  {
    pActuatorList = &(itActuatorPin->second);
    for (itActuator = pActuatorList->begin(); itActuator != pActuatorList->end(); itActuator++)
    {
      (*itActuator)->PublishMqttState(mqttClient);
    }
  }
}


void PinAttribution::SubscribeToMqttSetTopic(PubSubClient &mqttClient)
{
  std::map<int, std::list<GenericActuator *>>::iterator itPin;
  std::list<GenericActuator *> *pList;
  std::list<GenericActuator *>::iterator itActuator;
  for (itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
  {
    pList = &(itPin->second);
    for (itActuator = pList->begin(); itActuator != pList->end(); itActuator++)
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

  std::map<int, std::list<GenericActuator *>>::iterator itPin;
  std::list<GenericActuator *> *pList;
  std::list<GenericActuator *>::iterator itActuator;
  for (itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
  {
    pList = &(itPin->second);
    for (itActuator = pList->begin(); itActuator != pList->end(); itActuator++)
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
  std::map<int, std::list<GenericActuator *>>::iterator itPin;
  std::list<GenericActuator *> *pList;
  std::list<GenericActuator *>::iterator itActuator;
  GenericActuator *pActuator;
  for (itPin = _actuators.begin(); itPin != _actuators.end(); itPin++)
  {
    LOG("actuator registered on pin")
    LOG_LN(itPin->first);
    pList = &(itPin->second);
    for (itActuator = pList->begin(); itActuator != pList->end(); itActuator++)
    {
      LOG_LN("calling web request processing")
      pActuator = *itActuator;
      pActuator->ProcessWebCommand(pWebServer);
    }
  }
}

void PinAttribution::UpdateAllSensors(void)
{
  std::map<int, std::list<GenericSensor *>>::iterator itPin;
  std::list<GenericSensor *> *pList;
  std::list<GenericSensor *>::iterator itSensor;
  for (itPin = _sensors.begin(); itPin != _sensors.end(); itPin++)
  {
    pList = &(itPin->second);
    for (itSensor = pList->begin(); itSensor != pList->end(); itSensor++)
    {
      (*itSensor)->UpdateSensor();
    }
  }
}


void PinAttribution::AppendWebData(std::string &str)
{
  //str.append("<TR>");
  std::map<int, std::list<GenericSensor *>>::iterator itPin;
  std::list<GenericSensor *> *pList;
  std::list<GenericSensor *>::iterator itSensor;
  for (itPin = _sensors.begin(); itPin != _sensors.end(); itPin++)
  {
    pList = &(itPin->second);
    for (itSensor = pList->begin(); itSensor != pList->end(); itSensor++)
    {
      //str.append("<TD>");
      (*itSensor)->AppendWebData(str);
      //str.append("</TD>");
    }
  }
//  str.append("</TR>");
  std::map<int, std::list<GenericActuator *>>::iterator itActuatorPin;
  std::list<GenericActuator *> *pActuatorList;
  std::list<GenericActuator *>::iterator itActuator;
//  str.append("<TR>");
  for (itActuatorPin = _actuators.begin(); itActuatorPin != _actuators.end(); itActuatorPin++)
  {
    pActuatorList = &(itActuatorPin->second);
    for (itActuator = pActuatorList->begin(); itActuator != pActuatorList->end(); itActuator++)
    {
//      str.append("<TD>");
      (*itActuator)->AppendWebData(str);
//      str.append("</TD>");
    }
  }
  str.append("</TR>");
}

void PinAttribution::AppendSensorSelect(std::string &str, std::string selectName, int pin)
{ 
  str.append("<select class=\"gridpos3\" id=\"");
  str.append(selectName);
  str.append("\" name=\"");
  str.append(selectName);
  str.append("\" style='display:");

  if (parameters.IsInput(pin))
  {
    str.append("inline;'> <option value=\"0\">None</option>");
  } else {
    str.append("none;'> <option value=\"0\" selected >None</option>");
  }

  for(int i = 0;i < SUPPORTED_SENSORS_COUNT;i++)
  {
    str.append("<option value=\"");
    sprintf(allocatedPins._localConvertBuffer,"%d",i + 1 );
    str.append(allocatedPins._localConvertBuffer);
    str.append("\"");
    if ((parameters.IsInput(pin)) && (parameters.GetSensorIdOnPin(pin) == (i + 1 )))
    {
      str.append(" selected >");
    } else {
      str.append(">");
    }
    str.append(allocatedPins._supportedSensors[i]); 
    str.append("</option>");
  }
  str.append("</select>");
}

void PinAttribution::AppendActuatorSelect(std::string &str, std::string selectName, int pin)
{ 
  str.append("<select class=\"gridpos3\" id=\"");
  str.append(selectName);
  str.append("\" name=\"");
  str.append(selectName);
  str.append("\" style='display:");
  if (parameters.IsOutput(pin))
  {
    str.append("inline;'> <option value=\"0\">None</option>");
  } else {
    str.append("none;'> <option value=\"0\" selected >None</option>");
  }
  
  for(int i = 0;i < SUPPORTED_ACTUATORS_COUNT;i++)
  {
    str.append("<option value=\"");
    sprintf(allocatedPins._localConvertBuffer,"%d",i + 1);
    str.append(allocatedPins._localConvertBuffer);
    str.append("\"");
    if ((parameters.IsOutput(pin)) && (parameters.GetActuatorIdOnPin(pin) == (i + 1)))
    {
      str.append(" selected >");
    } else {
      str.append(">");
    }
    str.append(allocatedPins._supportedActuators[i]); 
    str.append("</option>");
  }
  str.append("</select>");
}

void PinAttribution::HandlePinsConfiguration(ESP8266WebServer &webServer)
{
  int i;
  std::string str;
  str.append("<!DOCTYPE html> <html lang=\"fr-FR\">\n");
  str.append("<head><title>Pin attribution</title><script>\n");
  str.append("function HideOrShow(elem,disp)\n{document.getElementById(elem).style.display = disp;}\n");
  for(i=1;i<=8;i++)
  { 
    str.append("function ShowSensorsOrActuators");
    sprintf(allocatedPins._localConvertBuffer,"%d",i);
    str.append(allocatedPins._localConvertBuffer);  
    str.append("(select)\n{ if (select.value == 'Sensor')\n{");
    str.append(" HideOrShow('ActuatorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none'); HideOrShow('SensorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','inline'); HideOrShow('PlsD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none');}\n");
    str.append("else if (select.value == 'Actuator')\n{HideOrShow('ActuatorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','inline'); HideOrShow('SensorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none');");
    str.append(" HideOrShow('PlsD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none');\n } else  {\n HideOrShow('ActuatorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none');  HideOrShow('SensorD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','none');");
    str.append("HideOrShow('PlsD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("','inline');\n }};");
  }
  str.append("\n</script>\n");
  Parameters::AppendCss(str);
  
  str.append("</head>\n<body>\n<H1>");
  str.append(parameters.DeviceName().c_str());
  str.append(" I/O setup</H1>\n<HR><div>");
  str.append("<form class=\"grid3column\" method=\"POST\" action=\"confPins\">\n");
//  str.append("<TABLE class=\"center\"><TR><TH> Pins </TH><TH> Type </TH><TH> Label </TH></TR>\n");

  std::string selectName;
  for (i =1;i<=8;i++)
  {
    sprintf(allocatedPins._localConvertBuffer,"%d",i);
//    str.append("<TR><TD> D");
    str.append("<label>D");
    str.append(allocatedPins._localConvertBuffer);  
    str.append(" </label> <select class=\"gridpos2\" id=\"DirectionD");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("\" name=\"DirectionD");  
    str.append(allocatedPins._localConvertBuffer);  
    str.append("\" onchange=ShowSensorsOrActuators");
    str.append(allocatedPins._localConvertBuffer);  
    str.append("(this)> <option value=\"0\"");
    if ((!parameters.IsInput(i)) && (!parameters.IsOutput(i)))
    {
      str.append(" selected ");
    }
    str.append(">Select type</option> <option value=\"Sensor\"");
    if (parameters.IsInput(i))
    {
      str.append(" selected ");
    }
    str.append(">Sensor</option> <option value=\"Actuator\"");
    if (parameters.IsOutput(i))
    {
      str.append(" selected ");
    }
    str.append(">Actuator</option></select>\n");
   
    selectName = "SensorD";
    selectName.append(allocatedPins._localConvertBuffer);  
    AppendSensorSelect(str,selectName,i);

    selectName = "ActuatorD";
    sprintf(allocatedPins._localConvertBuffer,"%d",i);
    selectName.append(allocatedPins._localConvertBuffer);  
    AppendActuatorSelect(str,selectName,i);

    sprintf(allocatedPins._localConvertBuffer,"%d",i);
    str.append(" <select class=\"gridpos3\" id=\"PlsD");
    str.append(allocatedPins._localConvertBuffer);
    str.append("\" name=\"PlsD");
    str.append(allocatedPins._localConvertBuffer);
    str.append("\" style='display:");
    if ((!parameters.IsInput(i)) && (!parameters.IsOutput(i)))
    {
      str.append("inline");
    } else {
      str.append("none");
    }
    str.append(";\'><option value=\"0\" selected > Select type </option></select>\n"); 
  }
   
  str.append("<input class=\"gridposcenteredon3\" value=\"Save to memory\" type=\"submit\"></form></div>");

  Parameters::AppendSaveToFlash(str);

  str.append("</body>\n</HTML>");
  
  webServer.send(200, "text/html", str.c_str());
}


void PinAttribution::ToggleOnboardLed(void)
{
  if (_onboardLedState == LOW)
    _onboardLedState = HIGH;
  else
    _onboardLedState  = LOW;

  digitalWrite(BUILTIN_LED, _onboardLedState);
}

 uint8_t PinAttribution::PinToDPin(uint8_t pin)
 {
  if (pin == 0)
    return D0;
  else if (pin == 1)
    return D1;
  else if (pin == 2)
    return D2;
  else if (pin == 3)
    return D3;
  else if (pin == 4)
    return D4;
  else if (pin == 5)
    return D5;
  else if (pin == 6)
    return D6;
  else if (pin == 7)
    return D7;
  else if (pin == 8)
    return D8;
 }

 uint8_t PinAttribution::DPinToPin(uint8_t dPin)
 {
    if (dPin == D0)
    return 0;
  else if (dPin == D1)
    return 1;
  else if (dPin == D2)
    return 2;
  else if (dPin == D3)
    return 3;
  else if (dPin == D4)
    return 4;
  else if (dPin == D5)
    return 5;
  else if (dPin == D6)
    return 6;
  else if (dPin == D7)
    return 7;
  else if (dPin == D8)
    return 8;
 }
