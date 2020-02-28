/*
 * Lolin wemos D1 R2 & mini
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "FS.h"

//-------------------VARIABLES GLOBALES--------------------------
int contconexion = 0;
int led1=12;
int led2=15;

const char *ssid = "TeleCentro-39b0";
const char *password = "FWMMHDNMJDZ5";
char   SERVER[50]   = "tailor.cloudmqtt.com"; //"m11.cloudmqtt.com"
int    SERVERPORT   = 12498;
String USERNAME = "xoifzhmn";   
char   PASSWORD[50] = "ef5utxHyF74A";     

unsigned long previousMillis = 0;

char charPulsador [15];
String strPulsador;
String strPulsadorUltimo;

char PLACA[50];

char valueStr[15];
String strtemp = "";
char TEMPERATURA[50];
char PULSADOR[50];
char SALIDADIGITAL[50];
char RELAY1[50];
char RELAY2[50];
char ESTADO[50];
char SALIDAANALOGICA[50];
int x=0;
int valorA;



/////////////////////////////////////////////
///////////////// SPIFFS ////////////////////
/////////////////////////////////////////////

//---------------------LoadConfig----------------------------------
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }
  const char* Relay1 = doc["Relay1"];
  const char* Relay2 = doc["Relay2"];
  String SalidaAnalogica = doc["salidaAnalogica"];

/*Serial.println("*****");
Serial.println(SalidaAnalogica);
Serial.println("*****");
*/


  if((String)Relay1=="ON"){
    digitalWrite(led1, HIGH);
    }
  else{
    digitalWrite(led1, LOW);
    }
    if((String)Relay2=="ON"){
    digitalWrite(led2, HIGH);
    }
  else{
    digitalWrite(led2, LOW);
    }

  analogWrite(13,SalidaAnalogica.toInt());
  
  Serial.print("Estado Relay1: ");
  Serial.println(Relay1);
  Serial.print("Estado Relay2: ");
  Serial.println(Relay2);
  Serial.print("Salida Analogica: ");
  Serial.println(SalidaAnalogica);
  return true;
}

//----------------------------SaveConfig----------------------------
bool saveConfig() {
  StaticJsonDocument<200> doc;
  
  doc["Relay1"] = "OFF";
  doc["Relay2"] = "OFF";
  doc["salidaAnalogica"]=0;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}



/////////////////////////////////////////////
/////////////////   MQTT  ///////////////////
/////////////////////////////////////////////



//-------------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

//------------------------CALLBACK-----------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("**");
  
  char PAYLOAD[5] = "    ";
  
  Serial.print("Mensaje Recibido: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    PAYLOAD[i] = (char)payload[i];
  }
  Serial.println(PAYLOAD);
  
   if (String(topic) ==  String(ESTADO)){
   //client.publish(RELAY1, "OFF");
   }

  if (String(topic) ==  String(RELAY1)) {
    if (payload[1] == 'N'){
     digitalWrite(12, HIGH);
    }
    if (payload[1] == 'F'){
      digitalWrite(12, LOW);
    }
  }
  if (String(topic) ==  String(RELAY2)) {
    if (payload[1] == 'N'){
     digitalWrite(15, HIGH);
    }
    if (payload[1] == 'F'){
      digitalWrite(15, LOW);
    }
  }

  if (String(topic) ==  String(SALIDAANALOGICA)) {
    analogWrite(13, String(PAYLOAD).toInt());
    valorA=String(PAYLOAD).toInt();
  }

  String accion=String(topic).substring(10);
  guardar();
}

//------------------------RECONNECT-----------------------------
void reconnect() {
  uint8_t retries = 3;
  // Loop hasta que estamos conectados
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");
    // Crea un ID de cliente al azar
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    USERNAME.toCharArray(PLACA, 50);
    if (client.connect("", PLACA, PASSWORD)) {
      Serial.println("conectado");
      client.subscribe(RELAY1);
      client.subscribe(RELAY2);
      client.subscribe(ESTADO);
      client.subscribe(SALIDAANALOGICA);
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intenta nuevamente en 5 segundos");
      // espera 5 segundos antes de reintentar
      delay(5000);
    }
    retries--;
    if (retries == 0) {
      // esperar a que el WDT lo reinicie
      while (1);
    }
  }
}

