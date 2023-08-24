#include <ESP8266WiFi.h>   //incluir las librerias necesarias
#include <ESP8266WebServer.h>

//se crea la red
const char *NOMBRE_RED = "IOT-PRACTICA-1",
           *CLAVE_RED = ""; //digite una contraseña de minimo 8 digitos para su red
const int PUERTO = 80,LED_D1 = 2; //GPIO 2 = D4
int LED_D2 = 14; //GPIO 14 = D5

// Tener un estado del LED/ variables booleanas 
bool estadoDelLED = false;
bool estadoDelLED2 = false;

/se ajusta la dirección ip de la  red y se emplea el servidor por el puerto 80
IPAddress ip(192, 168, 1, 1);
IPAddress puertaDeEnlace(192, 168, 1, 1);
IPAddress mascaraDeRed(255, 255, 255, 0);
ESP8266WebServer servidor(PUERTO);




//----------------------------------------inicio pagina html--------------------------------------------------
String obtenerInterfaz()
{
  String HtmlRespuesta = "<!DOCTYPE html>"
                         "<html lang='en'>"
                         "<head>"
                         "<meta charset='UTF-8'>"
                         "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                         "<title>iot-practica-1</title>"
                         "</head>"
                         "<body>";
  HtmlRespuesta += "<center style='color: orange;font-size:40px;'>Bienvenido a la actividad N°1</center>";
  HtmlRespuesta += "<marquee style='color: blue;font-size:35px;'>***IoT 2023***</marquee>";
  
  HtmlRespuesta += "<center>Estado del led N°1 ----> ";
  if (estadoDelLED)
  {
    HtmlRespuesta += "encendido</center>";
    HtmlRespuesta += "<a center style='color: green;font-size:30 px;' href='/apagar'>---------------------------------Apagar----------------------------</a>";
  }
  else
  {
    HtmlRespuesta += "apagado</center>";
    HtmlRespuesta += "<a style='color: red;font-size:30 px;' href='/encender'>---------------------------------Encender----------------------------</a>";
  }

 HtmlRespuesta += "<center>Estado del led N°2 ----> ";
    if (estadoDelLED2)
  {
    HtmlRespuesta += "encendido</center>";
    HtmlRespuesta += "<a center style='color: green;font-size:30 px;' href='/apagar2'>---------------------------------Apagar----------------------------</a>";
  }
  else
  {
    HtmlRespuesta += "apagado</center>";
    HtmlRespuesta += "<a center style='color: red;font-size:30 px;' href='/encender2'>---------------------------------Encender----------------------------</a>";
  }
  
  HtmlRespuesta += "<center>ヽ༼ ಠ益ಠ ༽ﾉ";
                   "</html>";
  return HtmlRespuesta;
}

void mostrarInterfazHTML() //se llama la pagina almacenada en obtener Interfaz
{
  servidor.send(200, "text/html", obtenerInterfaz());
}
//----------------------------------------------------------------------------------------------------------

//-----------------------------------------Funciones-----------------------------------------------------
void rutaEncenderLed()
{
  estadoDelLED = true;
  digitalWrite(LED_D1, HIGH);
  mostrarInterfazHTML();
}

void rutaApagarLed()
{
  estadoDelLED = false;
  digitalWrite(LED_D1, LOW);
  mostrarInterfazHTML();
}


void rutaEncenderLed2()
{
  estadoDelLED2 = true;
  digitalWrite(LED_D2, HIGH);
  mostrarInterfazHTML();
}

void rutaApagarLed2()
{
  estadoDelLED2 = false;
  digitalWrite(LED_D2, LOW);
  mostrarInterfazHTML();
}

void rutaRaiz()
{
  mostrarInterfazHTML();
}

void rutaNoEncontrada()// en caso de error se proyecta el mesaje de err 404 no pagina no encontrada
{
  servidor.send(404, "text/plain", "404");
}

//------------------------------------------------------------------------------------

//-------------------------setup y loop -----------------------------------------------
void setup()
{
  // Configuración del LED
  pinMode(LED_D1, OUTPUT);
  pinMode(LED_D2, OUTPUT);
  // Configurar como un access point
  WiFi.softAP(NOMBRE_RED, CLAVE_RED);
  WiFi.softAPConfig(ip, puertaDeEnlace, mascaraDeRed);
  delay(100);

  // Configuración de las rutas de solicitud
  servidor.on("/", rutaRaiz);
  servidor.on("/encender", rutaEncenderLed);
  servidor.on("/apagar", rutaApagarLed);
  servidor.on("/encender2", rutaEncenderLed2);
  servidor.on("/apagar2", rutaApagarLed2);
  servidor.onNotFound(rutaNoEncontrada);
  // Empezar a escuchar
  servidor.begin();
}

// En el loop manejamos al cliente conectado y eso es todo ;)
void loop()
{
  servidor.handleClient();//se escucha el servidor en todo momento 
}
//------------------------------------------------------------------------------------------
