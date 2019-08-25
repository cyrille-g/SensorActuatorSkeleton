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

#ifndef _CGE_GENERIC_ACTUATOR_H
#define _CGE_GENERIC_ACTUATOR_H

#include <list>
#include <string>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>

typedef enum 
{
  ACTUATOR_IR,
  ACTUATOR_UNDEF = 0xFF
} actuatorType_t;

class GenericActuator {
  public:
    virtual std::string *GenerateWebData(void) = 0;
    virtual void ProcessWebCommand(ESP8266WebServer *pWebServer) = 0;
    virtual void PublishMqttState(PubSubClient &mqttClient) = 0;
    virtual bool ProcessMqttCommand(char* message) = 0;

    bool SubscribeToMqttSetTopic(PubSubClient &mqttClient);

    bool isItMyName(char *name) { return (strcmp(name, _actuatorName.c_str())== 0);}
    bool isItMySetTopic(char *setTopic)  { return (strcmp(setTopic, _setTopic.c_str())== 0);}
    
    static std::list<GenericActuator *> FindActuators(int pin);
    
  protected:
    std::string _actuatorName;
    actuatorType_t _actuatorType;
    
    std::string _stateTopic;
    std::string _setTopic;

    private:
    /* declare your actuator finders here. example:
    static std::list<GenericActuator *> FindAircon(int pin);
    */
};
  

#endif
