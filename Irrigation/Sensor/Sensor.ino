#include <ESP8266WiFi.h>
#include "Logger.h"

// Der ESP unterstützt die std nicht vollständig, deswegen
#define espmin(X, Y) (((X)<(Y))?(X):(Y))

// WiFi connection
const char* ssid = YOURSSID;
const char* password = YOURPASSWORD;
const char* id = "3"; // set different ids when running multipled sensors
char server[] = YOURSERVERADRESS;
int port = 80;
WiFiClient client;
const unsigned long postingInterval = 1800L * 1000000L; // delay between updates, in microseconds


#include <DHT.h>
#define DHTPIN 13
#define DHTTYPE DHT11   // DHT 11 

DHT dht(DHTPIN, DHTTYPE,16);

// needed to avoid link error on ram check
extern "C" 
{
#include "user_interface.h"
}
// Messung
value_type domeasuring() {
  //digitalWrite(4, LOW);
  //delay(300);
  value_type retval;
  retval.hum = dht.readHumidity();
  if (isnan(retval.hum))
    retval.hum=999;
  
  retval.temp = dht.readTemperature();
  if (isnan(retval.temp))
    retval.temp=999;
    
  retval.soil = analogRead(A0);
  //digitalWrite(4, HIGH);
  Serial.println("Humidity:"+String(retval.hum,2)+"\tTemperature:"+String(retval.temp,2)+"\tSoil:"+String(retval.soil,2));
  return retval;
}

void httpRequest(float h, float t, float s) {
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
  url += "&soil=";
  url += String(s,2);
  url += "&id=";
  url += id;
  
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

void setup() {
  value_type values[3];
  // start serial
  pinMode(4, OUTPUT);
  pinMode(A0, INPUT);
  dht.begin();

  Serial.begin(115200);
  Serial.println("WLAN Temperatur und Feuchtigkeitslogger - NRNT");
  digitalWrite(4, LOW);
  delay(1200);
  // Erste Messung
  values[0]= domeasuring();
  delay(1200);
  // Zweite Messung
  values[1]= domeasuring();
  delay(1200);
  // Dritte Messung
  values[2]= domeasuring();
  
  // Mittleren Wert finden
  float temp;
  float hum;
  float minmin = espmin(espmin(values[0].temp, values[1].temp), values[2].temp);
  if (values[0].temp = minmin)
  {
    temp=espmin(values[1].temp, values[2].temp);
    hum=espmin(values[1].hum, values[2].hum);
  }
  else if (values[1].temp = minmin)
  {
    temp=espmin(values[0].temp, values[2].temp);
    hum=espmin(values[0].hum, values[2].hum);
  }
  else
  {
    temp=espmin(values[0].temp, values[1].temp);
    hum=espmin(values[0].hum, values[1].hum);
  }
  float soil=(values[0].soil+values[1].soil+values[1].soil)/3.0;

  // Daten senden

  // Connect to WIFI
  WiFi.mode(WIFI_STA);
  WiFiStart();

  // debug
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  
  Serial.println("Humidity:"+String(hum,2)+"\tTemperature:"+String(temp,2)+"\tSoil:"+String(soil,2));
  httpRequest(hum,temp,soil);
  Serial.println("Good Night");
  ESP.deepSleep(postingInterval, WAKE_NO_RFCAL); //WAKE_DEFAULT, WAKE_RFCAL, WAKE_NO_RFCAL, WAKE_RF_DISABLED.
  delay(1000);
}

void loop() {
 // do nothing
}
