#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "PubSubClient.h"

#include <U8g2lib.h>
#include "4x6_t_german.h"
#include "5x8_t_german.h"
#include "6x13B_t_german.h"
#include "settings.h"
#include "secrets.h"

// declare telnet server (do NOT put in setup())
WiFiServer TelnetServer(port);
WiFiClient Telnet;
WiFiClient MQTTClient;
PubSubClient MQTTclient(MQTTClient);

// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2_b(U8G2_R2, U8X8_PIN_NONE);  //ArduinoMega with HW I2C is used (PIN21/22)
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C trainDisplay(U8G2_R2, /* clock=*/SCL1, /* data=*/SDA1, /* reset=*/U8X8_PIN_NONE); // clock
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C clockDisplay(U8G2_R2, /* clock=*/SCL2, /* data=*/SDA2, /* reset=*/U8X8_PIN_NONE); // platform

const int clockX = 16; // center the analog clock
const int clockY = 16; // center the analog clock

int iHour = 10;
int iMinute = 10;

String commandStr;

void handleTelnet()
{
  if (TelnetServer.hasClient())
  {
    // client is connected
    if (!Telnet || !Telnet.connected())
    {
      if (Telnet)
        Telnet.stop();                   // client disconnected
      Telnet = TelnetServer.available(); // ready for new client
    }
    else
    {
      TelnetServer.available().stop(); // have client, block new conections
    }
  }

  if (Telnet && Telnet.connected() && Telnet.available())
  {
    // client input processing
    while (Telnet.available())
      Serial.write(Telnet.read()); // pass through
                                   // do other stuff with client input here
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{

  String sTopic = topic;
  String sPayload = ""; //(char *)payload;

  for (int i = 0; i < length; i++)
  {
    sPayload = sPayload + (char)payload[i];
  }

  Serial.print("MQTT Message: [");
  Serial.print(sTopic);
  Serial.print("] ");
  Serial.println(sPayload);

  Telnet.print("MQTT Message: [");
  Telnet.print(sTopic);
  Telnet.print("] ");
  Telnet.println(sPayload);

  /*
    if (sTopic == hostname + "/" + "MeasureLoopCount")
    {
      iMeasureEachLoop = sPayload.toInt();
      iLoopCount = sPayload.toInt();
    }
  */
}

// draw analog clock
void processClock(int iHour, int iMinute)
{
  clockDisplay.firstPage();
  do
  {
    clockDisplay.drawCircle(clockX, clockY, 15);

    // hour ticks
    for (int z = 0; z < 360; z = z + 30)
    {
      // Begin at 0° and stop at 360°
      float angle = z;
      angle = (angle / 57.29577951); // Convert degrees to radians
      int x2 = (clockX + (sin(angle) * 15));
      int y2 = (clockY - (cos(angle) * 15));
      int x3 = (clockX + (sin(angle) * (15 - 4)));
      int y3 = (clockY - (cos(angle) * (15 - 4)));
      clockDisplay.drawLine(x2, y2, x3, y3);
    }

    // display minute hand
    float angle = iMinute * 6;
    angle = (angle / 57.29577951); // Convert degrees to radians
    int x3 = (clockX + (sin(angle) * (15 - 3)));
    int y3 = (clockY - (cos(angle) * (15 - 3)));
    clockDisplay.drawLine(clockX, clockY, x3, y3);

    // display hour hand
    angle = iHour * 30 + int((iMinute / 12) * 6);
    angle = (angle / 57.29577951); // Convert degrees to radians
    x3 = (clockX + (sin(angle) * (20 - 11)));
    y3 = (clockY - (cos(angle) * (20 - 11)));
    clockDisplay.drawLine(clockX, clockY, x3, y3);
  } while (clockDisplay.nextPage());
}

void processTrain(String cmdString)
{
  // since german umlauts are always nasty in different OS the sketch expect to receive their representive eg. "ae" instead of "ä"
  // here we convert it to octal code for proper display
  cmdString.replace("ae", "\344"); // replace ä
  cmdString.replace("oe", "\366"); // replace ö
  cmdString.replace("ue", "\374"); // replace ü
  cmdString.replace("Ae", "\304"); // replace Ä
  cmdString.replace("Oe", "\326"); // replace Ö
  cmdString.replace("Ue", "\334"); // repalce Ü
  cmdString.replace("s?", "\337"); // replace ß

  // parse the incoming text and split the payload in lines
  int L1 = cmdString.indexOf("-L1");
  int L2 = cmdString.indexOf("-L2");
  int L3 = cmdString.indexOf("-L3");
  int L4 = cmdString.indexOf("-L4");
  String sLine1 = cmdString.substring(L1 + 4, L2);
  String sLine2 = cmdString.substring(L2 + 4, L3);
  String sLine3 = cmdString.substring(L3 + 4, L4);
  String sLine4 = cmdString.substring(L4 + 4);

  // convert the string to array of char which is expected by the u8g2 library
  char arcLine1[14];
  sLine1.toCharArray(arcLine1, 14);
  char arcLine2[18];
  sLine2.toCharArray(arcLine2, 18);
  char arcLine3[25];
  sLine3.toCharArray(arcLine3, 25);
  char arcLine4[6];
  sLine4.toCharArray(arcLine4, 6);

  // now send the data to the display
  trainDisplay.firstPage();
  do
  {
    // 1st row: Name of the Train
    trainDisplay.setFont(u8g2_font_6x13B_t_german);
    trainDisplay.drawStr(0, 9, arcLine1);
    // 2nd row: Target of the train
    trainDisplay.setFont(u8g2_font_5x8_t_german);
    trainDisplay.drawStr(0, 20, arcLine2);
    // 3rd row: 2nd Target
    trainDisplay.setFont(u8g2_font_5x8_t_german);
    trainDisplay.drawStr(0, 32, arcLine3);
    // ab
    trainDisplay.setFont(u8g2_font_4x6_t_german);
    trainDisplay.drawStr(100, 10, Line5Left);
    trainDisplay.setFont(u8g2_font_5x8_t_german);
    trainDisplay.drawStr(110, 10, Line5Right);
    // Time of departure
    trainDisplay.setFont(u8g2_font_5x8_t_german);
    trainDisplay.drawStr(100, 22, arcLine4);
  } while (trainDisplay.nextPage());
}

void setup()
{

  // init OLED
  clockDisplay.begin();
  clockDisplay.setContrast(150);
  clockDisplay.clearDisplay();
  clockDisplay.setDisplayRotation(U8G2_R1);

  trainDisplay.begin();
  trainDisplay.setContrast(150);
  trainDisplay.clearDisplay();
  Line5Left = "";
  Line5Right = "";

  delay(1000);

  // connect to WiFi
  Serial.printf("Connecting to %s ", wifi_ssid);
  processTrain("-L1 -L2 Conneccting to -L3 " + String(wifi_ssid) + "-L4 ");  // send status to PlatformDisplay
  delay(2000);
  
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname.c_str()); // define hostname
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    processTrain("-L1 -L2 WIFI Connecion failed !!! -L3 -L4 ");  // send status to PlatformDisplay
    delay(5000);
    ESP.restart();
  }
  
  processTrain("-L1 Connected to -L2 " + String(wifi_ssid) + "-L3 " + WiFi.localIP().toString() + "-L4 ");  // send status to PlatformDisplay
  delay(5000);
  
  Serial.println();
 
  // OTA Stuff
  ArduinoOTA.setPort(iOTAPort);
  ArduinoOTA.setHostname(hostname.c_str());

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       Serial.println("Start updating " + type); });

  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         Serial.println("Auth Failed");
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         Serial.println("Begin Failed");
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         Serial.println("Connect Failed");
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         Serial.println("Receive Failed");
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         Serial.println("End Failed");
                       } });

  Serial.println("Starting OTA");
  ArduinoOTA.begin();

  // TELNET Stuff

  TelnetServer.begin();
  Serial.print("Starting telnet server on port " + (String)port);
  TelnetServer.setNoDelay(true); // ESP BUG ?
  Serial.println();
  delay(100);

  // MQTT Stuff
  MQTTclient.setServer(MQTTBroker, MQTTPort);
  MQTTclient.setCallback(callback);

