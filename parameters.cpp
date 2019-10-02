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


#include "parameters.h"

#include "settings.h"
#include <LogManagement.h>
#include <EEPROM.h>

Parameters parameters;

void Parameters::ResetToDefault(void)
{
    _deviceName = DEVICENAME;
   
    _mainSsid = WIFISSID1;
    _mainSsidPwd = WIFIPWD1;
    _backupSsid = WIFISSID2;
    _backupSsidPwd = WIFIPWD2;
      
    _mqttIp = MQTT_SERVER_IP;
    _mqttPort = MQTT_PORT;
    _mqttUsername = MQTT_USERNAME;
    _mqttPwd = MQTT_PWD;
    _mqttPrefix = MQTT_PREFIX;
    
    _ntpIp = NTP_SERVER;
    _ntpPort = NTP_SERVER_PORT;
      
    _configurationSsid = WIFI_CONFIGURATION_SSID;
    _configurationPwd = WIFI_CONFIGURATION_PWD;
    _webserverPort = WEBSERVER_PORT;
    
    _otaPort = OTAPORT;
    _otaPwd = OTAPASSWORD;  

    for(int i=0;i<8;i++)
    {
      _direction[i] = CGE_NONE;
      _actuatorSensor[i] = 0;    
    }
}

Parameters::Parameters(void)
{
  ResetToDefault();
  _lastSaveAt = 0;
}

void Parameters::begin(void)
{
  LoadFromEeprom();
}

void Parameters::HandleConfPins(ESP8266WebServer &webServer)
{
  String argName;
  std::string webValue;
  LOG("Pins web setup arg count ")
  LOG_LN(webServer.args())
  
  uint8_t pin = 0;
  /* we need to do this in 2 passes as the second processing depends on the first */
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();

    if (argName.startsWith("DirectionD"))
    {
      pin = argName.charAt(10) - '0';
      LOG("set pin ")
      LOG(pin)
      if (webValue.compare("Sensor") == 0)
      {
       LOG_LN(" as sensor")
        _direction[pin - 1] = CGE_INPUT;
      } else if (webValue.compare("Actuator") == 0)
      {
       LOG_LN(" as actuator")
        _direction[pin - 1] = CGE_OUTPUT;
      } else {
         LOG_LN(" as not used")
        _direction[pin - 1] = CGE_NONE;
      }
    }
  }

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();

    if (argName.startsWith("SensorD"))
    {
      pin = argName.charAt(7) -'0';
      if (_direction[pin - 1] == CGE_INPUT)
      {
        _actuatorSensor[pin - 1] = atoi(webValue.c_str());
        LOG("Set sensor type on pin D")
        LOG(pin)
        LOG(" as ")
        LOG_LN(_actuatorSensor[pin - 1])
      }
    } 
    else if (argName.startsWith("ActuatorD"))
    {
      pin = argName.charAt(9) - '0';
      if (_direction[pin - 1] == CGE_OUTPUT)
      {
        _actuatorSensor[pin-1] = atoi(webValue.c_str());
        LOG("Set actuator type on pin D")
        LOG(pin)
        LOG(" as ")
        LOG_LN(_actuatorSensor[pin - 1])
      } 
    }
  }
}

void Parameters::HandleConfWifi(ESP8266WebServer &webServer)
{
  String argName;
  std::string webValue;
  LOG_LN("Wifi web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();
    if (argName == "u_ssid1")
      { LOG_LN("updated main SSID") _mainSsid = webValue;}
    else if ((argName == "u_pwd1") && (webValue[0]!='\0'))
      { LOG_LN("updated main wifi pwd") _mainSsidPwd = webValue;}
    if (argName == "u_ssid2")
      { LOG_LN("updated backup SSID") _backupSsid = webValue;}
    else if ((argName == "u_pwd2") && (webValue[0]!='\0'))
      { LOG_LN("updated backup wifi pwd") _backupSsidPwd = webValue;}
  }
}

