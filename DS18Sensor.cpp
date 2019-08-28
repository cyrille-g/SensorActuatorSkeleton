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
#include "settings.h"
#include <LogManagement.h>

DS18Sensor::DS18Sensor(uint8_t pin): _pin(pin)
{
  _pOneWire = new OneWire(pin);
  this->setOneWire(_pOneWire);
  _minValidTemp = -55.0;
  _maxValidTemp = 125.0;
  _sensorName = "";
  _stateTopic = "";
   LOG_LN("Created onewire obj")
}

DS18Sensor::~DS18Sensor(void)
{
  delete _pOneWire;
}

byte *DS18Sensor::GetDeviceAddress(void)
{
  return _sensorAddress;
}

bool DS18Sensor::scan(void)
{
  begin();
  return scanPinForDS18();
}

bool DS18Sensor::scanPinForDS18(void)
{
  byte i;
  byte data[12];
  byte addr[8];

  bool ret = false;
  LOG_LN("start scanning");
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
  char buf[15];  
  sprintf(buf,"STATE/PIN%d",_pin);
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
  
   char *pName = &_stateTopic[6];
   _sensorName.append(pName);

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
void DS18Sensor::AppendWebData(std::string &str)
{
  char localConvertBuffer[10];
  str.append("<div class=SAparagraph>");
  str.append(_sensorName);
  str.append("<BR><BR>Mqtt state: ");
  str.append(_stateTopic);
  str.append("<BR><BR><span class="); 
  if ( _temperature == -51.0)
  {
    str.append("redboldtext>  -- Temperature unavailable -- </span>");
  } else {
   /* add temperature */
    str.append("greenboldtext> Temperature: ");
    itoa(_temperature,localConvertBuffer,10);
    str.append(localConvertBuffer);
    str.append(" &#176;C</span>");
  }
  str.append("</div><BR>");
}

void DS18Sensor::UpdateSensor(void)
{
      requestTemperaturesByAddress(_sensorAddress);
    _temperature = getTempC(_sensorAddress);
}
