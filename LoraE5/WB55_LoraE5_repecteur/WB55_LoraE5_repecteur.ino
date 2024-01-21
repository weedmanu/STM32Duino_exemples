#include <Arduino.h>
#include<SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "logo.h"                           // logos fait pour OLED 128x64 

#define SCREEN_WIDTH    128                 // OLED largeur en pixel
#define SCREEN_HEIGHT   64                  // OLED hauteur en pixel
#define OLED_RESET      -1                  // GPIO ou -1 si vous partagez la broche de réinitialisation Arduino)

// on déclare notre écran OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SoftwareSerial LoraE5(A0, A1);              // on ouvre une 2ème com série sur A0 A1 (on y branche le loraE RX TX)
String inputString, message = "";           // pour la réception global et pour l'extraction du message
bool stringComplete = false;                // un flag pour savoir quand le message reçu est complet

// fonction qui efface l'écran et l'affiche vide
void clear_display() {
  display.clearDisplay();
  display.display();  
}

// fonction qui affiche le logo coeur à l'écran
void love(void) {
  display.clearDisplay(); // On efface l'écran   
  display.drawBitmap((SCREEN_WIDTH-LOGO_COEUR_WIDTH)/2, 0, COEUR, LOGO_COEUR_WIDTH, LOGO_COEUR_HEIGHT, WHITE); 
  display.display();
}

// fonction qui affiche le logo st choisi à l'écran 5s puis affiche le logo coeur 
void logo_st(int ID) {
  display.clearDisplay(); // On efface l'écran 
  if (ID == 0) { display.drawBitmap((SCREEN_WIDTH-LOGO_ST_WIDTH)/2, 0, LOGO_ST, LOGO_ST_WIDTH, LOGO_ST_HEIGHT, WHITE);}
  else         {display.drawBitmap(0, 0, LOGO_STM32, LOGO_STM32_WIDTH, LOGO_STM32_HEIGHT, WHITE);}
  display.display();
  delay(5000);
  clear_display();
  love();
}

// fonction qui affiche les datas de la DHT à l'écran 5s puis affiche le logo coeur
void display_dht(String temperature, String humidity) {
  display.clearDisplay();
  display.setTextSize(2);      
  display.setTextColor(SSD1306_WHITE); 
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setCursor(0, 0);
  display.print(temperature);
  display.print(" ");
  display.write(248);
  display.println("C");
  display.print(humidity);
  display.println(" %");
  display.display();
  delay(5000);
  clear_display();
  love();
}
/*
// fonction qui sert a envoyer des commandes AT au module Lora
// et retourne la reponse du module
void comande_AT_LoraE5(int timeout_ms, char *commande) {
  static char recv_buf[512];
  static int caractere;
  static int index = 0;
  static int startMillis = 0;
  memset(recv_buf, 0, sizeof(recv_buf));
  LoraE5.print(commande);
  Serial.print(commande);
  delay(100);
  startMillis = millis();
  do {
    while (LoraE5.available() > 0) {
      caractere = LoraE5.read();
      recv_buf[index++] = caractere;
      Serial.print((char)caractere);
      delay(2);
    }
  } while (millis() - startMillis < timeout_ms);
  Serial.println();
}*/

// fonction qui sert a envoyer des commandes AT au module Lora
// et retourne la reponse du module
void comande_AT_LoraE5(int timeout_ms, char *commande) {
  static char recv_buf[512];
  static int caractere;
  static int index = 0;
  static int startMillis = 0;
  memset(recv_buf, 0, sizeof(recv_buf));
  LoraE5.print(commande);
  Serial.print(commande);
  delay(100);
  while (LoraE5.available() > 0) {
    caractere = LoraE5.read();
    recv_buf[index++] = caractere;
    Serial.print((char)caractere);
    delay(2);
    if ((char)caractere == '\n') {
      delay(1000);
      break;
    }
  }
  Serial.println();
}

// fonction pour convertir une string en Hexa en string ASCII
String hexToString(String hex){                   // un paramètre: le message en hex a convertir
  String text = "";                               // on déclare une string vide pour réceptionner les caractères convertis
  for(int k=0;k< hex.length();k++) {              // pour chaque caractère du message :
    if(k%2!=0) {                                    // si la place de ce caractère dans le message est impaire
      char temp[3];                                 // on créé un tableau vide de 3
      sprintf(temp,"%c%c",hex[k-1],hex[k]);         // on y place le caractère précédent et ce caractère 
      int number = (int)strtol(temp, NULL, 16);     // on converti cette partie de hexa en int    
      text+=char(number);                           // on ajoute dans notre string de réception le char correspondant 
    }  
  }  
  return text;                                    // on retourne notre string de réception
}

