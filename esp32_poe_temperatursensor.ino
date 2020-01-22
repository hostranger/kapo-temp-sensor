/*
    This sketch shows the Ethernet event usage

*/
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12


#include <ETH.h>
#include <WebServer.h>
#include <DHTesp.h>

/* Put IP Address details */
IPAddress local_ip(192,168,10,124);
IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);
IPAddress dns1(192,168,10,10);

WebServer server(80);
DHTesp dht;

char * Hostname = "ESP32-temp-sensor1";

// float Temperature = 10;
int dhtPin = 0;

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
//      ETH.setHostname("esp32-ethernet");
      ETH.setHostname(Hostname);
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void testClient(const char * host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleTempReadout() {
//  Temperature = 24;
  delay(dht.getMinimumSamplingPeriod());

//  float Humidity = dht.getHumidity();
  float Temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  
  Serial.println("Temperature: " + String(Temperature));
  server.send(200, "text/plain", String(Temperature)); 
//  server.send(200, "text/plain", "Temp: ");
}

void setup()
{
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  ETH.config(IPAddress(local_ip),
             IPAddress(gateway),
             IPAddress(subnet),
             IPAddress(dns1));
  server.on("/", handleRoot);
  
  server.on("/test", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/temp", handleTempReadout);

  server.onNotFound(handleNotFound);

  server.begin();
}


void loop()
{
//  if (eth_connected) {
//    testClient("google.com", 80);
//    testServer();
//  }
  server.handleClient();
}
