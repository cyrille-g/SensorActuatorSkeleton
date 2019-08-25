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
#include "settings.h" 
#include <LogManagement.h>
#include "DHTSensor.h"

DHTSensor::DHTSensor (void)                       
{

};

bool DHTSensor::begin(uint8_t pin)
{
//  DHTesp::setup(pin,AUTO_DETECT);
if (pin != D5)
{
  return false;
}
else
{
 DHTesp::setup(pin,DHT22);

  _stateTopic="STATE/PIN";
  char buf[10];  
  sprintf(buf,"%d",pin);
  switch (getModel())
  {
     case DHT11:
     _stateTopic.append("_DHT11");
     _sensorType = SENSOR_DHT11;
    LOG("Found DHT11 sensor")
     break;
     
     case DHT22:
     _stateTopic.append("_DHT22");
     _sensorType = SENSOR_DHT22;
     LOG("Found DHT22 sensor")
    
     break;
        
     default:
     _sensorType = SENSOR_UNDEF;
     _stateTopic="";
      LOG("Found no sensor")
      return false;
     break;
  }
  char *pName = &_stateTopic[6];
   _sensorName.append(pName);
  return true;
}
}


void DHTSensor::PublishMqttState(PubSubClient &mqttClient)
{
  const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(10);
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& sensorRoot = jsonBuffer.createObject();

  /* first, add temperature */
  sensorRoot[_sensorName.c_str()]["temperature"] = temperature;
    
 /* add humidity and comfort values */
  sensorRoot[_sensorName.c_str()]["humidity"] = humidity;

  float heatIndex = computeHeatIndex(temperature,humidity);
  double dewPoint = computeDewPoint(temperature,humidity);
  ComfortState comfortState;
  float comfortRatio = getComfortRatio(comfortState,temperature,humidity);
  
  sensorRoot[_sensorName.c_str()]["heatIndex"] = heatIndex;
  sensorRoot[_sensorName.c_str()]["dewPoint"] = dewPoint;
  sensorRoot[_sensorName.c_str()]["comfortRatio"] = comfortRatio;
  sensorRoot[_sensorName.c_str()]["comfortState"] = comfortState;

  char *sensorBuffer = (char *)malloc(sensorRoot.measureLength() + 1);
  sensorRoot.printTo(sensorBuffer, sizeof(sensorBuffer));
  mqttClient.publish(_stateTopic.c_str(), sensorBuffer, true);
  free (sensorBuffer);
  sensorBuffer= NULL;
} 

std::string *DHTSensor::GenerateWebData(void)
{
  std::string *pWebData = new std::string("<TABLE><TR><TD>DHT Sensor ");
  char localConvertBuffer[20];
  /* add name */
  pWebData->append(_sensorName);
  pWebData->append("<BR> Mqtt state ");
  pWebData->append(_stateTopic);
  pWebData->append("<BR><BR>Temperature: <p style=\"color:");

  /* add temperature */
  if ( isnan(temperature))
  {
    pWebData->append("red\"> -- unavailable -- ");
  } else {
    pWebData->append(":green\"> ");
    sprintf(localConvertBuffer,"%f °C",temperature);
    pWebData->append(localConvertBuffer);
  }

  pWebData->append("</p>");

  pWebData->append("Humidity: <p style=\"color:");
  /* add humidity and comfort values */
  if ( isnan(humidity))
  {
    pWebData->append("red\"> -- unavailable -- ");
  } else {
    pWebData->append("green\"> ");
   sprintf(localConvertBuffer,"%f %",humidity);
    pWebData->append(localConvertBuffer);
  }
  pWebData->append("</p>");

  if ((!isnan(temperature)) && (!isnan(humidity)))
  {
    float heatIndex = computeHeatIndex(temperature,humidity);
    double dewPoint = computeDewPoint(temperature,humidity);
    ComfortState comfortState;
    float comfortRatio = getComfortRatio(comfortState,temperature,humidity);
    pWebData->append(" Heat index: ");
    itoa(heatIndex,localConvertBuffer,10);
    pWebData->append(localConvertBuffer);
    pWebData->append("<BR> Dew point: ");
    itoa(dewPoint,localConvertBuffer,10);
    pWebData->append(localConvertBuffer);
    pWebData->append("<BR> Comfort ratio: ");
    itoa(comfortRatio,localConvertBuffer,10);
    pWebData->append(localConvertBuffer);
  
    pWebData->append("<BR> ");
    switch(comfortState)
    {
       case Comfort_OK:
         pWebData->append("Temperature and humidity ok");
       break;
       case Comfort_TooHot:
        pWebData->append("Too hot");
       break;
       case Comfort_TooCold:
        pWebData->append("Too cold");
       break;
       case Comfort_TooDry:
        pWebData->append("Too dry");
       break;
       case Comfort_TooHumid:
        pWebData->append("Too dry");
       break;
       case Comfort_HotAndHumid:
        pWebData->append("Too hot and too humid");
       break;
       case Comfort_HotAndDry:
        pWebData->append("Too hot and too dry");
       break;
       case Comfort_ColdAndHumid:
        pWebData->append("Too cold and too humid");
       break;
       case Comfort_ColdAndDry:
        pWebData->append("Too cold and too dry");
       break;
       default:
        pWebData->append("unkonwn (something is broken !)");
       break;
    }
  }
  pWebData->append("<BR><BR></TD></TR></TABLE>");
  return pWebData;
} 


void DHTSensor::UpdateSensor(void)
{
  readSensor();
}
