/*
 ESP8266 IO HTTP
 
 This program reads on digital input of the ESP8266 and displays
 the result on a webpage. The ESP8266 connects directly to a
 defined wifi on bootup.

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "user_interface.h" // Needed for timer

// Define Pin GPIO2 as input pin
#define INPUT_PIN 2

// Define time function for periodic read in of digital input
os_timer_t myTimer;
#define TIMER_UPDATE 1000    // reading cycle [milliseconds]

// Define array to hold a specif number of readings of the digital
// input. It is used as round robin array, pushing a new reading to
// the beginning of the array and deleting the last/oldest reading.
#define N_READINGS 60        // length of array
#define READINGS_DEFAULT 1   // default value for initialization
bool readings[N_READINGS];

// WiFi parameter
const char* ssid     = "Freifunk";
const char* password = "";

// Webserver parameter
ESP8266WebServer server(80);
String webpage_head, webpage_trail;

// Start MDNS
// If MDNS is supported by client, the ESP8266 can be accessed via
// address "http://[mdns_name].local"
MDNSResponder mdns;
const char* mdns_name = "esp8266";

// Setup routine
void setup() {

  // Configure pin INPUT_PIN as input
  pinMode(INPUT_PIN, INPUT);

  // Initialize array for readings
  for (int i = 0; i < N_READINGS; i++) {
    readings[i] = READINGS_DEFAULT;
  }

  // Configure serial port
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi
  connect_wifi();
  
  // Activate MDNS responder
  if (mdns.begin(mdns_name, WiFi.localIP())) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
  }
  
  // Start webserver
  start_httpd();
  
  // Configure timer
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, TIMER_UPDATE, true);
}

// the loop function runs over and over again forever
void loop() {
  server.handleClient();
  
  // It seems, that the ESP8266 may lose Wifi when running
  // for a longer time. In this case, try to reconnect
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost WiFi connection. Reconnecting");
    connect_wifi();
  }
}

// =======================================================

// Add reading to array of readings
void addToReadings(bool *readings, bool reading) {
  // First, move all elements on position up and drop last
  // (oldest) element
  for (int i = N_READINGS-1; i >= 1; i--) {
    readings[i] = readings[i - 1];
  }
  // Second, add reading to the first position of array
  readings[0] = reading;
  
  // Output array
  Serial.print("(");
  for (int i = 0; i < N_READINGS; i++) {
    Serial.print(readings[i]);
  }
  Serial.println(")");
}

// Count all elements in array of reading that are not
// equal READINGS_DEFAULT
int countInReadings(bool *readings) {
  int count = 0;
  for (int i = 0; i< N_READINGS; i++) {
    if (readings[i] != READINGS_DEFAULT) {
      count++;
    }
  }
  return count;
}

// Timer-controlled read in of digital input
void timerCallback(void *pArg) {
  bool inp=digitalRead(INPUT_PIN);
  Serial.print("Read digital input: ");
  Serial.println(inp);
  addToReadings(&readings[0], inp);
}

// Conecting to WiFi network
void connect_wifi() {
  WiFi.disconnect();
  
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ");

  // Explicitly set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
}

// Setup the webserver
void start_httpd() {
  webpage_head+="<html><head>";
  webpage_head+="<title>ESP8266 Webserver</title>";
  webpage_head+="<meta http-equiv=\"refresh\" content=\"5\" >";
  webpage_head+="</head><body>";
  webpage_head+="<h1>ESP8266 Webserver</h1>";
  webpage_head+="<p>Welcome to the website of ESP8266!</p>";
  webpage_head+="<p>The following value is a counter for ";
  webpage_head+="&glqq;on&glqq;-Readings of the digital input pin ";
  webpage_head+="in a round robin array.</p>";
  webpage_head+="<p>Value: ";

  webpage_trail+="</p>";  
  webpage_trail+="</body></html>";

  server.on("/", [](){
    Serial.println("HTTP request");
    server.send(200,
                "text/html",
                webpage_head
                + countInReadings(&readings[0])
                + webpage_trail
               );
  });
  server.begin();
  Serial.println("HTTP server started"); 
}
