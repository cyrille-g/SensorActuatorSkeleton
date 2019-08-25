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

#include <LogManagement.h>

#include "PinAttribution.h"
#include "LocalNetwork.h"
#include "settings.h"

LocalNetwork network;

LocalNetwork::LocalNetwork(void)
{
  //wifi not starting fix
  WiFi.mode(WIFI_OFF);
  WifiSetup();
  NtpSetup();
  WebServerSetup();
  MqttSetup();
  OtaSetup();
}


void LocalNetwork::WebServerSetup(void)
{
  _webServer.on("/",[this]() {handleRoot();});
  _webServer.on("/command",[this]() {handleCommmand();});
  _webServer.on("/log", [this]() {handleLog();});
  _webServer.onNotFound([this]() {handleNotFound();});

  _httpUpdater.setup(&_webServer);
  _webServer.begin(WEBSERVER_PORT);
} 
 
void LocalNetwork::MqttSetup(void)
{  
  _mqttClient.setClient(_espClient);
  LOG_LN("Wifi connected, reaching for MQTT server")
  _mqttWaitingCounter = MQTT_WAIT_RECONNECT_COUNTER;
  _mqttClient.setServer(MQTT_SERVER_IP, (uint16_t)MQTT_PORT);
  _mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) { AirconCommandCallback(topic, payload, length);});
}

void LocalNetwork::OtaSetup(void)
{
  _otaUpdating = false;
  ArduinoOTA.setPort(OTAPORT);
  ArduinoOTA.setHostname(DEVICENAME);

  // authentication by default
  ArduinoOTA.setPassword(OTAPASSWORD);

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

  ArduinoOTA.begin();
}

void LocalNetwork::NtpSetup(void)
{
  _ntpEventTriggered = false;
  // NTP and time setup.
  // this one sets the NTP server as gConftimeServer with gmt+1
  // and summer/winter mechanic (true)
  NTP.begin(NTP_SERVER, 1, true);
  // set the interval to 20s at first. On success, change that value 
  NTP.setInterval(20);

  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
      network.SetNtpEvent(event);
      });
}

void LocalNetwork::WifiSetup(void) {

   _wifiMulti.addAP(WIFISSID1, WIFIPWD1);
   _wifiMulti.addAP(WIFISSID2 ,WIFIPWD2);
  // We start by connecting to a WiFi network
   while (_wifiMulti.run() != WL_CONNECTED)
   { 
    // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(50);
   }  

  LOG("Connected to ");
  LOG(WiFi.SSID());              // Tell us what network we're connected to
  LOG(" IP address:\t");
  LOG_LN(WiFi.localIP()); 


  if (!MDNS.begin(DEVICENAME)) {   // Start the mDNS responder for DEVICENAME.local
    LOG_LN("Error setting up MDNS responder")
  }
  
  LOG("Device answers to ")
  LOG(DEVICENAME)
  LOG_LN(".local")
  
}


void LocalNetwork::CheckWifi(void)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(5);
    LOG_LN("WIFI Disconnected. Attempting reconnection")
    WifiSetup();
    //led off once setup is done
    digitalWrite(BUILTIN_LED,HIGH);
    return;
  }
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
      _mqttWaitingCounter=0;
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


   if (_ntpEventTriggered &&_ntpEvent)
   {
    LOG("Time Sync error: ")
        if (_ntpEvent == noResponse) {
          LOG_LN("NTP server not reachable")
        } else if (_ntpEvent == invalidAddress) {
          LOG_LN("Invalid NTP server address")
        }
   } else {
      LOG("Setting time to  ")
      LOG(NTP.getTimeDateString(NTP.getLastNTPSync()).c_str())
      LOG_LN(" and interval to 10mins / 1day")
      // set the system to do a sync every 86400 seconds when succesful, so once a day
      // if not successful, try syncing every 600s, so every 10 mins
      NTP.setInterval(600, 86400);

   }
  _ntpEventTriggered = false;
}


void LocalNetwork::TryReconnectMqtt(void)
{
  LOG("Attempting MQTT connection...")
  // Attempt to connect
  if (_mqttClient.connect(DEVICENAME, MQTT_USERNAME, MQTT_PWD)) 
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

  if (strcmp(token, MQTT_PREFIX) != 0) {
    //no; leave
    LOG_LN("wrong MQTT prefix")
    return;
  }
  /* this is for us. Send it to actuators */
  allSensorsAndActuators.ProcessMqttRequest(topic, message);
 
}



/* -------------------------------- WEBSERVER HANDLERS ------------------------*/

void LocalNetwork::handleRoot(void)
{
  if (_otaUpdating)
  return;
  
  /* Send the command to read sensors */
  allSensorsAndActuators.UpdateAllSensors();
  
  char localConvertBuffer[10]= {0};
  string resp("<head><title>");
  resp.append(DEVICENAME);
  resp.append("</title></head><body><H1>");
  resp.append(DEVICENAME);
  resp.append("</H1/<BR><BR>");
  
  std::string *pWebData = allSensorsAndActuators.GenerateWebData();
  resp.append(*pWebData);
  delete pWebData;
  
  resp.append("</body></html>");
  _webServer.send(200, "text/html", resp.c_str());
}

void LocalNetwork::handleCommmand(void)
{
  
  allSensorsAndActuators.ProcessWebRequest(&_webServer);
  handleRoot();
}

void LocalNetwork::handleNotFound(void) 
{
  if (_otaUpdating)
    return;
  _webServer.send(404, "text/plain", "404 File Not Found");
}

void LocalNetwork::handleReset(void)
{
  if (_otaUpdating)
    return;
    
  _webServer.send(200, "text/plain", "Resetting in 1s");
  delay(1000);
  ESP.reset();
}

void LocalNetwork::handleLog(void) 
{
  if (_otaUpdating)
    return;
    
  String resp("");

  resp += "<html>" \
          "<head><title>" ;
  resp += DEVICENAME;
  resp += "</title></head><body><h1>";
  resp += DEVICENAME;
  resp += " logs</h1>" ;
#ifdef NEED_QUEUE_LOG
  resp += Logger.getAllLogs();
#else
  resp += "log queue disabled<BR><BR>";
#endif
  resp += "</body></html>";
  _webServer.send(200, "text/html", resp);
}
