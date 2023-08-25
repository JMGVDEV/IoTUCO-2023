#include <ESP8266WiFi.h> //Libreria para el ESP8266


//Ingresar nombre de red y clave
const char* ssid = "ZZZZZZZ";
const char* password = "ZZZZZZZZ";
 
int ledPin = 5; // Declarar el pin que vamos a usar para el led. 

//Inicializacion del servidor web (Se declara el servidor en el puerto 80)
WiFiServer server(80);
 
void setup() {
  Serial.begin(115200); //Inicializar el puerto para la comunicacion serial a 115200 baudios
    delay(10); // Espera de 10 ms para que todo tenga tiempo de inicializar
 
  pinMode(ledPin, OUTPUT); //Se configura el pin donde ira el led como salida y se le da intruccion de apagar.
  digitalWrite(ledPin, LOW);
 
  // Se imprime por comunicacion serial a cual red se realiza la conexion
  Serial.println();
  Serial.println();
  Serial.print("Conectando a:");
  Serial.println(ssid);
 
 //Peticion del esp para conectarse a la red buscando la red con el nombre y la contraseña suministrada
  WiFi.begin(ssid, password);

  //Bucle cuya funcion es imprimir puntos por el puerto serial de manera continua hasta que el ESP se conecte a la red.
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Una vez salga del bucle, significa que se logro conectar a la red lo cual va a dar aviso por medio del puerto serial.
  Serial.println("");
  Serial.println("Conexión WiFi exitosa");
 
  // Iniciar el servidor y se da aviso por le puerto serial
  server.begin();
  Serial.println("Servidor iniciado");
 
  // Una vez iniciado el servidor se procede a imprimir la URL de este para que el usuario pueda conectarse (la direccion se la asigna el router a la placa). 
  Serial.print("Usa esta dirección IP en el navegador para conectarte al servidor:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}
 
void loop() {
  //Se garantiza que haya algun cliente conectado, se crea un objeto cliente, pero 
  //si no hay nungun cliente conectado al servido el objeto va a ser igual a falso 
  //se queda ahi hasta que un cliente se conecte. 
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  //Cuando se conecta un cliente el esp avisa por medio del puerto serial y se queda 
  //En el bucle esperando que el cliente haga una peticion 
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 

 //Cuando se recibe una solicitud por parte del cliente, en la variable string request 
 //Se guarda la primera linea de la peticion del cliente  y luego se imprime la solicitud de datos que nos hizo el cliente
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  //Borrar todos los datos que quedaron el buffer de cliente (La solicitud)
  client.flush();
 
  // Respuesta al requerimiento: Con la variable value controlamos el estado del pin
 //Si el requerimiento del cliente contiene /LED=ON se enciende el led (Esto se logra gracias a los botones)
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }

   //Si el requerimiento del cliente contiene /LED=OFF se apaga el led
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }
 
//Una vez se realizan los cambios se debe actualizar la pagina pata que se vea si el led esta prendido o apagado
 //Eso se hace con la funcion client.println
  // Return the response

  //De aqui en adelante es la creacion de codigo HTML para la pagina del servidor 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  
 


//El servidor envia todas las lianes de codigo al servidor generando el html

 //Estado del pin
  client.print("El led en en este momento esta: ");
 
  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");

  //Cuando el servidor muentra el html desconecta al cliente porque se termina el loop, 
  //no mantiene al cliente conectado, el servidro conecta al cliente cada vez que le hace una peticion
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");

//Los botones con envian a una direccion xxx.xxx.xxx.xxx/LED=ON o LED=OFF
//Esas direcciones son peticiones que el cliente le hace al servidor para apagar o prender elv led
//Eso se ve por le monitor serial 
 
}