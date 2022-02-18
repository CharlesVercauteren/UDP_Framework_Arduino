// UDP_framework
//
// ©2022 by Charles Vercauteren 
// 10 januari 2022
//
// Getest op Arduino Uno Wifi.
//
// Applicatie:
// -----------
// UDP framework dat dient als basis voor Wifi communicatie met andere systemen
//
// Om te kunnen testen zonder client kan een UDP-datagram vanop de 
// commanodlijn naar de Arduino worden gestuurd met onderstaand
// commando (macOS en Linux):
//
//   echo "Test" > /dev/udp/10.89.1.90/2000.
// 
// Hierbij is 10.89.1.90 het ip-adres van de Arduino en 2000 de te gebruiken
// We kunnen ook gebruik maken van "Netcat" op macOS, dan kunnen we zenden en ontvangen:
//
//   nc -u 10.89.1.90 2000
//
// UDP-poort. Beiden worden gedefiniëerd in "secrets.h".
// Wensen we te werken met DHCP ipv. een vast ip-adres maak DHCP_MODE in "secrets.h" dan "true".
// De sketch zal de ontvangen string terugsturen.
//
// De ssid en het wachtwoord voor de Wifi worden gedefiniëerd in het bestand "secrets.h"
// Ook het poort nummer dat we wensen te gebruiken voor UDP staat in dit bestand.

// Includes en variabelen
//-----------------------
// Wifi
#include "secrets.h"
#include <WiFiNINA.h>

String ssid = SECRET_SSID;           // Wifi SSID in secrets.h
String password = SECRET_PASS;       // Wifi wachtwoord (WPA or WEP key) in secrets.h
int status = WL_IDLE_STATUS;         // Wifi radio status

// IP
bool dhcpMode = DHCP_MODE;
IPAddress ip;                     

// UDP
WiFiUDP udp;
unsigned int localPort = LOCALPORT;     // Te gebruiken UDP poort, zie secrets.h
#define REPLYBUFFER_SIZE  2048
char replyBuffer[REPLYBUFFER_SIZE];     // Buffer voor uitgaande data

//------
// Setup
//------
void setup() {
  
  // Seriële instellen
  Serial.begin(9600);
  while (!Serial) { ;}              // wait for serial port to connect. Needed for native USB port only
  
  // Wifi module aanwezig ?
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Geen wifi module gevonden, programma stopt !");
    // Stop
    while (true);
  }

  // Recentste firmware in wifi module ?
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Gelieve een upgrade van de wifi firmware uit te voeren!");
  }

  // Wifi instellen
  if (!dhcpMode) { 
    ip.fromString(IP_ADDRESS);
    WiFi.config(ip);
  }
  
  // Start UDP server
  if (udp.begin(localPort) != 1) {  
    Serial.println("UDP kan niet worden gestart, programma stopt !");
    while(true); 
  } 


  while (status != WL_CONNECTED) {
    Serial.print("Probeer om te verbinden met ");
    Serial.println(ssid);
    // Verbind met Wifi in WPA/WPA2 mode:
    status = WiFi.begin(ssid.c_str(), password.c_str());
    // Wacht 10 seconden en probeer opnieuw
    delay(10000);
  }

  // We zijn verbonden, print info.
  Serial.println("\nVerbonden met het volgend netwerk:");
  printCurrentNet();
  printWifiData();
}

//-----
// Loop
//-----
void loop() {

  String replyString = "";
  String temp = "";

  // UDP pakket binnengekomen ?
  int packetSize = udp.parsePacket();

  // Verwerk indien een pakket binnengekomen.
  if (packetSize) {
    temp = "";    
    char c;
    
    // We lezen kar per kar. Functie read(buffer, size) leest blijkbaar max. 64 kar zodat we meerdere
    // malen zouden moeten lezen indien meer dan 64 kar.
    for (int i=0; i<packetSize; i++) {
      c=udp.read();
      temp+=c;
    }
    Serial.print("Packetsize: ");
    Serial.println(packetSize);
    Serial.print("Verstuurd: ");
    Serial.println(temp);
    replyString = temp;
    udpSendString(replyString);  
  }
}

//----------
// Functions
//----------

// --> UDP functions <--
void udpSendString(String toSend) {
  String temp;

  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  toSend.toCharArray(replyBuffer,REPLYBUFFER_SIZE);
  udp.write(replyBuffer);
  udp.endPacket();
}

// --> Wifi functions begin ---
void printWifiData() {
  // Print IP-adres:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP-adres: ");
  Serial.println(ip);

  // Print MAC-adres
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC-adres: ");
  printMacAddress(mac);
  Serial.println();  Serial.println();
}

void printCurrentNet() {
  // Print ssid vh netwerk waar we mee verbonden zijn.:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print het mac-adres van vh AP waar we mee verbonden zijn.
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // Print de signaal sterkte.
  long rssi = WiFi.RSSI();
  Serial.print("Signaal sterkte (RSSI):");
  Serial.println(rssi);

  // Print type encryptie.
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryptie type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
}
