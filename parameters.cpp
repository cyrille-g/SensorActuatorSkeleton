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
    else if (argName == "u_pwd1") 
      { LOG_LN("updated main wifi pwd") _mainSsidPwd = webValue;}
    if (argName == "u_ssid2")
      { LOG_LN("updated backup SSID") _backupSsid = webValue;}
    else if (argName == "u_pwd2") 
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
    else if (argName == "u_mqttPwd")
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

void Parameters::HandleConfOta(ESP8266WebServer &webServer)
{
  String argName;
  std::string webValue;
  LOG_LN("Ota web setup")

  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    argName = webServer.argName(i);
    webValue = webServer.arg(i).c_str();
    if (argName == "u_otaPort")
      { LOG_LN("updated OTA port") _otaPort = atoi(webValue.c_str());}
    else if (argName == "u_otaPwd")
      { LOG_LN("updated OTA pwd") _otaPwd = webValue;}
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

  HandleConfiguration(webServer);
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
    else if (argName == "u_confPwd")
      { LOG_LN("updated conf pwd") _configurationPwd = webValue;}
    else if (argName == "u_confOtaPort")
      { LOG_LN("updated conf ota port") _otaPort = atoi(webValue.c_str());}
    else if (argName == "u_otaPwd")
      { LOG_LN("updated ota pwd") _otaPwd = webValue;}
    else if (argName == "u_confWebserverPort")
      { LOG_LN("updated webserver port") _webserverPort = atoi(webValue.c_str());}
  }

  HandleConfiguration(webServer);
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
    
  return true;
}

void Parameters::AppendCss(std::string &str)
{
  str.append("<style>.greenboldtext{ color: #008080; font-weight: bold;}.redboldtext{ color: #800080; font-weight: bold;}");
  str.append(".SAparagraph {display: inline-block; width: 500px;margin: 0 auto;  border: 1px solid #CCC; border-radius: 1em;}");
  str.append("input[type=text],checkbox,select { width: 50%; padding: 8px 20px; margin: 0px 20px; float: right;");
  str.append("border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; } label { display: text-align: right;} input[type=submit] {");
  str.append("width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px;");
  str.append("cursor: pointer;}form {width: 486px; margin: 0 auto; padding: 1em; border: 1px solid #CCC; border-radius: 1em;background-color: #f2f2f2;}");
  str.append("input[type=submit]:hover { background-color: #45a049;} div { border-radius: 2px; background-color: #f2f2f2; padding: 10px; }</style>");
}

