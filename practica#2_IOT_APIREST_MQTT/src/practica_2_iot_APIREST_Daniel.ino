//---------------------------Librerias necesarias-------------------------------------
#include <ArduinoJson.h> 
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
//---------------------------declaracion de variables globales---------------------------------
const char* ssid = "";
const char* password = "";
const char* mqttServer = ""; 
const int mqttPort = ;// Por defecto se empela el 1883
WiFiClient cliente;
WiFiClient espClient;
PubSubClient client(espClient);
String url = "";//http://worldtimeapi.org/api/timezone/
String entrada_usuario = "";
//--------------------------Función para conectarse a internet------------------------------
void conectarse_internet() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
//---------------------------------Se comunica con la API y se procesa su respuesta en formato JSON---------------------------------------------------------
void restapi(String enlace)
{
   HTTPClient http; 
   if (http.begin(cliente, enlace))
   {
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();
      if (httpCode > 0) { 
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);
         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
            client.publish("StatusRequest", "Se recibio una respuesta");
            //PROCESAMENTO JSON
            StaticJsonDocument<768> arhivojson;
           DeserializationError error = deserializeJson(arhivojson, payload);
            if (error) {
            client.publish("JsonStatus", "esta zona horaria no cumple con el formato esperado !!!");
            return;
            } else{
               client.publish("JsonStatus", "Se obtuvo el archivo .JSON exitosamente :)!!");
            }
            String dia = "";
            String mes = "";
           const char* utc_datetime = arhivojson["utc_datetime"];
         String datetime = arhivojson["datetime"];
         int day_of_week = arhivojson["day_of_week"];
         if (day_of_week == 0){dia = "Domingo";}
         if (day_of_week == 1){dia = "Lunes";}
         if (day_of_week == 2){dia = "Martes";}
         if (day_of_week == 3){dia = "Miercoles";}
         if (day_of_week == 4){dia = "Jueves";}
         if (day_of_week == 5){dia = "Viernes";}
         if (day_of_week == 6){dia = "Sabado";}
        String hora = datetime.substring(11, 19);
        String meso = datetime.substring(5, 7);
        String numdia = datetime.substring(8, 10);
        String año = datetime.substring(0, 4);
        if (meso == "1"){mes = "Enero";}
        if (meso == "2"){mes = "Febrero";}
        if (meso == "3"){mes = "Marzo";}
        if (meso == "4"){mes = "Abril";}
        if (meso == "5"){mes = "Mayo";}
        if (meso == "6"){mes = "Junio";}
        if (meso == "7"){mes = "Julio";}
        if (meso == "8"){mes = "Agosto";}
        if (meso == "09"){mes = "Septiembre";}
        if (meso == "10"){mes = "Octubre";}
        if (meso == "11"){mes = "Noviembre";}
        if (meso == "12"){mes = "Diciembre";}
        String v1 = " de ";
        String v2 = " -- ";
        String v3 = ", ";
        String cadr = dia + v3 + numdia + v1 + mes + v1 + año + v2 + hora ; 
        Serial.println(cadr);
        client.publish("Output", cadr.c_str());
        reconnect();
         }
         else{ 
          String err="Se presento un error de tipo:  ";
           String errorNum = String(httpCode);
           err= err+errorNum;
            client.publish("StatusRequest", err.c_str());
            reconnect();
         }
      }
      else {
         client.publish("StatusRequest", "ALGO SALIO MAL :(");
         reconnect();
      }
      http.end();
   }
   else {
      Serial.printf("[HTTP} Unable to connect\n");
   }
}
//---------------------------Función callback - recibe el mensaje del broker al que esta suscrito-------------------------- 
void callback(char* topic, byte* payload, unsigned int length) { 
  for (int i = 0; i < length; i++) {
    entrada_usuario+=(char)payload[i];
    Serial.print((char)payload[i]);
  }
  url+=entrada_usuario;
  entrada_usuario="";
  restapi(url);
  url="http://worldtimeapi.org/api/timezone/";
}

//-----------------------------------INICIA SETUP----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  conectarse_internet();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", NULL, NULL)) {
        client.publish("Alive", "Alive"); //Publicando en el topic alive para saber que la conexion fue exitosa. 
        reconnect(); //Funcion reconectar
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe("Input");
 
}
//--------------------------------Funcion loop-------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
void reconnect() {
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("ESP8266Client", NULL, NULL )) {
      Serial.println("connected");
      client.subscribe("Input");
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
