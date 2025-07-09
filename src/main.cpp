/// Wifi UDP server that acts as a serial port gateway
///
/// Connections:
///  - Wifi: UDP connection for users
///  - Serial (USB serial): Also for user side
///  - Serial0 (Hardware serial): for the RustyBugA side

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

// Wifi credentials
// secrets.hpp should define WIFI_SSID, WIFI_PASS, and MDNS_HOSTNAME.
// check secrets_example.hpp for an example
#include "secrets.hpp"
char *ssid = WIFI_SSID;
char *pass = WIFI_PASS;
char *mdns_hostname = MDNS_HOSTNAME;

// Udp server setup
WiFiUDP udp;
const unsigned int localUdpPort = SERVER_PORT;
uint8_t incomingPacket[255];
static IPAddress lastRemoteIP = IPAddress(0, 0, 0, 0);
static uint16_t lastRemotePort = 0;

// Serial port setup
// https://forum.seeedstudio.com/t/how-to-use-serial1-with-xiao-esp32c3/266306/5
HardwareSerial Serial_0(0);

void setup()
{
  // Configure the serial port
  Serial.begin(115200);
  while (!Serial)
  {
    ; // Wait for the serial port to connect. Needed for native USB port only
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to WiFi with ssid: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (!MDNS.begin(mdns_hostname))
  { // "esp32" will be the hostname (esp32.local)
    Serial.println("Error starting mDNS");
  }
  else
  {
    Serial.print("mDNS responder started with name: ");
    Serial.println(mdns_hostname);
  }

  // Start the UDP server
  udp.begin(localUdpPort);
  Serial.printf("UDP server started at port %d\n", localUdpPort);

  // Initialize Serial_1 for UART1
  Serial_0.begin(115200, SERIAL_8N1, D9, D10);
}

void loop()
{
  // 1. Receive UDP and send to Serial_0
  {
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
      int len = udp.read(incomingPacket, 255);
      if (len > 0)
      {
        Serial_0.write(incomingPacket, len);
      }
      // Save remote IP/port for replies
      lastRemoteIP = udp.remoteIP();
      lastRemotePort = udp.remotePort();
    }
  }

  // 2. Read Serial_0 and send to last UDP client and Serial
  while (Serial_0.available() > 0)
  {
    uint8_t buf[128];
    int n = Serial_0.readBytes(buf, sizeof(buf));

    Serial.write(buf, n);

    if (lastRemotePort != 0)
    {
      udp.beginPacket(lastRemoteIP, lastRemotePort);
      udp.write(buf, n);
      udp.endPacket();
    }
  }

  // 3. Read Serial and send to Serial_0
  while (Serial.available() > 0)
  {
    uint8_t buf[128];
    int n = Serial.readBytes(buf, sizeof(buf));
    Serial_0.write(buf, n);
  }

  delay(1);
}