void Parameters::HandleConfMqtt(ESP8266WebServer &webServer)
{
  String argName;
  std::string webValue;
  LOG_LN("MQTT web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();
    if (argName == "u_mqttIp")
      { LOG_LN("updated MQTT server IPand") _mqttIp = webValue;}
    else if (argName == "u_mqttPort")
      { LOG_LN("updated MQTT port") _mqttPort = atoi(webValue.c_str());}
    else if (argName == "u_mqttUsername")
      { LOG_LN("updated MQTT username") _mqttUsername = webValue;}
    else if ((argName == "u_mqttPwd") && (webValue[0]!='\0'))
      { LOG_LN("updated MQTT pwd") _mqttPwd = webValue;}
    else if (argName == "u_mqttPrefix")
      { LOG_LN("updated MQTT prefix") _mqttPrefix = webValue;}
  }
}

void Parameters::HandleConfNtp(ESP8266WebServer &webServer)
{
  String argName;
  std::string webValue;
  
  LOG_LN("NTP web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();
    if (argName == "u_ntpIp")
      { LOG_LN("updated NTP server IP") _ntpIp = webValue;}
    else if (argName == "u_ntpPort")
      { LOG_LN("updated NTP server port") _ntpPort = atoi(webValue.c_str());}
  }
}

void Parameters::HandleConfDeviceName(ESP8266WebServer &webServer)
{
 
  LOG_LN("Device name web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    if (webServer.argName(i) == "u_devicename")
      { LOG_LN("updated device name") _deviceName = webServer.arg(i).c_str();}
  }
}

void Parameters::HandleSaveToFlash(ESP8266WebServer &webServer)
{
  LOG_LN("Save to flash web handler")
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    if ((webServer.argName(i) == "u_confirmSave") && 
    ((_lastSaveAt == 0) || (_lastSaveAt + EEPROM_MIN_TIME_BETWEEN_SAVE_MS < millis())))
      {
         LOG_LN("saving to eeprom")
        /* we never saved, or we waited long enough. do it */
        _lastSaveAt = millis();
        SaveInEeprom();
      }
  }
}

void Parameters::HandleConfConf(ESP8266WebServer &webServer)
{
  std::string argName;
  std::string webValue;
  LOG_LN("Setup web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i).c_str();
    webValue = webServer.arg(i).c_str();
    if (argName == "u_confSsid")
      { LOG_LN("updated setup SSID") _configurationSsid = webValue;}
    else if ((argName == "u_confPwd") && (webValue[0]!='\0'))
      { LOG_LN("updated conf pwd") _configurationPwd = webValue;}
    else if (argName == "u_confOtaPort")
      { LOG_LN("updated conf ota port") _otaPort = atoi(webValue.c_str());}
    else if (argName == "u_otaPwd")
      { LOG_LN("updated ota pwd") _otaPwd = webValue;}
    else if (argName == "u_confWebserverPort")
      { LOG_LN("updated webserver port") _webserverPort = atoi(webValue.c_str());}
    else if (argName == "u_otaPort")
      { LOG_LN("updated OTA port") _otaPort = atoi(webValue.c_str());}
    else if ((argName == "u_otaPwd") && (webValue[0]!='\0'))
      { LOG_LN("updated OTA pwd") _otaPwd = webValue;}
      
  }
}


int Parameters::SaveStringInEeprom(int addr,std::string data)
{
  if (addr > 512) return addr;
  uint8_t strSize = data.length();
  if ((addr + 1 +strSize)> 512) return addr;
  
  int i;
  EEPROM.write(addr,strSize);
  addr++;
  for(i=0;i<strSize;i++)
  {
    EEPROM.write(addr,data[i]);
    addr++;
  }
  return addr;
}
 

