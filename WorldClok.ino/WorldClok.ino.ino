#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password =  "";
const char* mqttServer =" ";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const char* TopicSub = "input";
const char* TopicPub = "alive";

WiFiClient clientWifi;
WiFiClient espClient;
PubSubClient client(espClient);
String urlWeb = "http://www.worldtimeapi.org/api/timezone/";
String message = "";
String dayWeek = "";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print(F("Connecting to ")) ;
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
String getMonthName(int month) {
  switch (month) {
    case 1: return "Enero";
    case 2: return "Febrero";
    case 3: return "Marzo";
    case 4: return "Abril";
    case 5: return "Mayo";
    case 6: return "Junio";
    case 7: return "Julio";
    case 8: return "Agosto";
    case 9: return "Septiembre";
    case 10: return "Octubre";
    case 11: return "Noviembre";
    case 12: return "Diciembre";
    default: return "Mes Invalido";
  }
}

void messageConverter(String messageREST) {
  // Define a DynamicJsonDocument
  DynamicJsonDocument jsonDoc(800); // Increase the size as needed

  // Parse JSON data into the JsonDocument
  DeserializationError error = deserializeJson(jsonDoc, messageREST);

  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    client.publish(TopicPubJson, "Data zone didn't found");
    return;
  }

  client.publish(TopicPubJson, "Data zone found it");

  String datetime = jsonDoc["datetime"];
  int dayOfWeek = jsonDoc["day_of_week"];
  String dayWeek;

  switch (dayOfWeek) {
    case 1: dayWeek = "Lunes"; break;
    case 2: dayWeek = "Martes"; break;
    case 3: dayWeek = "Miércoles"; break;
    case 4: dayWeek = "Jueves"; break;
    case 5: dayWeek = "Viernes"; break;
    case 6: dayWeek = "Sábado"; break;
    case 7: dayWeek = "Domingo"; break;
  }

  // Extract date and time components
  int year = datetime.substring(0, 4).toInt();
  int month = datetime.substring(5, 7).toInt();
  int day = datetime.substring(8, 10).toInt();
  int hour = datetime.substring(11, 13).toInt();
  int minute = datetime.substring(14, 16).toInt();

  char outputday[50]; // Adjust the size as needed
  snprintf(outputday, sizeof(outputday), "%s, %02d de %s de %04d -- %02d:%02d", dayWeek.c_str(), day, getMonthName(month).c_str(), year, hour, minute);
  client.publish(TopicPubOut, outputday);
}

void RestApi (String urlREST){
  HTTPClient http;
   if (http.begin(clientWifi, urlREST)) //Start connection
   {
      int httpCode = http.GET();  // GET Request
      if (httpCode > 0) {
 
         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          client.publish(TopicPubStatus, "OK");
          String payload = http.getString();   // Get HTTP response
          messageConverter(payload);
         }
         else{
          client.publish(TopicPubStatus, "Error");
         }
      }
      else {
      }
 
      http.end();
   }
   else {
   }
 
   delay(1500);
}

/********* MQTT Callback ***************************
   here is defined the logic to execute after
   a messages arrives in the desired
   topic, for this example the variable:
   TopicSub
************************************************/
void callback(char* topic, byte* payload, unsigned int length) {

  //Notify about message arrived 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  //Print the message received
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  //Prints with visualization purposes
  Serial.println();
  Serial.println("-----------------------");
 
}

void setup() {
 
  //Start Serial Communication
  Serial.begin(115200);
  
  //Connect to WiFi
  setup_wifi();

  //Connect to MQTT Broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  //MQTT Connection Validation
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  //Publish to desired topic and subscribe for messages
  client.publish(TopicPub, "Hello from ESP8266");
  client.subscribe(TopicSub);
 
}
 
void loop() {
  //MQTT client loop
  client.loop();
}