#include <ESP8266WiFi.h> //Esta biblioteca permite que el ESP8266 se conecte a una red Wi-Fi.
#include <ESP8266HTTPClient.h> //// Esta biblioteca permite realizar solicitudes HTTP con el ESP8266.
#include <PubSubClient.h> // Se utiliza para establecer una conexión MQTT y publicar/suscribirse a temas MQTT.
#include <ArduinoJson.h> // Esta biblioteca se utiliza para analizar y generar datos en formato JSON.
//WIfi SSID & PASS
const char* ssid = "QuintanaDaza"; //
const char* password = "americadecali74"; //Contraseña

//Configuración MQTT
const char* mqttServer = "190.240.28.22"; // IP DE LA MÁQUINA
const int mqttPort = 1883; //Puerto usado por el servicio mosquitto
const char* mqttUser = ""; // Usuario MQTT (si es necesario)
const char* mqttPassword = ""; // Contraseña MQTT (si es necesario)

//Topics
const char* TopicSub = "Input"; //Tema al que te suscribes
const char* TopicPubAlive = "Alive"; //Tema para publicar "Vivo"
const char* TopicPubStatus = "StatusRequest"; //Tema para publicar "Solicitud de Estado"
const char* TopicPubJson = "JsonStatus"; //Tema para publicar "Estado en JSON"
const char* TopicPubOut = "Output"; //Tema para publicar "Estado en JSON"


/********* Declara variables y URL ***************************
   A las variables no se les asigna un valor especifico
   ya que dependerá de como se vayan a utilizar
*************************************************************/

//Se declaran variables y la URL
WiFiClient espClient; //Se utiliza para establecer conexiones a través de Wi-Fi Y para establecer una conexión con servidor MQTT
PubSubClient clienteMQTT (espClient); //Establecer conexiones MQTT y permite publicar y suscribirse a temas MQTT en ESP8266

String url ="http://www.worldtimeapi.org/api/timezone/";

String mensaje = "";
String diaSemana = "";
WiFiClient clienteWifi; //Declara una instancia de clase, establece conexiones y comunicaciones a través de wifi

/********* setup_wifi ***************************
   Configura la conexión Wi-Fi con las credenciales definidas
   mientras no se conecta, muestra puntos de espera.
  Cuando se conecta se muestra la dirección IP local.
*************************************************************/
void configurar_wifi() { //Configura la conexión de Wi-Fi, cuando se conecta muestra la dirección IP
  delay(10);
  //Comenzamos conectándonos a una red Wi-Fi
  Serial.println();
  Serial.print(F("Conectando a "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.print(F("."));
  Serial.println("WiFi conectado");
}
void connect_MQTT() {
 while (!clienteMQTT.connected()) {
    Serial.println("Conectando a MQTT...");

    if (clienteMQTT.connect("ESP8266Client", mqttUser, mqttPassword)) {
      clienteMQTT.publish(TopicPubAlive, "Alive");
      Serial.println("Conectado a MQTT");
    } else {
      Serial.print("Error en la conexión MQTT, estado: ");
      Serial.print(clienteMQTT.state());
      delay(2000);
    }
  }
}  

void setup() {
  // Inicia la comunicación serial
  Serial.begin(115200);
  configurar_wifi();
  
  clienteMQTT.setServer(mqttServer, mqttPort);
  clienteMQTT.setCallback(callback);
  connect_MQTT();
  clienteMQTT.subscribe(TopicSub);
}
void loop() { //La función loop() se encarga de realizar una solicitud HTTP GET a una URL específica y luego imprimir el contenido de la respuesta en el puerto serial.
  if (!clienteMQTT.connected()) {
     connect_MQTT();
  }
   clienteMQTT.loop();
  }
String obtenerFechaYHora( String url) {
  HTTPClient http;

  if (http.begin(espClient,url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();

      DynamicJsonDocument jsonDoc(1024);
      DeserializationError error = deserializeJson(jsonDoc, payload);

      if (!error) {
        String datetime = jsonDoc["datetime"];
        return datetime;
      } else {
        Serial.println("Error al analizar JSON");
      }
    } else {
      Serial.println("Error en la solicitud HTTP");
      http.end();
    }
  } else {
    Serial.println("No se pudo iniciar la conexión HTTP");
  }

  return "";
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en tema: ");
  Serial.println(topic);

  for(int i=0; i< length; i++)
  {
    mensaje+=(char) payload[i];
  }
 url+=mensaje;
 mensaje="";
 obtenerFechaYHora(url);
 url= "http://www.worldtimeapi.org/api/timezone/";
}
