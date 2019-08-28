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

class Parameters
{
  public:

    Parameters(void);
    static void AppendCss(std::string &str);
    void HandleConfWifi(ESP8266WebServer &webServer);
    void HandleConfMqtt(ESP8266WebServer &webServer);
    void HandleConfNtp(ESP8266WebServer &webServer);
    void HandleConfDeviceName(ESP8266WebServer &webServer);
    void HandleConfOta(ESP8266WebServer &webServer);
    void HandleConfConf(ESP8266WebServer &webServer);
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

    void begin(void);
    
    
  private:
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

    unsigned long int _lastSaveAt;

    int SaveStringInEeprom(int addr,std::string data);
    bool SaveInEeprom(void);
    int LoadStringFromEeprom(int addr, std::string &str);
    bool LoadFromEeprom(void);
    void ResetToDefault(void);
};

extern Parameters parameters;

#endif