// Setup done, now print show default dispaly
  Line5Left = "|->";
  Line5Right = " ab";
  processTrain(DefaultPlatformString);
  processClock(10, 10); // init Clock with 10:10

  // Init Serial
  Serial.begin(115200);
}

void loop()
{
  String cmdString;
  ArduinoOTA.handle();
  handleTelnet();

  // manage MQTT 
  while (!MQTTclient.connected())
  {
    Telnet.println("Connect to MQTT Broker");
    Serial.println("Connect to MQTT Broker");
    MQTTclient.connect(hostname.c_str());
    MQTTclient.subscribe("ESP_MobaDisplay/test");
    delay(5000);
  }

  delay(1000);

  MQTTclient.loop();

  while (Serial.available() > 0)
  {
    cmdString = Serial.readString();
  }

  Telnet. 

  String cmdType = cmdString.substring(0, 5); // expect either TrainX or ClockX
  Serial.println("Command: " + cmdString);    // Print received command to serial monitor
  // Serial.println("CommandType: " + cmdType);
  delay(100);

  if (cmdType == "Clock")
  {
    if (cmdString.charAt(7) == 'T')
    {
      int iDDot = cmdString.indexOf(":");
      iHour = cmdString.substring(iDDot - 2, iDDot).toInt();
      iMinute = cmdString.substring(iDDot + 1).toInt();
      //      Serial.println("Hour: " + iHour);
      //      Serial.println("Minute: " + iMinute);
    }

    if (cmdString.charAt(7) == 'S')
    {
      iMinute = iMinute + 1;
      if (iMinute == 59)
      {
        iMinute = 0;
        iHour = iHour + 1;
      }
    }
    processClock(iHour, iMinute);
  }

  if (cmdType == "Train")
  {
    if (cmdString != "")
    {
      processTrain(cmdString); // Process the received command
      cmdString = "";          // Clear variable to receive next command
    }
  }
}
