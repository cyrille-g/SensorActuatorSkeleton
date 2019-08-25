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

#include <ArduinoJson.h>

#include "DS18Sensor.h"

DS18Sensor::DS18Sensor(uint8_t pin)
{
  _pin = pin;
  _pOneWire = new OneWire(pin);
  _pDallasTemp = new DallasTemperature(_pOneWire);
  _minValidTemp = -55.0;
  _maxValidTemp = 125.0;
  _sensorName = "";
  _stateTopic = "";
}

DS18Sensor::~DS18Sensor(void)
{
  delete _pOneWire;
  delete  _pOneWire;
}

inline byte *DS18Sensor::GetDeviceAddress(void)
{
  return _sensorAddress;
}

bool DS18Sensor::begin(void)
{
  DallasTemperature::begin();
  return scanPinForDS18();
}

bool DS18Sensor::scanPinForDS18(void)
{
  byte i;
  byte data[12];
  byte addr[8];

  bool ret = false;
  
  if ( !_pOneWire->search(addr)) 
  {
    LOG_LN("No more addresses")
    _pOneWire->reset_search();
    return ret;
  }
  
  LOG("ROM = ");
  for( i = 0; i < 8; i++) 
  {
    LOG(' ')
    LOG("0x")
    LOG(addr[i]);
    if ( i != 7 )
    {
      LOG(", ")
    }
  }
  LOG_LN(" ")
  
  if (OneWire::crc8(addr, 7) != addr[7]) 
  {
     LOG_LN("Invalid CRC");
      return ret;
  }
 
  _sensorType = SENSOR_UNDEF;
  _stateTopic ="PIN";
  char buf[10];  
  sprintf(buf,"%d",_pin);
  _stateTopic.append(buf);

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      LOG_LN(" Found a DS18S20 sensor"); 
      _sensorType = SENSOR_DS18S20;
      _stateTopic.append("_DS18S20");
      ret = true;
      break;
    case 0x28:
      LOG_LN(" Found a DS18B20 sensor");
      _sensorType = SENSOR_DS18B20;
      _stateTopic.append("_DS18B20");
      ret = true;
      break;
    case 0x22:
      _sensorType = SENSOR_DS18B22;
      LOG_LN(" Found a DS18B22 sensor")
      _stateTopic.append("_DS18B22");
      ret = true;
    break;
    
    default:
      _stateTopic="";
      LOG_LN("Device is not a DS18xxx");
      return ret;
      break;
  } 

  memcpy(_sensorAddress,addr ,8);
  /* set resolution to 9 bits */
  setResolution(_sensorAddress, 9);  

  return ret;
}
 

void DS18Sensor::PublishMqttState(PubSubClient &mqttClient)
{
  const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& sensorRoot = jsonBuffer.createObject();

  /* add temperature */
  sensorRoot[_sensorName.c_str()]["temperature"] = _temperature;

  char *sensorBuffer = (char *)malloc(sensorRoot.measureLength() + 1);
  sensorRoot.printTo(sensorBuffer, sizeof(sensorBuffer));
  mqttClient.publish(_stateTopic.c_str(), sensorBuffer, true);
  free (sensorBuffer);
}
std::string *DS18Sensor::GenerateWebData(void)
{
  std::string *pWebData = new std::string("<TABLE><TR><TD>DS18 Sensor ");
  char localConvertBuffer[10];
  /* add name */
  pWebData->append(_sensorName);
  pWebData->append("<BR> Mqtt state ");
  pWebData->append(_stateTopic);
  pWebData->append("<BR><BR>Temperature: <input type=\"color\" value=\"");
  /* add temperature */
  itoa(_temperature,localConvertBuffer,10);
  pWebData->append(localConvertBuffer);
  if ( _temperature == -51.0)
  {
    pWebData->append("#FF0000\"> -- unavailable -- ");
  } else {
    pWebData->append("#00FF00\"> ");
    itoa(_temperature,localConvertBuffer,10);
    pWebData->append(localConvertBuffer);
    pWebData->append(" °C");
  }

  pWebData->append("</input></TD></TR></TABLE>");
  return pWebData;
}

void DS18Sensor::UpdateSensor(void)
{
  if (_pDallasTemp!=NULL)
    _pDallasTemp->requestTemperaturesByAddress(_sensorAddress);
    _temperature = _pDallasTemp->getTempC(_sensorAddress);
}
