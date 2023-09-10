//Librerias utilizadas
#include <ArduinoJson.h> //Para procesar el formato Json
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //Para el protocolo MQTT
 
const char* ssid = "xxxxxxxxx";
const char* password = "xxxxxx";
const char* mqttServer = "xxx.xxx.xxx.xxx"; //Ip de la maquina donde esta alojado el broker
const int mqttPort = 1883;//Puerto usado por el servicio mosquitto es este por defecto 
const char* mqttUser = "";
const char* mqttPassword = ""; //Vacio por que el practica no usamos autenticacion
//Topicos mqtt
const char* TopicSub = "Input"; //A este se suscribe

//Topicos de publicacion
const char* TopicAlive = "Alive";
const char* TopicStatusRequest = "StatusRequest";
const char* TopicJsonStatus = "JsonStatus";
const char* TopicOutput = "Output";

WiFiClient cliente;
WiFiClient espClient;
PubSubClient client(espClient);

//Url que hay que completar para consultar
String url = "http://worldtimeapi.org/api/timezone/";

//Ciudad de la que queremos consultar la hora
String ciudad = "";


/********* Setup wifi ***************************
   setup wifi connect to wifi with the constants
   defined up
   while does not connect print a "."
   if connect then print the local ip
************************************************/
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


//Funcion para realizar la consulta una vez obtenida la zona horaria.
void restapi(String urlcompleta)
{
   HTTPClient http; // HTTPClient es el tipo de la variable 
 
   if (http.begin(cliente, urlcompleta)) //Start connection CON EL CLIENTE WIFI Y LA URL
   {
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();  // GET Request NOTA SI LA RESPUESTA ES 200 EL COMUNICACION CON LA PAGINA FUE EXITOSA Y SI SALE 404 NO SE ENCONTRO ESTA SE GUARDA EN LA VARIABLE HTTPCODE
      if (httpCode > 0) { 
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);
 
         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();   // Get HTTP response REALIZAR EL GET A LA PAGINA DEL API
          // LA VARAIBLE payload ES DONDE ESTA LA INFO QUE sE DEBE PROCESAR EN FORMATO JSON

            Serial.println(payload);

            client.publish(TopicStatusRequest, "OK");  // Print content of payload  
            //PROCESAMENTO JSON
            StaticJsonDocument<768> resp;


           DeserializationError error = deserializeJson(resp, payload);
            //Verificando que el json no tenga error
            if (error) {
            client.publish(TopicJsonStatus, "Json no obtenido, por favor coloque una zona horaria valida");
            return;
            } else{
               client.publish(TopicJsonStatus, "Json obtenido con exito");
            }
           
           //Strings para guardar y publicar las variables
            String dia1 = "";
            String year = "";
            String nd = "";
            String mes = "";
            String hora = "";
            String sep1 = " -- ";
            String sep2 = " de ";
            int nmes;

            //Evaluacion dia de la semana
            int dia = resp["day_of_week"];

            if (dia==0){
                dia1="Domingo, ";
            }

            if (dia==1){
                 dia1="Lunes, ";
            }

            if (dia==2){
                dia1="Martes, ";
            }
            if (dia==3){
                dia1="Miercoles, ";
            }
            if (dia==4){
                 dia1="Jueves, ";
            }
            if (dia==5){
                dia1="Viernes, ";
            }
            if (dia==6){
                dia1="Sabado, ";
            }

            

            //Sacar los datos de hora y fecha
            String fyh = resp["datetime"];
            //Scar los datos recorriendo lo obtenido en datetime
            year= fyh.substring(0,4);
            nd= fyh.substring(8,10);
            mes= fyh.substring(5,7);
            hora= fyh.substring(11,16);

            nmes=mes.toInt();

             if (nmes==1){
                mes="Enero";
            }

            if (nmes==2){
                mes="Febrero";
            }

            if (nmes==3){
                mes="Marzo";
            }

            if (nmes==4){
                mes="Abril";
            }

            if (nmes==5){
                mes="Mayo";
            }

            if (nmes==6){
                mes="Junio";
            }

            if (nmes==7){
                mes="Julio";
            }

            if (nmes==8){
                mes="Agosto";
            }

            if (nmes==9){
                mes="Septiembre";
            }

            if (nmes==10){
                mes="Octubre";
            }

            if (nmes==11){
                mes="Noviembre";
            }

            if (nmes==12){
                mes="Diciembre";
            }

            //Unificacion de todos los valores para dar el resultado final
            String pib= dia1 + nd + sep2 + mes + sep2 + year + sep1 + hora ; 

            client.publish(TopicOutput, pib.c_str());



            //Reconexion al servidor
            reconnect();
         }

         //Respuesta si sale un error en el get

         else{ 
          String err="Error ";
           String errorNum = String(httpCode);

           err= err+errorNum;
           
            client.publish(TopicStatusRequest, err.c_str());  // Print content of payload 
            reconnect();


         }
      }

      //Excepcion si falla el get
      else {
         Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
         client.publish(TopicStatusRequest, "ERROR");  // Print content of payload 
         reconnect();
      }
 
      http.end(); //FINALIZACION DE LA CONEXION 
   }
   else {
      Serial.printf("[HTTP} Unable to connect\n"); // CUANDO HAY UN ERROR DE RED
   }
 
  
}


/********* MQTT Callback ***************************
   here is defined the logic to execute after
   a messages arrives in the desired
   topic, for this example the variable:
   TopicSub
************************************************/
void callback(char* topic, byte* payload, unsigned int length) {

  //Notify about message arrived 
 // Serial.print("Message arrived in topic: ");
 // Serial.println(topic);
  
  //Print the message received
  //Serial.print("Message:");

  //Recibiendo el mensaje del topico donde esta la ciudad 
  for (int i = 0; i < length; i++) {
    ciudad+=(char)payload[i];
    Serial.print((char)payload[i]);
  }
  
  //Prints with visualization purposes
  //Serial.println();
  //Serial.println("-----------------------");

  //Completando la URL  para la api
  url+=ciudad;
  //Reseteo del valor de ciudad.
  ciudad="";
  //Llamando la funcion restapi para hacer consulta
  restapi(url);
  //Reseteo de la url inicial
  url="http://worldtimeapi.org/api/timezone/";
 
}

//Funcion del setput
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
        client.publish(TopicAlive, "Alive"); //Publicando en el topic alive para saber que la conexion fue exitosa. 
        reconnect(); //Funcion reconectar
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
  //Publish to desired topic and subscribe for messages
  
  client.subscribe(TopicSub);
 
}
//Loop para que el esp siempre este conectado al servidor MQTT.
 
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  //MQTT client loop
  client.loop();
}

//Funcion para realizar la reconexion al servidor MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      // ... and resubscribe
      client.subscribe(TopicSub);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