bool Parameters::SaveInEeprom(void)
{
  /* write magic number */  
  EEPROM.put(EEPROM_MAGIC_NUMBER_ADDR, EEPROM_MAGIC_NUMBER_VALUE);
  int nextaddress = EEPROM_MAGIC_NUMBER_ADDR + sizeof(EEPROM_MAGIC_NUMBER_VALUE);
   
  nextaddress = SaveStringInEeprom(nextaddress, _deviceName);
  nextaddress = SaveStringInEeprom(nextaddress, _mainSsid);    
  nextaddress = SaveStringInEeprom(nextaddress, _mainSsidPwd);    
  nextaddress = SaveStringInEeprom(nextaddress, _backupSsid);    
  nextaddress = SaveStringInEeprom(nextaddress, _backupSsidPwd);    
  nextaddress = SaveStringInEeprom(nextaddress, _mqttIp);    
  EEPROM.put(nextaddress, _mqttPort);    
  nextaddress+=sizeof(_mqttPort);
  nextaddress = SaveStringInEeprom(nextaddress, _mqttUsername);    
  nextaddress = SaveStringInEeprom(nextaddress, _mqttPwd);    
  nextaddress = SaveStringInEeprom(nextaddress, _mqttPrefix);
  nextaddress = SaveStringInEeprom(nextaddress, _ntpIp);    
  EEPROM.put(nextaddress,_ntpPort);    
  nextaddress+=sizeof(_ntpPort);
  nextaddress = SaveStringInEeprom(nextaddress, _configurationSsid);    
  nextaddress = SaveStringInEeprom(nextaddress, _configurationPwd);    
  EEPROM.put(nextaddress,_webserverPort);    
  nextaddress+=sizeof(_webserverPort);
  EEPROM.put(nextaddress, _otaPort);    
  nextaddress+=sizeof(_otaPort);
  nextaddress = SaveStringInEeprom(nextaddress, _otaPwd);  
  for(int i=0;i<8;i++)
  {
      EEPROM.put(nextaddress, _direction[i]);    
      nextaddress++;
  }
    for(int i=0;i<8;i++)
  {
      EEPROM.put(nextaddress, _actuatorSensor[i]);    
      nextaddress++;
  }
  EEPROM.commit();
  return true;
}

int Parameters::LoadStringFromEeprom(int addr, std::string &str)
{
  int i;
  str = "";
  
  if (addr > 512)
  {
    LOG_LN("Load eeprom: addr past end")
    return 0;
  }
  uint8_t strSize = EEPROM.read(addr);
  
  if ((addr + strSize + 1)> 512 )
  {
    LOG_LN("Load eeprom: addr + size past end")
    return 0;
  }

  char *pStr = (char *)malloc(strSize + 1);
  addr++;
  for(i=0;i<strSize;i++)
  {
    *(pStr+i) =EEPROM.read(addr);
    addr++;
  }
  *(pStr+i) = '\0';

  str.append(pStr);
  free(pStr);
  return addr;
}

