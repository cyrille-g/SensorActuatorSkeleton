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

#ifndef _CGE_AIRCON_H
#define _CGE_AIRCON_H

#include <ir_Daikin.h>
#include <map>
#include <list>
#include <string.h>
#include <PubSubClient.h>

#include "settings.h"

#include "GenericActuator.h"

#define SELECTION_ERROR 0xFF



class DaikinAirConditioning : public GenericActuator
{
  public:
    DaikinAirConditioning(int pin);
    ~DaikinAirConditioning();

    void SendAirConditioningCommand(void);

    /* virtual implementations */
    void PublishMqttState(PubSubClient &mqttClient);
    bool ProcessMqttCommand(char* message);
    void AppendWebData(std::string &str);
    void ProcessWebCommand(ESP8266WebServer *pWebServer);


  private:

    IRDaikinESP *_pDaikinAircon;

    uint8_t _temperature;
    uint8_t _fan;
    uint8_t _power;
    uint8_t _powerful;
    uint8_t _quiet;
    uint8_t _swingVertical;
    uint8_t _programMode;

    std::map<int, std::string> _FanSpeedStates;
    std::map<int, std::string> _ProgramStates;
    std::map<int, std::string> _OnOffStates;
   
    void AppendSelection(std::map<int, std::string> &state, char *settingName, int selected,std::string &str);

    void AppendFanSelection(std::string &str);
    void AppendProgramSelection(std::string &str);
    void AppendTemperatureSelection(std::string &str);
    void AppendPowerSelection(std::string &str);
    void AppendPowerfulSelection(std::string &str);
    void AppendSwingSelection(std::string &str);
    void AppendQuietSelection(std::string &str);

};

#endif