//------------------------SETUP-----------------------------
void setup() {

  //prepara GPI13 y 12 como salidas 
  pinMode(13, OUTPUT); // D7 salida analógica
  analogWrite(13, 0); // analogWrite(pin, value);
  pinMode(12, OUTPUT); // D6 salida digital
  //digitalWrite(12, LOW);
  pinMode(15, OUTPUT); // D6 salida digital
  //digitalWrite(15, LOW);

  // Entradas
  pinMode(14, INPUT); // D5

  // Inicia Serial
  Serial.begin(115200);
  Serial.println("");

  // Conexión WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && contconexion <50) { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      //para usar con ip fija
     /* IPAddress ip(192,168,0,156); 
      IPAddress gateway(192,168,1,1); 
      IPAddress subnet(255,255,255,0); 
      WiFi.config(ip, gateway, subnet); */
      
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }
  
  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);

  String temperatura = "/" + USERNAME + "/" + "temperatura"; 
  temperatura.toCharArray(TEMPERATURA, 50);
  String pulsador = "/" + USERNAME + "/" + "pulsador"; 
  pulsador.toCharArray(PULSADOR, 50);
 // String salidaDigital = "/" + USERNAME + "/" + "salidaDigital"; 
 // salidaDigital.toCharArray(SALIDADIGITAL, 50);
  String Relay1 = "/" + USERNAME + "/" + "Relay1"; 
  Relay1.toCharArray(RELAY1, 50);
  String Relay2 = "/" + USERNAME + "/" + "Relay2"; 
  Relay2.toCharArray(RELAY2, 50);
  String salidaAnalogica = "/" + USERNAME + "/" + "salidaAnalogica"; 
  salidaAnalogica.toCharArray(SALIDAANALOGICA, 50);

  String Estado = "/" + USERNAME + "/" + "Estado"; 
  Estado.toCharArray(ESTADO, 50);

//-------------------SPIFFS---------------

Serial.println("Mounting FS...");

if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

}

//--------------------------LOOP--------------------------------
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if(x==0){
   loadConfig();
    x++;
  }
    
    
 /* if (currentMillis - previousMillis >= 10000) { //envia la temperatura cada 10 segundos
    previousMillis = currentMillis;
    int analog = analogRead(17);
    float temp = analog*0.322265625;
    strtemp = String(temp, 1); //1 decimal
    strtemp.toCharArray(valueStr, 15);
    Serial.println("Enviando: [" +  String(TEMPERATURA) + "] " + strtemp);
    client.publish(TEMPERATURA, valueStr);
  }*/
  
  if (digitalRead(14) == 0) {
    strPulsador = "presionado";
  } else {
    strPulsador = "NO presionado";
  }

  if (strPulsador != strPulsadorUltimo) { //envia el estado del pulsador solamente cuando cambia.
    strPulsadorUltimo = strPulsador;
    strPulsador.toCharArray(valueStr, 15);
    Serial.println("Enviando: [" +  String(PULSADOR) + "] " + strPulsador);
    client.publish(PULSADOR, valueStr);
  }
}

void guardar(){

int estado1=digitalRead(led1);
int estado2=digitalRead(led2);



  
  StaticJsonDocument<200> doc;
  
 if(estado1==1 && estado2==1){
  doc["Relay1"] = "ON";
  doc["Relay2"] = "ON";
  }
 else if(estado1==1 && estado2==0){
  doc["Relay1"] = "ON";
  doc["Relay2"] = "OFF";
  }
 else if(estado1==0 && estado2==1){
  doc["Relay1"] = "OFF";
  doc["Relay2"] = "ON";
  }
  else{
  doc["Relay1"] = "OFF";
  doc["Relay2"] = "OFF";
  }
  doc["salidaAnalogica"]=(String)valorA;
  File configFile = SPIFFS.open("/config.json", "w");
  serializeJson(doc, configFile);
  
  loadConfig();
  }

  

  
