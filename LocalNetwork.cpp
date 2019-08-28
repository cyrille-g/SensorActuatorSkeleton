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




#include "PinAttribution.h"
#include "LocalNetwork.h"

#include "Parameters.h"
#include "settings.h"
#include <LogManagement.h>


LocalNetwork network;

LocalNetwork::LocalNetwork(void)
{
  //wifi not starting fix
  WiFi.mode(WIFI_OFF);
  _keepConfServer = false;
  _webServerStartedAt = 0;
  _lastFailedWifiAttemptAt = 0;
  _pWifiMulti = NULL;
  NtpSetup();
  MqttSetup();
  OtaSetup();
}

void LocalNetwork::begin(void)
{
  TryWifiConnect();
    
  WebServerSetup();
  _webServerStartedAt = millis();
    
  NTP.begin(parameters.NtpIp().c_str(), 1, true);
  ArduinoOTA.begin(); 
}


void LocalNetwork::ConfSoftApWebServer(void)
{
  WiFi.mode(WIFI_OFF);
  delay(50);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(parameters.ConfigurationSsid().c_str(),parameters.ConfigurationPwd().c_str());
  delay(100);
  
  IPAddress Ip(192, 168, 150, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  IPAddress myIP = WiFi.softAPIP();

  LOG("AP IP address: ");
  LOG_LN(myIP);
}

void LocalNetwork::WebServerRunning(void)
{
  _webServer.on("/", [this]() {
    if (_otaUpdating)
      return;
    HandleWebInterface(_webServer);
    _frontendReload="<html><head><meta http-equiv=\"refresh\" content=\"1;URL=";
    _frontendReload.append(_webServer.uri().c_str());
    _frontendReload.append("\"></head></html>");

  });
  _webServer.on("/command", [this]() {
    if (_otaUpdating)
      return;
    HandleCommmand();
    _webServer.send(200, "text/html", _frontendReload.c_str());
  });
  _webServer.on("/log", [this]() {
    if (_otaUpdating)
      return;
    HandleLog();
  });

  _webServer.on("/reset", [this]() {
    if (_otaUpdating)
      return;
    HandleReset();
  });

  _webServer.onNotFound([this]() {
    if (_otaUpdating)
      return;
    HandleNotFound();
  });

  _httpUpdater.setup(&_webServer);
  _webServer.begin(parameters.WebserverPort());

}

void LocalNetwork::WebServerSetup(void)
{
  _webServer.on("/setup", [this]() {
    _setupReload="<html><head><meta http-equiv=\"refresh\" content=\"1;URL=";
    _setupReload.append(_webServer.uri().c_str());
    _setupReload.append("\"></head></html>");

    _keepConfServer = true;
      if (_otaUpdating)
    return;
    parameters.HandleConfiguration(_webServer);
  });

  _webServer.on("/confWifi", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfWifi(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  _webServer.on("/confMqtt", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfMqtt(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  _webServer.on("/confNtp", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfNtp(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  _webServer.on("/confDevicename", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfDeviceName(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  _webServer.on("/confConf", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfConf(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  _webServer.on("/confOta", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleConfOta(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });


  _webServer.on("/saveToFlash", [this]() {
    if (_otaUpdating)
      return;
    parameters.HandleSaveToFlash(_webServer);
    _webServer.send(200, "text/html", _setupReload.c_str());
  });

  WebServerRunning();
}

void LocalNetwork::MqttSetup(void)
{
  _mqttClient.setClient(_espClient);
  LOG_LN("Wifi connected, reaching for MQTT server")
  _mqttWaitingCounter = MQTT_WAIT_RECONNECT_COUNTER;
  _mqttClient.setServer(parameters.MqttIp().c_str(), (uint16_t)parameters.MqttPort());
  _mqttClient.setCallback([this](char* topic, byte * payload, unsigned int length) {
    AirconCommandCallback(topic, payload, length);
  });
}

void LocalNetwork::OtaSetup(void)
{
  _otaUpdating = false;
  ArduinoOTA.setPort(parameters.OtaPort());
  ArduinoOTA.setHostname(parameters.DeviceName().c_str());

  // authentication by default
  ArduinoOTA.setPassword(parameters.OtaPwd().c_str());

  ArduinoOTA.onStart([this]() {
    LOG_LN("Starting OTA")
    SetOtaUpdating();
  });

  ArduinoOTA.onEnd([this]() {
    LOG_LN("\nEnd");
    ResetOtaUpdating();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    LOG_LN(progress)
  });

  ArduinoOTA.onError([](ota_error_t error) {
    LOG("Error ")
    LOG_LN(error)

    if (error == OTA_AUTH_ERROR)
      LOG_LN("Auth Failed")
      else if (error == OTA_BEGIN_ERROR)
        LOG_LN("Begin Failed")
        else if (error == OTA_CONNECT_ERROR)
          LOG_LN("Connect Failed")
          else if (error == OTA_RECEIVE_ERROR)
            LOG_LN("Receive Failed")
            else if (error == OTA_END_ERROR)
              LOG_LN("End Failed")

            });

}

void LocalNetwork::NtpSetup(void)
{
  _ntpEventTriggered = false;
  // NTP and time setup.
  // this one sets the NTP server as gConftimeServer with gmt+1
  // and summer/winter mechanic (true)
  // set the interval to 20s at first. On success, change that value
  NTP.setInterval(20);

  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    network.SetNtpEvent(event);
  });
}

bool LocalNetwork::TryWifiConnect(void) 
{
  if (_otaUpdating)
    return false;
    
  /* reconnect timer */
  if ((_lastFailedWifiAttemptAt!=0) && (_lastFailedWifiAttemptAt + WIFI_CONNECTION_TIMER_BETWEEN_TRIES_MS) > millis())
    return false;

  WiFi.mode(WIFI_OFF);
  delay(50);  
  LOG_LN("Attempting wifi connection")
  int i = 0;
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  _pWifiMulti = new ESP8266WiFiMulti;
  _pWifiMulti->addAP(parameters.MainSsid().c_str(), parameters.MainSsidPwd().c_str());
  _pWifiMulti->addAP(parameters.BackupSsid().c_str(), parameters.BackupSsidPwd().c_str());

  for(i = 0; (i<WIFI_CONNECTION_TRIES_BEFORE_ERROR) && (_pWifiMulti->run() != WL_CONNECTED);i++)
  {
    // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(50);
  }

  if (i<WIFI_CONNECTION_TRIES_BEFORE_ERROR)
  {
    LOG("Connected to ")
    LOG(WiFi.SSID().c_str())              // Tell us what network we're connected to
    LOG(" IP address: ")
    LOG_LN(WiFi.localIP())
  
  
    if (!MDNS.begin(parameters.DeviceName().c_str())) {   // Start the mDNS responder for DEVICENAME.local
      LOG_LN("Error setting up MDNS responder")
    }

    LOG("Device answers to ")
    LOG(parameters.DeviceName().c_str())
    LOG_LN(".local")

    return true;
  } else {

    delete _pWifiMulti;
    _lastFailedWifiAttemptAt = millis();
    _wifiSetupFailed++;
    LOG_LN("Failed to connect to wifi network, try again later, use softAP for now");
    ConfSoftApWebServer();
    return false;
  }
}

bool LocalNetwork::CheckWifi(void)
{
  bool ret = false;
  if (WiFi.status() != WL_CONNECTED)
  {
    if (TryWifiConnect()) 
    { /* we went from "unconnected" to "connected" */
      /* stop the webserver with the configuration, unless told not to */
      _webServer.close();
      /* restart the webserver, without the configuration tool */
      WebServerRunning();
    }
  }
  return ret;
}

void LocalNetwork::CheckOtaUpdate(void)
{
  ArduinoOTA.handle();

  /* we do not want to process anything in OTA mode, and we need some time to flash */
  if (_otaUpdating)
    delay(50);
}

void LocalNetwork::CheckAndProcessMqttEvents(void)
{
  if (_otaUpdating)
    return;

  if (_mqttClient.connected() )
  {
    if (_mqttWaitingCounter < MQTT_WAIT_RECONNECT_COUNTER)
    {
      _mqttWaitingCounter++;
    } else {
      _mqttWaitingCounter = 0;
      LOG_LN("try reconnecting MQTT")
      TryReconnectMqtt();
    }
  }

  if (_mqttClient.connected())
  {
    _mqttClient.loop();
  }
}

void LocalNetwork::ProcessWebServerEvents(void)
{
  if (_otaUpdating)
    return;

  _webServer.handleClient();
}

void LocalNetwork::ProcessNtpEvents(void)
{
  if (_otaUpdating)
    return;

  if (_ntpEventTriggered)
  {
    if (_ntpEvent != 0)
    {
      LOG("Time Sync error: ")
      if (_ntpEvent == noResponse) {
        LOG_LN("NTP server not reachable")
      } else if (_ntpEvent == invalidAddress) {
        LOG_LN("Invalid NTP server address")
      }
    } else {
    LOG("Setting time to  ")
    
    LOG(NTP.getTimeDateString(NTP.getLastNTPSync()))
    LOG_LN(" and interval to 10mins / 1day")
    // set the system to do a sync every 86400 seconds when succesful, so once a day
    // if not successful, try syncing every 600s, so every 10 mins
    NTP.setInterval(600, 86400);

    }
  _ntpEventTriggered = false;
  }
}

void LocalNetwork::TryReconnectMqtt(void)
{
  LOG("Attempting MQTT connection...")
  // Attempt to connect
  if (_mqttClient.connect(parameters.DeviceName().c_str(), parameters.MqttUsername().c_str(),parameters.MqttPwd().c_str()))
  {
    LOG_LN(" connected")

    /* subscribe to the set topics */
    allSensorsAndActuators.SubscribeToMqttSetTopic(_mqttClient);
    /* send our current state */
    allSensorsAndActuators.PublishMqttState(_mqttClient);

  } else {
    LOG(" failed, rc=");
    LOG_LN(_mqttClient.state());
  }
}

void LocalNetwork::AirconCommandCallback(char* topic, byte* payload, unsigned int length)
{
  LOG("Message arrived [")
  LOG(topic)
  LOG_LN("] ")

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  LOG_LN(message)

  /* first, check topic validity. If it is MQTT_PREFIX /room/MQTT_POWER, then go forward */
  char *token = strtok(topic, "/");

  // does it start with MQTT_PREFIX ?
  if (token == NULL )
  {
    return;
  }

  if (strcmp(token, parameters.MqttPrefix().c_str()) != 0) {
    //no; leave
    LOG_LN("wrong MQTT prefix")
    return;
  }
  /* this is for us. Send it to actuators */
  allSensorsAndActuators.ProcessMqttRequest(topic, message);

}



void LocalNetwork::SendStandardWebInterface(ESP8266WebServer &webServer)
{
  /* Send the command to read sensors */
  allSensorsAndActuators.UpdateAllSensors();

  string resp("<HTML><head><title>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" frontend </title>");
  Parameters::AppendCss(resp);
  resp.append("<div class=SAparagraph><H1><center>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" frontend</H1></center></div><BR><BR>");

  char localConvertBuffer[10] = {0};
  resp.append("<table>");
  allSensorsAndActuators.AppendWebData(resp);
  resp.append("</table></body></html>");
  webServer.send(200, "text/html", resp.c_str());
}


/* -------------------------------- WEBSERVER HANDLERS ------------------------*/

void LocalNetwork::HandleWebInterface(ESP8266WebServer &webServer)
{
  if (_otaUpdating)
    return;

  SendStandardWebInterface(webServer);
}

void LocalNetwork::HandleCommmand(void)
{
  allSensorsAndActuators.ProcessWebRequest(&_webServer);
}

void LocalNetwork::HandleNotFound(void)
{
  if (_otaUpdating)
    return;
  _webServer.send(404, "text/plain", "404 File Not Found");
}

void LocalNetwork::HandleReset(void)
{
  if (_otaUpdating)
    return;

  _webServer.send(200, "text/plain", "Resetting in 1s");
  delay(1000);
  ESP.reset();
}

void LocalNetwork::HandleLog(void)
{
  if (_otaUpdating)
    return;

  string resp("");
  resp.append("<html><head><title>");
  resp.append(parameters.DeviceName().c_str());
  resp.append("</title></head><body><h1>");
  resp.append(parameters.DeviceName().c_str());
  resp.append(" logs</h1>") ;
#ifdef NEED_QUEUE_LOG
  resp.append(Logger.GetAllLogsWithBrSeparator());
#else
  resp.append("log queue disabled<BR><BR>");
#endif
  resp.append("</body></html>");
  _webServer.send(200, "text/html", resp.c_str());
}
