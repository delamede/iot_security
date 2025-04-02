````
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <OneWire.h>
#include <DallasTemperature.h>

using namespace httpsserver;

// Informations WiFi
const char* ssid = "cocobox";
const char* password = "12345678";

// Definition des broches des LEDs
const int rouge = 23;    // LED rouge
const int jaune = 22;    // LED jaune
const int vert = 1;      // LED verte
const int ONEWIREBUS = 3;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONEWIREBUS);
DallasTemperature sensors(&oneWire);

// Déclaration de server comme variable globale
HTTPSServer *server;

// Gestionnaire pour la page d'affichage uniquement
void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Lire la température
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  

  
  String html = "<html>";
  html += "<head>";
  html += "<title>Affichage de Temperature Securise</title>";
  html += "<meta http-equiv=\"refresh\" content=\"1\">";
  html += "</head>";
  html += "<body>";
  html += "<h1>Temperature en temps reel</h1>" + String(temperature) + " Celsius";
  html += "</body></html>";
  
  res->print(html.c_str());
}

void setup() {
  Serial.begin(115200);
  
  // Configuration des broches des LEDs
  pinMode(rouge, OUTPUT);
  pinMode(jaune, OUTPUT);
  pinMode(vert, OUTPUT);
  digitalWrite(rouge, LOW);
  digitalWrite(jaune, LOW);
  digitalWrite(vert, LOW);

  // Initialisation du capteur de température
  sensors.begin();

  // Connexion au WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au WiFi !");
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
  
  // Afficher les informations sur la mémoire
  Serial.println("Mémoire libre : " + String(ESP.getFreeHeap()) + " octets");

  // Créer un nouveau certificat auto-signé avec une clé RSA 1024 bits
  SSLCert *cert = new SSLCert();
  int createCertResult = createSelfSignedCert(
    *cert,
    KEYSIZE_1024,
    "CN=ESP32,O=ESPRESSIF,C=FR",
    "20230101000000",
    "20330101000000"
  );
  
  if (createCertResult != 0) {
    Serial.printf("Erreur lors de la création du certificat: %d\n", createCertResult);
    return;
  }
  
  Serial.println("Certificat créé avec succès");
  
  // Créer le serveur avec le certificat
  server = new HTTPSServer(cert);
  
  // Configuration des routes du serveur - uniquement la page d'accueil
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  server->registerNode(nodeRoot);
  
  // Démarrer le serveur
  server->start();
  if (server->isRunning()) {
    Serial.println("Serveur HTTPS démarré sur le port 443");
    Serial.println("Mémoire libre après démarrage : " + String(ESP.getFreeHeap()) + " octets");
  } else {
    Serial.println("Échec du démarrage du serveur HTTPS");
    Serial.println("Vérifier la mémoire disponible ou le certificat");
  }
}

void loop() {
  // Lecture de la température et contrôle des LEDs indépendamment du serveur web
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  
  // Contrôle des LEDs en fonction de la température
  if (temperature < 28) {
    digitalWrite(vert, HIGH);
    digitalWrite(jaune, LOW);
    digitalWrite(rouge, LOW);
  } else if (temperature >= 28 && temperature <= 29) {
    digitalWrite(vert, LOW);
    digitalWrite(jaune, HIGH);
    digitalWrite(rouge, LOW);
  } else {
    digitalWrite(vert, LOW);
    digitalWrite(jaune, LOW);
    digitalWrite(rouge, HIGH);
  }
  
  // Nécessaire pour le fonctionnement du serveur HTTPS
  server->loop();
  

}
````