// fonction pour extraire les datas du message et agir en fonction
void decode_message(char delimiter, String msg){      // deux paramètres: le délimiteur et le message a décoder
  int i1 = msg.indexOf(delimiter);                    // on récupère la place du premier délimiteur dans la string
  String nom = msg.substring(0, i1);                  // on extrait le contenu du début de la string au premier délimiteur (le nom)
  if (nom == "DHT") {                                 // si le nom est DHT ( msg sou forme: DHT|21.54|50.36| )
    int i2 = msg.indexOf(delimiter, i1+1);            // on récupère la place du deuxième délimiteur dans la string
    int i3 = msg.indexOf(delimiter, i2+1);            // on récupère la place du troisième délimiteur dans la string
    String temp = msg.substring(i1 + 1, i2);          // on extrait le contenu entre le premier et le deuxième délimiteur (la tempèrature)
    String humi = msg.substring(i2 + 1, i3);          // on extrait le contenu entre le deuxième et le troisième délimiteur (l'humidité)
    display_dht(temp, humi);                          // on affiche les datas sur l'écran
  }  
  else if (nom == "LOGO") {                           // si le nom est LOGO ( msg sous forme: LOGO|0| ou LOGO|1| )
    int i2 = msg.indexOf(delimiter, i1+1);            // on récupère la place du deuxième délimiteur dans la string
    int choix = (msg.substring(i1 + 1, i2)).toInt();  // on extrait le contenu entre le premier et le deuxième délimiteur (0 ou 1) converti en int
    logo_st(choix);                                   // on affiche le logo correspondant au choix sur l'écran 
  }
}

// La fonction E5serialEvent qui sert a transmettre la com série du module LoraE5 
// à la com série de notre stm32 WB55
void E5serialEvent() {  
  while (LoraE5.available()) {              // quand un caractère arrive
    char inChar = (char)LoraE5.read();      // on le place dans une variable
    inputString += (char)inChar;            // on l'ajoute à notre string de réception
    if (inChar == '\n') {                   // si le caractère est un retour à la ligne
      stringComplete = true;                // le message est complet
    }
    if (stringComplete) {                                                     // si le message est complet
      Serial.println(inputString);                                            // on l’écrit tel quel 
      int first_index = inputString.indexOf('"');                             // on cherche la place du premier guillemet
      int second_index = inputString.lastIndexOf('"');                        // on cherche la place du dernier guillemet
      String msgHEX = inputString.substring(first_index+1, second_index);     // on extrait ce qui est entre guillemet  
      Serial.println(msgHEX);                                                 // on affiche le message extrait (il est en hexa)
      message = hexToString(msgHEX);                                          // on converti ce message en ASCII
      Serial.println(message);                                                // on affiche le message converti
      inputString = "";                                                       // on vide la string de réception
      stringComplete = false;                                                 // on rebascule le flag 
      decode_message('|', message);                                           // on manipule le message reçu
    }
  }
}

// Le setup
void setup(void) {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);                                      // initialise l'écran, adresse I2C : 0x3C 
  display.clearDisplay();                                                         // On efface l'écran   
  love();                                                                         // on lance la fonction qui affiche le logo cœur
  Serial.begin(9600);                                                             // on démarre la com série avec le pc à 9600 baud
  LoraE5.begin(9600);                                                             // on démarre la com série avec le module LoraE5 à 9600 baud
  Serial.println("LoraE5 config");                                                // on écrit dans la com série avec le pc pour infos
  comande_AT_LoraE5(1000, "AT\r\n");                                              // on lance une commande de test avec notre fonction comande_AT_LoraE5
  delay(200);                                                                     // un petit délais 
  comande_AT_LoraE5(1000, "AT+MODE=TEST\r\n");                                    // on lance la commande pour entrer en mode test
  delay(200);                                                                     // un petit délais 
  // on lance la commande de config: 
  // Fréquence, mode, bande passante, puissance RX, puissance TX, CRC (check nb de byte envoyés et reçus), IQ (inversé), réseau public/privé (off/on) 
  comande_AT_LoraE5(2000, "AT+TEST=RFCFG,866,SF12,125,12,12,15,ON,OFF,OFF\r\n");  
  comande_AT_LoraE5(2000, "AT+TEST=RXLRPKT\r\n");                                 // on lance la commande qui met le module en mode réception
  Serial.print("LoraE5 en mode réception\r\n");                                   // on écrit dans la com série avec le pc pour infos
}

// le loop
void loop(void){
  E5serialEvent();  // on écoute la com série du LoraE5 en boucle
}
