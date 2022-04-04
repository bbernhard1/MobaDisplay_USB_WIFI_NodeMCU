

// Wifi Settings
String hostname = "ESP_MobaDisplay";

// Telnet Settings
int port = 23;

// MQTT Settings
const char* MQTTBroker = "10.0.0.1"; 
const int MQTTPort = 1883;  // IPS MQTT Port

// OTA Settings
int iOTAPort = 8266;    // ESP32: 3232, ESP8266: 8266

// I/O for First Clock Display
const int SCL1 = 5;
const int SDA1 = 4;

// I/O for First Platform Display
const int SCL2 = 2;
const int SDA2 = 0;

// Configure Platform Display Line 5 
char* Line5Left = "|->";
char* Line5Right = " ab"; 

// Default Platform Display after powerup
const char* DefaultPlatformString = "-L1 -L2 Nicht einsteigen -L3 -L4";

// const char* DefaultPlatformString = "-L1 iTrain -L2 Platform Display -L3 -L4 by BB";
// const char* Line5Left = "(c)2021";
// const char* Line5Right = ""; 

