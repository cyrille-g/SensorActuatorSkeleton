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

#ifndef _CGE_PARAMETERS_H
#define _CGE_PARAMETERS_H

#include <ESP8266WebServer.h>
#include <string>

typedef enum {
  CGE_NONE = 0,
  CGE_INPUT = 1,
  CGE_OUTPUT = 2
} CGE_PinDirection_t;

class Parameters
{
  public:

    static void AppendCss(std::string &str);
    static void AppendSaveToFlash(std::string &Str);

    Parameters(void);
      
    void HandleConfWifi(ESP8266WebServer &webServer);
    void HandleConfMqtt(ESP8266WebServer &webServer);
    void HandleConfNtp(ESP8266WebServer &webServer);
    void HandleConfDeviceName(ESP8266WebServer &webServer);
    void HandleConfConf(ESP8266WebServer &webServer);
    void HandleConfPins(ESP8266WebServer &webServer);
    void HandleSaveToFlash(ESP8266WebServer &webServer);
    void HandleConfiguration(ESP8266WebServer &webServer);
    
    std::string &DeviceName(void) { return _deviceName;}
  
    std::string &MainSsid(void) { return _mainSsid; }
    std::string &MainSsidPwd(void) { return _mainSsidPwd; }
    std::string &BackupSsid(void) { return _mainSsid; }
    std::string &BackupSsidPwd(void) { return _mainSsidPwd; }

    std::string MqttIp(void) {return _mqttIp;}
    int MqttPort(void) { return _mqttPort;}
    std::string MqttUsername(void) { return _mqttUsername; }
    std::string MqttPwd(void) { return _mqttPwd;}
    std::string MqttPrefix(void) { return _mqttPrefix;}
    
    std::string NtpIp(void) {return _ntpIp; }
    int NtpPort(void) { return _ntpPort; }
    
    std::string ConfigurationSsid(void) { return _configurationSsid;}
    std::string ConfigurationPwd(void) { return _configurationPwd; }

    int WebserverPort(void) { return _webserverPort; }

    int OtaPort(void) { return _otaPort;}
    std::string OtaPwd(void) { return  _otaPwd; }

    bool IsInput(uint8_t pin);
    bool IsOutput(uint8_t pin);
    uint8_t GetSensorIdOnPin(uint8_t pin);
    uint8_t GetActuatorIdOnPin(uint8_t pin);


    void begin(void);
    
    
  private:
    char _localConvertBuffer[10];
  
    std::string _deviceName;
    
    std::string _mainSsid;
    std::string _mainSsidPwd;
    std::string _backupSsid;
    std::string _backupSsidPwd;
    
    std::string _mqttIp;
    uint16_t _mqttPort;
    std::string _mqttUsername;
    std::string _mqttPwd;
    std::string _mqttPrefix;
    
    std::string _ntpIp;
    uint16_t _ntpPort;
    
    std::string _configurationSsid;
    std::string _configurationPwd;
    uint16_t _webserverPort;    

    uint16_t _otaPort;
    std::string _otaPwd;  

    CGE_PinDirection_t _direction[8]; /* 0: not selected; 1: INPUT (sensor) 2: OUTPUT (actuator) */
    uint8_t _actuatorSensor[8]; /* we depend on the direction bitfield to interpret this value either as an actuator or a sensor */

    unsigned long int _lastSaveAt;

    int SaveStringInEeprom(int addr,std::string data);
    bool SaveInEeprom(void);
    int LoadStringFromEeprom(int addr, std::string &str);
    bool LoadFromEeprom(void);
    void ResetToDefault(void);

};

extern Parameters parameters;

#endif
