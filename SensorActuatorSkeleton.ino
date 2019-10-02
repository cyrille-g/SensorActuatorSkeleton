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

#include "LocalNetwork.h"
#include "PinAttribution.h"
#include "parameters.h"

void setup(void) 
{
  // serial
  Serial.begin(115200);
  parameters.begin();
  network.begin();

  //search sensors and actuators 
  allocatedPins.begin();
  
  //led off once setup is done
  digitalWrite(BUILTIN_LED,HIGH);

}

/********************************** START MAIN LOOP*****************************************/
void loop() {

  /* Every function called here should be defined in the LocalNetwork class.
   *  The reason for that is the OTA update; when it happens, no other 
   *  function should run to avoid problems.
   *  To that end, a private boolean member in LocalNetwork is set.
   *  Every function has to exit immediately when that member is set.
   *  
   *  the esp8266 specific code is mostly written in the LocalNetwork class, 
   *  so this is the class to change to make it work on an arduino or any other MCU
   */

  
  if (network.CheckWifi())
  {
    network.CheckOtaUpdate();
    network.CheckAndProcessMqttEvents();
    network.ProcessNtpEvents();
  }

  network.ProcessWebServerEvents();
  
}
