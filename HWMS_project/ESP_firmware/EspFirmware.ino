/*
  ======================================================================
  |HWMS🌤️ ESP 8266 (ESP 01) firmware by https://github.com/Maxdgf      |
  ======================================================================
  - Creates web server with html page
  -[Note!] -> ESP 8266 (ESP 01) works on specific wifi network! all network data must be specified accurately.
*/

#include <string.h>
#include <sstream>
#include <vector>

#include <ESP8266WiFi.h>

using namespace std;

//Creating server object with channel
WiFiServer server(80);

//WiFi network data, replace with your data
const String WIFI_NAME = "wifi_name"; //WiFi network name
const String WIFI_PASSWORD = "password"; //WiFi network password
const unsigned long TIMEOUT = 5000; //webserver connection timeout

//Static esp 8266 (esp 01) ip configuration, replace with your data
IPAddress staticIP(0, 0, 0, 0); //your static ip
IPAddress gateWay(0, 0, 0, 0); //your WiFi network gateway
IPAddress subnet(0, 0, 0, 0); //your WiFi network subnet

//Web page style
const String WEB_PAGE_STYLE = 
"<style>"
"body { background-color: #0aabf0; font-family: Segoe UI, Tahoma, Geneva, Verdana, sans-serif; color: #fff; text-align: center; }"
"div { border: 5px solid #fff; border-radius: 10px; margin-top: 15px; }"
"p { font-style: italic; }"
"</style>";

//Local ip and input data from serial port
String localIp;
String inputData;

void setup() {
  Serial.begin(115200);
  delay(100);

  //Static ip config
  WiFi.config(staticIP, gateWay, subnet);
  //WiFi object begin
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD); 
  
  while (WiFi.status() != WL_CONNECTED)             
  {
    delay(500);                                          
    Serial.print("=");                                   
  }

  Serial.println("> Connected!");
  server.begin();
  Serial.println("server running."); 
               
  delay(10000);     

  //Checking static ip
  if(!WiFi.config(staticIP, gateWay, subnet)) {
    Serial.println("Failed to configure Static IP");
  } else {
    Serial.println("Static IP configured!");
  }

  IPAddress ip = WiFi.localIP();
  localIp = ip.toString();

  Serial.print("ESP 01 ip: ");
  Serial.println(WiFi.localIP()); 

  //Get wifi network info for static esp ip config

  /*Serial.print("Gateway (router) IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet Mask: " );
  Serial.println(WiFi.subnetMask());
  Serial.print("Primary DNS: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("Secondary DNS: ");
  Serial.println(WiFi.dnsIP(1));*/
}

//This function converts input data string separated by commas from serial port to string vector for easy interaction
vector<String> convertSerialDataToStringArray(String data) {
  vector<String> parameters;

  int startIndex = 0;
  int commaIndex = 0;

  while (true) {
    commaIndex = data.indexOf(',', startIndex);
    if (commaIndex == -1) {
      parameters.push_back(data.substring(startIndex));
      break;
    } else {
      parameters.push_back(data.substring(startIndex, commaIndex));
      startIndex = commaIndex + 1;
    }
  }  

  return parameters;
}

void loop() {
  //Creating client
  WiFiClient client = server.available();   

  //Start reading data from serial port
  if (Serial.available()) {
    inputData = Serial.readStringUntil('\n');
  }

  //inputData = "37ºC,30%,743.70mmHg,test weather,2"; data from serial port example (test)
  vector<String> weatherData = convertSerialDataToStringArray(inputData); //convert data from serial port to string vector

  if (client) {
    Serial.println("New client");                        

    String currentLine = ""; //current line
    unsigned long elapsedTime = millis(); // elapsed time

    //sending requests while client connected and elapsed time less timeout
    while (client.connected() && (millis() - elapsedTime < TIMEOUT)) {
      if (client.available()) {
        char c = client.read(); //current readed symbol
        elapsedTime = millis(); //reset elapsed time

        if (c == '\n') {
          if (currentLine.length() == 0) {
            //request body
            client.println("HTTP/1.1 200 OK");               
            client.println("Content-Type: text/html"); 
            client.println("Connection: close");             
            client.println("Refresh: 1"); //refresh frequency in seconds            
            client.println();
            client.println("<!DOCTYPE HTML>");             
            client.println("<html>");                     
            client.println("<head>");
            client.print("<title>HWMS weather control</title>");  
            client.println("<meta charset=UTF-8>"); 
            client.println("</head>");
            client.println("<body>");
            client.println(WEB_PAGE_STYLE);
            client.println("<p style='font-weight: bolder; font-size: 30px;' >HWMS🌤️</p>"); 
            client.println("<h3>Home Weather Monitoring System</h3>"); 
            client.println("<h3>Track temperature🌡️, humidity💧, and atmospheric pressure☁️ here!</h3>"); 
            client.println("<h4>current ip: ");
            client.println(localIp);
            client.println("</h4>");
            client.println("<div>");
            //paste data elements from string vector
            client.println("<h1 style='color: ");
            client.println(weatherData[5]);
            client.println(";' >🌡️Temperature: ");
            client.println(weatherData[0]);
            client.println("</h1>");
            client.println("<h1 style='color: ");
            client.println(weatherData[6]);
            client.println(";' >💧Humidity: ");
            client.println(weatherData[1]);
            client.println("</h1>");
            client.println("<h1 style='color: ");
            client.println(weatherData[7]);
            client.println(";' >☁️Pressure: ");
            client.println(weatherData[2]); 
            client.println("</h1>");
            client.println("<h1>📄Note: ");
            client.println(weatherData[3]); 
            client.println("</h1>");
            client.println("<h1>📊Weather quality (");
            client.println(weatherData[4]);
            client.println("/ 3 ): <progress value=");
            client.println(weatherData[4]);
            client.println(" max=3></progress></h1>");
            client.println("</div>"); 
            client.println("</body>");
            client.println("</html>"); 
            break; 
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    delay(10);
    client.stop(); //stop client                                  
    Serial.println("Client disconnected."); 
  }
}
