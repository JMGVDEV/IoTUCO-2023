#include <ESP8266WiFi.h> //Esta biblioteca permite que el ESP8266 se conecte a una red Wi-Fi.
#include <ESP8266HTTPClient.h> //// Esta biblioteca permite realizar solicitudes HTTP con el ESP8266.
#include <PubSubClient.h> // Se utiliza para establecer una conexión MQTT y publicar/suscribirse a temas MQTT.
#include <ArduinoJson.h> // Esta biblioteca se utiliza para analizar y generar datos en formato JSON.
//WIfi SSID & PASS
const char* ssid = "xxxxxxxxxx"; //
const char* password = "xxxxxxxxx"; //Contraseña

//Configuración MQTT
const char* mqttServer = "xxxxxxxxxxxx"; // IP DE LA MÁQUINA
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

/********* Llamada obtenerFechayHora ***************************
  Esta función se encarga de realizar una solicitud HTTP a una URL 
  especificada, obtener una respuesta en formato JSON, analizar esa 
  respuesta JSON y extraer la fecha y hora de ella
*************************************************************/
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
  
String obtenerFechaYHora(String url) {
  HTTPClient http; //Se crea instancia utilizada para realizar solicitudes HTTP

  if (http.begin(espClient, url)) { // Iniciar la conexión HTTP
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) { // Comprobar si la solicitud HTTP fue exitosa
      String payload = http.getString(); //Si la solicitud es exitosa, se obtiene el contenido de la respuesta HTTP como una cadena de texto y se almacena en la variable payload
      http.end(); //Se finaliza la conexion

      clienteMQTT.publish(TopicPubStatus, "OK" );

      DynamicJsonDocument jsonDoc(1024); //Se crea un objeto para almacenar el documento JSON 
      DeserializationError error = deserializeJson(jsonDoc, payload); //Analiza la cadena y se almacena

      if (!error) { //Verifica si hubo algun error
        String datetime = jsonDoc["datetime"];
        clienteMQTT.publish(TopicPubJson, "JSON conseguido correctamente" );
        return datetime;
      } else {
        clienteMQTT.publish(TopicPubJson, "JSON no conseguido correctamente" );
        Serial.println("Error al analizar JSON");
      }

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
      clienteMQTT.publish(TopicPubOut, outputday);
      
    } else {
      clienteMQTT.publish(TopicPubStatus, ("Error...%s", http.errorToString(httpCode).c_str() ) );
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