void Parameters::HandleConfiguration(ESP8266WebServer &webServer)
{
  char localConvertBuffer[10];
  
  std::string resp("<head><title>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" setup</title></head><body>");
  Parameters::AppendCss(resp);
  resp.append("<H1 align=center>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" setup</H1/<BR><BR>");

  resp.append("<HR align=center size=1 width=\"75%\"><H2 align=center>Device name</H2><BR>");
  resp.append("<form method=\"POST\" action=\"confDevicename\"><div><label for=\"name\">Device name: </label>");
  resp.append("<input type=\"text\" id=\"name\" name=\"u_name\" placeholder=\"");
  resp.append(_deviceName);
  resp.append("\"></div>");
  resp.append("<div><BR><input type=\"submit\"></div></form>");
  
  resp.append("<BR><HR align=center size=1 width=\"75%\"><H2 align=center>Wifi parameters</H2><BR>");
  resp.append("<form method=\"POST\" action=\"confWifi\"><div><label for=\"ssid1\">Main SSID: </label>");
  resp.append("<input type=\"text\" id=\"ssid1\" name=\"u_ssid1\" placeholder=\"");
  resp.append(_mainSsid);
  resp.append("\"></div>");
  resp.append("<div><label for=\"ssid1pwd\">Main wifi pwd: </label>");
  resp.append("<input type=\"text\" id=\"ssid1pwd\" name=\"u_ssid1pwd\"></div>");
  resp.append("<div><label for=\"ssid2\">Backup SSID: </label>");
  resp.append("<input type=\"text\" id=\"ssid2\" name=\"u_ssid2\" placeholder=\"");
  resp.append(_backupSsid);
  resp.append("\"></div>");
  resp.append("<div><label for=\"ssid2pwd\">Backup wifi pwd: </label>");
  resp.append("<input type=\"text\" id=\"ssid2pwd\" name=\"u_ssid2pwd\"></div>");
  resp.append("<div><BR><input type=\"submit\"></div></form>");
  
  resp.append("<BR><HR align=center size=1 width=\"75%\"><H2 align=center>Mqtt parameters</H2><BR>");
  resp.append("<form method=\"POST\" action=\"confMqtt\"><div><label for=\"mqttIp\">MQTT server IP: </label>");
  resp.append("<input type=\"text\" id=\"mqttIp\" name=\"u_mqttIp\" placeholder=\"");
  resp.append(_mqttIp);
  resp.append("\"></div>");
  resp.append("<div><label for=\"mqttPort\">MQTT server port: </label>");
  resp.append("<input type=\"text\" id=\"mqttPort\" name=\"u_mqttPort\" placeholder=\"");
  itoa(_mqttPort,localConvertBuffer,10);
  resp.append(localConvertBuffer);
  resp.append("\"></div>");
  resp.append("<div><label for=\"mqttUsername\">MQTT username: </label>");
  resp.append("<input type=\"text\" id=\"mqttUsername\" name=\"u_mqttUsername\"  placeholder=\"");
  resp.append(_mqttUsername);
  resp.append("\"></div>");
  resp.append("<div><label for=\"mqttPwd\">MQTT password: </label>");
  resp.append("<input type=\"text\" id=\"mqttPwd\" name=\"u_mqttPwd\"></div>");
  resp.append("<div><label for=\"mqttPrefix\">MQTT prefix: </label>");
  resp.append("<input type=\"text\" id=\"mqttPrefix\" name=\"u_mqttPrefix\" placeholder=\"");
  resp.append(_mqttPrefix);
  resp.append("\"></div>");
  resp.append("<div><BR><input type=\"submit\"></div></form>");

  resp.append("<BR><HR align=center size=1 width=\"75%\"><H2 align=center>NTP parameters</H2><BR>");
  resp.append("<form method=\"POST\" action=\"confNtp\"><div><label for=\"ntpIp\">NTP server IP: </label>");
  resp.append("<input type=\"text\" id=\"ntpIp\" name=\"u_ntpIp\"  placeholder=\"");
  resp.append(_ntpIp);
  resp.append("\"></div>");
  resp.append("<div><label for=\"ntpPort\">NTP server port: </label>");
  resp.append("<input type=\"text\" id=\"ntpPort\" name=\"u_ntpPort\"  placeholder=\"");
  itoa(_ntpPort,localConvertBuffer,10);
  resp.append(localConvertBuffer);
  resp.append("\"></div>");
  resp.append("<div><BR><input type=\"submit\"></div></form>");

  resp.append("<BR><HR align=center size=1 width=\"75%\"><H2 align=center>Setup parameters</H2><BR>");
  resp.append("<form method=\"POST\" action=\"confConf\"><div><label for=\"confSsid\">Setup SSID: </label>");
  resp.append("<input type=\"text\" id=\"confSsid\" name=\"u_confSsid\" placeholder=\"");
  resp.append(_configurationSsid);
  resp.append("\"></div>");
  resp.append("<div><label for=\"confPwd\">Setup pwd: </label>");
  resp.append("<input type=\"text\" id=\"confPwd\" name=\"u_confPwd\"></div>");
  resp.append("<div><label for=\"confPort\">Webserver port: </label>");
  resp.append("<input type=\"text\" id=\"confPort\" name=\"u_confWebserverPort\" placeholder=\"");
  itoa(_webserverPort,localConvertBuffer,10);
  resp.append(localConvertBuffer);
  resp.append("\"></div>");
  resp.append("<div><label for=\"otaPort\">Ota port: </label>");
  resp.append("<input type=\"text\" id=\"otaPort\" name=\"u_otaPort\" placeholder=\"");
  itoa(_otaPort,localConvertBuffer,10);
  resp.append(localConvertBuffer);
  resp.append("\"></div>");
  resp.append("<div><label for=\"otaPwd\">Ota pwd: </label>");
  resp.append("<input type=\"text\" id=\"otaPwd\" name=\"u_otaPwd\"></div>");
  resp.append("<div><BR><input type=\"submit\"></div></form>");

  resp.append("<BR><HR align=center size=1 width=\"75%\"><H2 align=center>Save to flash</H2><BR>");
  resp.append("<form method=\"POST\" action=\"saveToFlash\"> ");
  resp.append("<div>Check to confirm: <input type=\"checkbox\" name=\"u_confirmSave\"></div><div><BR><input type=\"submit\"></div></form></body></html>");
  
  webServer.send(200, "text/html", resp.c_str());
 }