bool Parameters::LoadFromEeprom(void)
{
  EEPROM.begin(512);  //Initialize EEPROM
  bool ret = false;
  /* check magic number */  
  uint32_t magicValue;
  LOG_LN("EEprom content");
  EEPROM.get(EEPROM_MAGIC_NUMBER_ADDR, magicValue);

  if (magicValue != EEPROM_MAGIC_NUMBER_VALUE)
    return ret;
  LOG_LN("magic value ok");
  int nextaddress = EEPROM_MAGIC_NUMBER_ADDR + EEPROM_MAGIC_NUMBER_SIZE;
   
  nextaddress = LoadStringFromEeprom(nextaddress, _deviceName);
  LOG_LN(_deviceName.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _mainSsid);    
  LOG_LN(_mainSsid.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _mainSsidPwd);    
  LOG_LN(_mainSsidPwd.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _backupSsid);    
  LOG_LN(_backupSsid.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _backupSsidPwd);    
  LOG_LN(_backupSsidPwd.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _mqttIp);    
  LOG_LN(_mqttIp.c_str())
  EEPROM.get(nextaddress, _mqttPort);    
  LOG_LN(_mqttPort)
  nextaddress+=sizeof(_mqttPort);
  nextaddress = LoadStringFromEeprom(nextaddress, _mqttUsername);    
  LOG_LN(_mqttUsername.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _mqttPwd);    
  LOG_LN(_mqttPwd.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _mqttPrefix);
  LOG_LN(_mqttPrefix.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _ntpIp);    
  LOG_LN(_ntpIp.c_str())
  EEPROM.get(nextaddress,_ntpPort);    
  LOG_LN(_ntpPort)
  nextaddress+=sizeof(_ntpPort);
  nextaddress = LoadStringFromEeprom(nextaddress, _configurationSsid);    
  LOG_LN(_configurationSsid.c_str())
  nextaddress = LoadStringFromEeprom(nextaddress, _configurationPwd);    
  LOG_LN(_configurationPwd.c_str())
  EEPROM.get(nextaddress,_webserverPort);    
  LOG_LN(_webserverPort)
  nextaddress+=sizeof(_webserverPort);
  EEPROM.get(nextaddress, _otaPort);    
  LOG_LN(_otaPort)
  nextaddress+=sizeof(_otaPort);
  nextaddress = LoadStringFromEeprom(nextaddress, _otaPwd);    
  LOG_LN(_otaPwd.c_str())

  for(int i=0;i<8;i++)
  {
      EEPROM.get(nextaddress, _direction[i]);    
      nextaddress++;
      LOG("Pin D")
      sprintf(parameters._localConvertBuffer,"%d",i + 1);
      LOG(parameters._localConvertBuffer)
      LOG(" set as ")
      LOG_LN(_direction[i]==CGE_INPUT?"sensor":_direction[i]==CGE_OUTPUT?"actuator":"none")
  }
  
  for(int i=0;i<8;i++)
  {
      EEPROM.get(nextaddress, _actuatorSensor[i]);    
      nextaddress++;
      LOG(" type ")
      LOG_LN(_actuatorSensor[i])
  }

  return true;
}

void Parameters::AppendCss(std::string &str)
{
str.append("<style>\n");
str.append(".greenboldtext{ color: #008080; font-weight: bold;}\n");
str.append(".redboldtext{ color: #800080; font-weight: bold;}\n");
str.append("label{text-align:left;padding: 5px 5px; grid-column: 1 / 2;}\n");
str.append(".SAparagraph {display: inline-block; width: 500px;margin: 0 auto;  border: 1px solid #CCC; border-radius: 1em;}\n");
str.append("input[type=text],checkbox,select {padding:5px 5px;margin:auto;border:1px solid #ccc;border-radius: 4px; box-sizing: border-box;}\n");

str.append(".grid2column {display: grid;grid-template-columns: max-content max-content; grid-gap: 2px; column-gap: 1rem;}\n");
str.append(".grid3column {display: grid;grid-template-columns: max-content max-content max-content; grid-gap: 2px; column-gap: 1rem;}\n");
str.append(".gridpos1 {grid-column: 1 / 2;}\n");
str.append(".gridpos2 {grid-column: 2 / 3;}\n");
str.append(".gridpos3 {grid-column: 3 / 4;}\n");
str.append(".gridposcenteredon3 {grid-column: 1 / 4;}\n");
str.append(".gridposcenteredon2 {grid-column: 1 / 3;}\n");
str.append("input[type=text] {grid-column: 2 / 3; }\n");
str.append("input[type=submit]:hover {background-color: #45a049;}\n");
str.append("input[type=submit] {width:90%; background-color: #4CAF50; color: white;padding: 6px; margin: 10px auto;\n");
str.append("border: none; border-radius: 4px;cursor: pointer;}\n");
//str.append("form {display:grid;grid-template-columns:max-content max-content; grid-gap:2px; column-gap: 1rem;\n");
str.append("form {width:auto; margin: 0 auto; padding: 1em; border: 1px solid #CCC; border-radius: 1em; background-color: #f2f2f2;}\n");
str.append("h1,hr,div{text-align:center;} hr {width:60%} body {text-align:center;}\n");
str.append("div.stickyMenu {width:180px;position:fixed;bottom:0;right:30px;padding:5px; margin:5px;text-align=right;}\n");
str.append("div {margin:auto;width:min-content;border-radius:2px; padding:10px;}\n");
 
str.append("</style>\n");
}

