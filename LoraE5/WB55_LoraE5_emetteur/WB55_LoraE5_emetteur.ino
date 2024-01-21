/***** EMETTEUR (STM32WB55 LORAE5 DHT21) *****/
                      /****** Ajout des librairies nécessaires ******/
#include<SoftwareSerial.h>              // pour ouvrir une deuxième com série
#include "DHT.h"                        // pour lire les sondes DHT

                      /****** Déclaration des constantes, variables et objets ******/
// les pins des boutons et des LEDS de la WB55
const byte sw[] = { PC4, PD0, PD1 };            // SW1, SW2, SW3     (STM32 WB55)
const int led[] = { LED1, LED2, LED3 };         // LED1, LED2, LED3  (STM32 WB55)

// les différents états des LEDS et boutons
int ledState[] = {LOW, LOW, LOW};               // l'état des LEDS
int buttonState[] = {HIGH, HIGH, HIGH};         // l'état des boutons
int lastButtonState[] = {HIGH, HIGH, HIGH};     // l'ancien état des boutons

// pour la sensibilité des boutons 
unsigned long lastDebounceTime = 0;        // la dernière fois que la broche de sortie a été basculée
const unsigned long debounceDelay = 100;   // le temps de rebond (la sensibilité du bouton)

// pour définir la sonde DHT
#define DHTPIN  D3                  // on définie le GPIO utilisé par la sonde DHT
#define DHTTYPE DHT21               // on définie le type de DHT (DHT11, DHT22 ou DHT21)
DHT dht(DHTPIN, DHTTYPE);           // on déclare notre sonde DHT

// pour communiquer avec entre la WB55 et le LoraE5
SoftwareSerial LoraE5(A0, A1);      // on ouvre une 2ème com série sur A0 A1 (on y branche le loraE RX TX)

                      /****** Déclaration des fonctions ******/
// fonction qui sert a envoyer des commandes AT au module LoraE5
// et retourne la réponse du module
void comande_AT_LoraE5(char *commande) {     
  static char recv_buf[512];                 // on réserve un buffer pour stocker les bytes qui arrivent
  static int caractere;                      // pour stocker chaque byte 
  static int index = 0;                      // la place dans le buffer
  memset(recv_buf, 0, sizeof(recv_buf));     // on stock en mémoire le buffer
  LoraE5.print(commande);                    // on envoie la commande au module LoraE5 
  Serial.print(commande);                    // on écrit la commande dans la com série avec le pc pour infos
  delay(100);                                // un petit délais
  while (LoraE5.available() > 0) {           // quand quelque chose arrive du LoraE5
    caractere = LoraE5.read();               // on le place dans une variable 
    recv_buf[index++] = caractere;           // on ajoute au buffer cette variable  
    Serial.print((char)caractere);           // on écrit cette variable en char dans la com série avec le pc pour infos
    delay(2);                                // un petit délais
    if ((char)caractere == '\n') {           // si le caractère est un retour à la ligne
      delay(1000);                           // un petit délais
      break;                                 // on sort de la boucle
    }
  }
  Serial.println();                          // une ligne vide dans la com série avec le pc pour espacer
}

// fonction qui envoie un message
void send_msg(String message) {
  String commande = "AT+TEST=TXLRSTR,\"" + message + "\"\r\n";  // on construit la commande
  Serial.print(commande);                                       // on écrit la commande dans la com série avec le pc pour infos
  LoraE5.print(commande);                                       // on envoie la commande au module LoraE5 
}

// fonction qui lit la sonde DHT en envoie les datas
void read_dht() {
  String datas = "";                                            // on déclare une string vide pour construire notre message
  float humi = dht.readHumidity();                              // on lit et stock la température dans une variable
  float temp = dht.readTemperature();                           // on lit et stock l'humidité dans une variable
  if (isnan(humi) || isnan(temp)) {                             // si la lecture de la sonde échoue
    datas = "DHT|erreur|erreur|";                               // on met en forme notre message d'erreur
  } else {                                                      // sinon
    datas = "DHT|" + String(temp) + "|" + String(humi) + "|";   // on met en forme notre message avec les datas
  }  
  send_msg(datas);                                              // on envoie les datas avec la fonction send_msg
}


                      /****** setup ******/
void setup() {
  Serial.begin(9600);                         // on initialise la com série avec le pc à 9600 baud           
  LoraE5.begin(9600);                         // on initialise la com série avec le module LoraE5 à 9600 baud
  dht.begin();                                // on initialise la sonde DHT
  Serial.print("LoraE5 config\r\n");          // on écrit dans la com série avec le pc pour infos
  comande_AT_LoraE5("AT\r\n");                // on lance la commande AT avec la fonction comande_at_lora5
  delay(200);                                 // un petit délais 
  comande_AT_LoraE5("AT+MODE=TEST\r\n");      // on passe en mode test avec la fonction comande_at_lora5
  delay(200);                                 // un petit délais 
  // on lance la commande de config: Fréquence, mode, bande passante, puissance RX, puissance TX,
  // CRC (contrôle le nombre de byte envoyés/reçus), IQ (inversé), réseau public/privé (off/on)   
  comande_AT_LoraE5("AT+TEST=RFCFG,866,SF12,125,12,12,15,ON,OFF,OFF\r\n");
  Serial.println("LoraE5 en mode émission");  // on écrit dans la com série avec le pc pour infos
  for (int i=0; i < sizeof(sw); i++){         // pour chaque boutons et LEDS
    pinMode(sw[i], INPUT_PULLUP);             // on met les boutons en mode INPUT
    pinMode(led[i], OUTPUT);                  // on met les LEDS en mode OUPUT
  }
}

                      /****** loop ******/
void loop() {
  for (int i=0; i<sizeof(sw); i++) {                        // pour chaque boutons et LEDS
    int reading = digitalRead(sw[i]);                       // on lit l'état du bouton
    if (reading != lastButtonState[i]) {                    // si la lecture est différente de l'ancien état
      Serial.println(reading);                              // on écrit la lecture dans la com série avec le pc pour infos
      lastDebounceTime = millis();                          // on active le temps du rebond 
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {    // après le temps du rebond
      if (reading != buttonState[i]) {                      // si la lecture est différente de l'état du bouton
        buttonState[i] = reading;                           // l'état du bouton prend la valeur de la lecture
        if (buttonState[i] == HIGH) {                       // si l'état est HIGH
          ledState[i] = !ledState[i];                       // on change l'état de la LED
          digitalWrite(led[i], ledState[i]);                // on met la LED dans le nouvel état (allumé)
          if (i == 0) {send_msg("LOGO|0|");}                // si on à appuyer sur le SW1, on envoi le msg qui demande le logo 0
          if (i == 1) {send_msg("LOGO|1|");}                // si on à appuyer sur le SW2, on envoi le msg qui demande le logo 1
          if (i == 2) {read_dht();}                         // si on à appuyer sur le SW2, on envoi le msg qui lit la sonde et envoi les datas
          ledState[i] = !ledState[i];                       // on remet l'état de la LED dans sont état initial
        }
      }
    }
    digitalWrite(led[i], ledState[i]);                      // on met la LED dans le nouvel état (éteint)
    lastButtonState[i] = reading;                           // l'ancien état du bouton prend la valeur de la lecture
  }
}
