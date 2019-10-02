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

void DHTSensor::AppendWebData(std::string &str)
{
  
  
  char localConvertBuffer[20];
  str.append("<div class=SAparagraph>");
  /* add name */
  str.append(_sensorName);
  str.append("<BR><BR> Mqtt state ");
  str.append(_stateTopic);
  str.append("<BR><p style=\"color:");

  /* add temperature */
  if ( isnan(temperature))
  {
    str.append("red\"> --Temperature unavailable -- ");
  } else {
    str.append(":green\"> Temperature ");
    sprintf(localConvertBuffer,"%f &#176;C",temperature);
    str.append(localConvertBuffer);
  }

  str.append("</p>");

  str.append("<p style=\"color:");
  /* add humidity and comfort values */
  if ( isnan(humidity))
  {
    str.append("red\"> -- Humidity unavailable -- ");
  } else {
    str.append("green\"> Humidity ");
   sprintf(localConvertBuffer,"%f %",humidity);
    str.append(localConvertBuffer);
  }
  str.append("</p>");

  if ((!isnan(temperature)) && (!isnan(humidity)))
  {
    float heatIndex = computeHeatIndex(temperature,humidity);
    double dewPoint = computeDewPoint(temperature,humidity);
    ComfortState comfortState;
    float comfortRatio = getComfortRatio(comfortState,temperature,humidity);
    str.append(" Heat index: ");
    itoa(heatIndex,localConvertBuffer,10);
    str.append(localConvertBuffer);
    str.append("<BR> Dew point: ");
    itoa(dewPoint,localConvertBuffer,10);
    str.append(localConvertBuffer);
    str.append("<BR> Comfort ratio: ");
    itoa(comfortRatio,localConvertBuffer,10);
    str.append(localConvertBuffer);
  
    str.append("<BR> ");
    switch(comfortState)
    {
       case Comfort_OK:
         str.append("Temperature and humidity ok");
       break;
       case Comfort_TooHot:
        str.append("Too hot");
       break;
       case Comfort_TooCold:
        str.append("Too cold");
       break;
       case Comfort_TooDry:
        str.append("Too dry");
       break;
       case Comfort_TooHumid:
        str.append("Too dry");
       break;
       case Comfort_HotAndHumid:
        str.append("Too hot and too humid");
       break;
       case Comfort_HotAndDry:
        str.append("Too hot and too dry");
       break;
       case Comfort_ColdAndHumid:
        str.append("Too cold and too humid");
       break;
       case Comfort_ColdAndDry:
        str.append("Too cold and too dry");
       break;
       default:
        str.append("unknown (something is broken !)");
       break;
    }
  }
  str.append("</div>");

} 


void DHTSensor::UpdateSensor(void)
{
  readSensor();
}