void Parameters::AppendSaveToFlash(std::string &Str)
{
  Str.append("<div class=\"stickyMenu\"><form class=\"grid2column\" method=\"POST\" action=\"saveToFlash\">\n");
  Str.append("<input value=\"Save to flash\" type=\"submit\">\n");
  Str.append("<label>Check to confirm</label> <input class=\"gridpos2\" type=\"checkbox\" name=\"u_confirmSave\"> </form></div>\n");
}

void Parameters::HandleConfiguration(ESP8266WebServer &webServer)
{  
  std::string resp("<!DOCTYPE html><html lang=\"fr-FR\"><head><title>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" setup</title>\n");
  Parameters::AppendCss(resp);
  resp.append("</head>\n<body>\n<H1>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" setup</H1>\n<BR><HR><H2>Device name</H2>\n\n");

  Parameters::AppendSaveToFlash(resp);
  
  resp.append("\n<div><form class=\"grid2column\" method=\"POST\" action=\"confDevicename\">\n<label for=\"name\">Device name</label>\n");
  resp.append("<input type=\"text\" id=\"name\" name=\"u_name\" placeholder=\"");
  resp.append(_deviceName);
  resp.append("\"><BR>\n<input class=\"gridposcenteredon2\" value=\"Save to memory\" type=\"submit\"></form>\n");
  
  resp.append("<BR><HR><H2>Wifi parameters</H2>\n");
  resp.append("<form class=\"grid2column\" method=\"POST\" action=\"confWifi\">\n<label for=\"ssid1\">Main SSID</label>\n");
  resp.append("<input type=\"text\" id=\"ssid1\" name=\"u_ssid1\" placeholder=\"");
  resp.append(_mainSsid);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"ssid1pwd\">Main wifi pwd</label>");
  resp.append("<input type=\"text\" id=\"ssid1pwd\" name=\"u_pwd1\" placeholder=\"*********\"><BR>\n");
  resp.append("<label for=\"ssid2\">Backup SSID</label>\n");
  resp.append("<input type=\"text\" id=\"ssid2\" name=\"u_ssid2\" placeholder=\"");
  resp.append(_backupSsid);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"ssid2pwd\">Backup wifi pwd</label>\n");
  resp.append("<input type=\"text\" id=\"ssid2pwd\" name=\"u_pwd2\" placeholder=\"*********\"><BR>\n");
  resp.append("<input class=\"gridposcenteredon2\" value=\"Save to memory\" type=\"submit\"></form>\n");
  
  resp.append("<BR><HR><H2>Mqtt parameters</H2>\n");
  resp.append("<form class=\"grid2column\" method=\"POST\" action=\"confMqtt\">\n<label for=\"mqttIp\">MQTT server IP</label>\n");
  resp.append("<input type=\"text\" id=\"mqttIp\" name=\"u_mqttIp\" placeholder=\"");
  resp.append(_mqttIp);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"mqttPort\">MQTT server port</label>\n");
  resp.append("<input type=\"text\" id=\"mqttPort\" name=\"u_mqttPort\" placeholder=\"");
  itoa(_mqttPort,_localConvertBuffer,10);
  resp.append(_localConvertBuffer);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"mqttUsername\">MQTT username</label>\n");
  resp.append("<input type=\"text\" id=\"mqttUsername\" name=\"u_mqttUsername\"  placeholder=\"");
  resp.append(_mqttUsername);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"mqttPwd\">MQTT password</label>\n");
  resp.append("<input type=\"text\" id=\"mqttPwd\" name=\"u_mqttPwd\" placeholder=\"*********\"><BR>");
  resp.append("<label for=\"mqttPrefix\">MQTT prefix</label>\n");
  resp.append("<input type=\"text\" id=\"mqttPrefix\" name=\"u_mqttPrefix\" placeholder=\"");
  resp.append(_mqttPrefix);
  resp.append("\">\n<BR><input class=\"gridposcenteredon2\" value=\"Save to memory\" type=\"submit\"></form>\n");

  resp.append("<BR><HR><H2>NTP parameters</H2>\n");
  resp.append("<form class=\"grid2column\" method=\"POST\" action=\"confNtp\">\n<label for=\"ntpIp\">NTP server IP</label>\n");
  resp.append("<input type=\"text\" id=\"ntpIp\" name=\"u_ntpIp\"  placeholder=\"");
  resp.append(_ntpIp);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"ntpPort\">NTP server port</label>\n");
  resp.append("<input type=\"text\" id=\"ntpPort\" name=\"u_ntpPort\"  placeholder=\"");
  itoa(_ntpPort,_localConvertBuffer,10);
  resp.append(_localConvertBuffer);
  resp.append("\">\n<BR><input class=\"gridposcenteredon2\" value=\"Save to memory\" type=\"submit\"></form>\n");

  resp.append("<BR><HR><H2>Setup parameters</H2>\n");
  resp.append("<form class=\"grid2column\" method=\"POST\" action=\"confConf\">\n<label for=\"confSsid\">Setup SSID</label>\n");
  resp.append("<input type=\"text\" id=\"confSsid\" name=\"u_confSsid\" placeholder=\"");
  resp.append(_configurationSsid);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"confPwd\">Setup pwd</label>\n");
  resp.append("<input type=\"text\" id=\"confPwd\" name=\"u_confPwd\" placeholder=\"*********\" ><BR>\n");
  resp.append("<label for=\"confPort\">Webserver port</label>\n");
  resp.append("<input type=\"text\" id=\"confPort\" name=\"u_confWebserverPort\" placeholder=\"");
  itoa(_webserverPort,_localConvertBuffer,10);
  resp.append(_localConvertBuffer);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"otaPort\">Ota port</label>\n");
  resp.append("<input type=\"text\" id=\"otaPort\" name=\"u_otaPort\" placeholder=\"");
  itoa(_otaPort,_localConvertBuffer,10);
  resp.append(_localConvertBuffer);
  resp.append("\"><BR>\n");
  resp.append("<label for=\"otaPwd\">Ota pwd</label>\n");
  resp.append("<input type=\"text\" id=\"otaPwd\" name=\"u_otaPwd\" placeholder=\"*********\">\n");
  resp.append("<BR><input class=\"gridposcenteredon2\" value=\"Save to memory\" type=\"submit\">\n</form>\n</div>\n</body>\n</html>");
 
  webServer.send(200, "text/html", resp.c_str());
 }

bool Parameters::IsInput(uint8_t pin)
{
  if (pin < 7)
    return (_direction[pin - 1] == CGE_INPUT);  
  else
    return false;
}

bool Parameters::IsOutput(uint8_t pin)
{
  if (pin < 7)
    return (_direction[pin - 1] == CGE_OUTPUT);  
  else
    return false;
}

uint8_t Parameters::GetSensorIdOnPin(uint8_t pin)
{
  if (pin < 7)
    return _actuatorSensor[pin - 1];  
  else
    return -1;
}

uint8_t Parameters::GetActuatorIdOnPin(uint8_t pin)
{
  if (pin < 7)
    return _actuatorSensor[pin - 1];  
  else
    return -1;
}
