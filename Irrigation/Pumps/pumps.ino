#include <ESP8266WiFi.h>

#// WiFi connection
const char* ssid = YOURSSID;
const char* password = YOURPASSWORD;
const char* id = "1"; // for operating a number of sensors.
char server[] = YOURSERVERADRESS;

const int pincount = 3; // No of relays
int pins[] = {5,4,12}; // Pins for relays
const unsigned long postingInterval = 3600L * 1000000L; // Sleepinterval
int port = 80;
WiFiClient client;

//DHT 
#include <DHT.h>
#define DHTPIN 13
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE,16);

// needed to avoid link error on ram check
extern "C" 
{
#include "user_interface.h"
}

void WiFiStart()
{ 
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
}

// Retrieve setpoint for a certain sensorid
int getSetpointForID(int id) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  delay(100);
  // We now create a URL for the request
  String url = "/GetSetpointv2.php?";
  url += "id=";
  url += id;
  
  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.println("connecting...");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" + 
               "Connection: close\r\n\r\n");
    delay(30);
  
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
  String s="";
  // Werte zurücklesen

  while (client.available()) {
    char c = client.read();
    //debug
    //Serial.print(c);
    s += String(c);
  }
  String startstr = "OUT:";
  int startpos=s.indexOf(startstr)+startstr.length();
  String retval = s.substring(startpos);
  return retval.toInt();
}

// create a logging request
void httpRequest(float w, int sid) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  delay(100);
  // We now create a URI for the request
  String url = "/newrecord.php?";
  url += "water=";
  url += String(w,2);
  url += "&id=";
  url += sid;
  
  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.println("connecting...");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" + 
               "Connection: close\r\n\r\n");
    Serial.println("data submitted.");
    delay(10);
  
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void httpRequestTempHum(float t, float h) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  delay(100);
  // We now create a URI for the request
  String url = "/newrecord.php?";
  url += "temp=";
  url += String(t,2);
  url += "&humidity=";
  url += String(h,2);
  url += "&id=99";
  
  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.println("connecting...");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" + 
               "Connection: close\r\n\r\n");
    Serial.println("data submitted.");
    delay(10);
  
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}




void setup() {
  // Activate DHT
  dht.begin();
 
  // Init Outputs
  for (int i=0;i<pincount;i++)
  {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }
  
  // start serial
  Serial.begin(115200);
  Serial.println("WLAN Pumpcontrol - NRNT");

  // Connect to WIFI
  WiFi.mode(WIFI_STA);
  WiFiStart();

  // debug
  while (client.available()) {
    char c = client.read();
    //Serial.write(c);
  }

  // First Temp& humidity easuring
   delay(1000);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  httpRequestTempHum(temp,hum);
}
// LOOP
void loop() {
  for (int i=0;i<pincount;i++)
  {
    int dur=getSetpointForID(i+1); //id starts with 1, index with 0
    Serial.println("Starting pump "+String(i+1)+" for "+String(dur)+" secs");
    digitalWrite(pins[i], LOW);  
    delay(dur*1000);
    digitalWrite(pins[i], HIGH);
    httpRequest(dur, i+1);
    delay(1000);
  }

  // Second Temp& Humidity Mesuring
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  httpRequestTempHum(temp,hum);
  Serial.println("Good Night");
  ESP.deepSleep(postingInterval, WAKE_NO_RFCAL); //WAKE_DEFAULT, WAKE_RFCAL, WAKE_NO_RFCAL, WAKE_RF_DISABLED.
  delay(1000);
}



