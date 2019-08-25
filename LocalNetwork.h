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

#ifndef _CGE_LOCALNETWORK_H
#define _CGE_LOCALNETWORK_H


#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>

#include <ArduinoOTA.h>
#include <NtpClientLib.h>


class LocalNetwork {

  public:
  LocalNetwork();
  ESP8266WebServer GetWebServer(void);
  PubSubClient GetMqttClient(void);

  void SetOtaUpdating(void) { _otaUpdating = true;}
  void ResetOtaUpdating(void) {_otaUpdating = false;}

  void SetNtpEvent(NTPSyncEvent_t ntpEvent) { _ntpEvent = ntpEvent;  _ntpEventTriggered = true;}
   
  void CheckWifi(void);
  void CheckOtaUpdate(void);
  void CheckAndProcessMqttEvents(void);
  void ProcessWebServerEvents(void);
  void ProcessNtpEvents(void);

  private:
  ESP8266WiFiMulti _wifiMulti; 
  uint16_t _mqttWaitingCounter;
  ESP8266WebServer _webServer;
  ESP8266HTTPUpdateServer _httpUpdater; // web interface to remotely update the firmware
  WiFiClient   _espClient;
  PubSubClient _mqttClient;

  bool _otaUpdating;
  bool _ntpEventTriggered;
  
  NTPSyncEvent_t _ntpEvent; // Last triggered event
  
  void WebServerSetup(void);
  void MqttSetup(void);
  void TryReconnectMqtt(void);  
  void WifiSetup(void);
  void OtaSetup(void);
  void NtpSetup(void);
  
  void handleLog(void) ;
  void handleReset (void);
  void handleNotFound(void) ;
  void handleCommmand(void);
  void handleRoot(void);

  void AirconCommandCallback(char* topic, byte* payload, unsigned int length) ;

};

extern LocalNetwork network;

#endif
