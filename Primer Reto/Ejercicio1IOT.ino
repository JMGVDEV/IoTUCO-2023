#include <ESP8266WiFi.h> // Libreria para el ESP8266

const char* ssid = "xxxxxxx"; // Nombre de la red Wi-Fi
const char* password = "xxxxxxx"; // Contraseña de la red Wi-Fi

int ledPin = 2; // GPIO2 / D4 - Pin para controlar el LED interno del ESP8266

WiFiServer server(80); // Inicialización del servidor en el puerto 80

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(ledPin, OUTPUT); // Configura el pin del LED como salida
  digitalWrite(ledPin, LOW); // Apaga el LED al inicio
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); // Conexión a la red Wi-Fi
  
  while (WiFi.status() != WL_CONNECTED) { // Espera a que el ESP8266 se conecte a la red
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  server.begin(); // Inicializa el servidor
  Serial.println("Server started");
  
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP()); // Imprime la dirección IP local del ESP8266
  Serial.println("/");
}

void loop() {
  WiFiClient client = server.available(); // Espera a que haya un cliente disponible
  if (!client) {
    return;
  }
  
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }
  
  String request = client.readStringUntil('\r'); // Lee la solicitud del cliente
  Serial.println(request);
  client.flush();
  
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) { // Si la solicitud contiene /LED=ON, enciende el LED
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  
  if (request.indexOf("/LED=OFF") != -1) { // Si la solicitud contiene /LED=OFF, apaga el LED
    digitalWrite(ledPin, LOW);
    value = LOW;
  }
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  
  client.print("Led pin is now: ");
  
  if (value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");
  
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
}
