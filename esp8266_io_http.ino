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

#define INPUT_PIN 2  // Define Pin GPIO2 as input pin

// Define time function for periodic read in of digital input
os_timer_t myTimer;
#define TIMER_UPDATE 1000

// Bolean variable for digital input reads
bool inp = 0;

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

  pinMode(INPUT_PIN, INPUT); // Configure pin INPUT_PIN as input

  Serial.begin(9600);        // Configure serial port
  delay(10);

  connect_wifi();            // Connect to WiFi
  
  // Activate MDNS responder
  if (mdns.begin(mdns_name, WiFi.localIP())) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
  }
  
  start_httpd();             // Start webserver
  
  // Configure timer
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, TIMER_UPDATE, true);
}

// the loop function runs over and over again forever
void loop() {
  server.handleClient();
}

// Timer-controlled read in of digital input
void timerCallback(void *pArg) {
  inp=digitalRead(INPUT_PIN);
  Serial.print("Read digital input: ");
  Serial.println(inp);
}

// Conecting to WiFi network
void connect_wifi() {
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ");

  WiFi.mode(WIFI_STA); // Explicitly set the ESP8266 to be a WiFi-client
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
  webpage_head+="<p>Digital Input: ";

  webpage_trail+="</p>";  
  webpage_trail+="</body></html>";

  server.on("/", [](){
    Serial.println("HTTP request");
    server.send(200, "text/html", webpage_head+inp+webpage_trail);
  });
  server.begin();
  Serial.println("HTTP server started"); 
}
