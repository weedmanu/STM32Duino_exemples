/***** RECEPTEUR (STM32WB55 LORAE5 OLED128x64) *****/
                      /****** Ajout des librairies nécessaires ******/

# include <Arduino.h>
#include<SoftwareSerial.h>         // pour ouvrir une deuxième com série
#include <Wire.h>                  // pour l'I2C
#include <Adafruit_GFX.h>          // pour l'écran oled
#include <Adafruit_SSD1306.h>      // pour l'écran oled
#include "logo.h"                  // les logos (fait pour OLED 128x64) 

                      /****** Déclaration des constantes, variables et objets ******/
// pour notre écran OLED
#define SCREEN_WIDTH    128        // OLED largeur en pixel
#define SCREEN_HEIGHT   64         // OLED hauteur en pixel
#define OLED_RESET      -1         // GPIO ou -1 si vous partagez la broche de réinitialisation Arduino)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// pour communiquer avec entre la WB55 et le LoraE5
SoftwareSerial LoraE5(A0, A1);       // on ouvre une 2ème com série sur A0 A1 (on y branche le loraE RX TX)
String inputString, message = "";    // pour la réception global et pour l'extraction du message
bool stringComplete = false;         // un flag pour savoir quand le message reçu est complet

char recv_buf[512];                 // un buffer de 512 places    
bool is_exist = false;              // un flag pour valider que le module LoraE5 répond

                    /****** Déclaration des fonctions ******/
// fonction qui efface l'écran et l'affiche vide
void clear_display() {
  display.clearDisplay();            // on efface l'écran
  display.display();                 // on affiche l'écran vide
}

// fonction qui affiche le logo st choisi à l'écran 5s puis affiche le logo coeur 
void logos(int ID) {
  display.clearDisplay();             // on efface l'écran 
  // si ID vaut 0 on dessine le logo coeur (on calcul pour centrer sur x)
  if (ID == 0) { display.drawBitmap((SCREEN_WIDTH-LOGO_COEUR_WIDTH)/2, 0, COEUR, LOGO_COEUR_WIDTH, LOGO_COEUR_HEIGHT, WHITE);} 
  // si ID vaut 0 on dessine le 1er logo ST 
  else if (ID == 1) { display.drawBitmap((SCREEN_WIDTH-LOGO_ST_WIDTH)/2, 0, LOGO_ST, LOGO_ST_WIDTH, LOGO_ST_HEIGHT, WHITE);}
  // sinon (ID vaut 2) on dessine le 2ème logo ST 
  else         {display.drawBitmap(0, 0, LOGO_STM32, LOGO_STM32_WIDTH, LOGO_STM32_HEIGHT, WHITE);}
  display.display();                  // on affiche le logo à l'écran
}

              
// fonction qui sert a envoyer des commandes AT au module LoraE5 et retourne la réponse du module
int commande_AT_LoraE5(char *check_reponse, int timeout_ms, String at_commande) {
  int temps, nouveau_byte, place = 0;                             // on déclare 3 variables de type int 
  memset(recv_buf, 0, sizeof(recv_buf));                          // on rempli le buffer de 0
  Serial.print(at_commande);                                      // on affiche la commande AT qu'on envoie au module LoraE5
  LoraE5.print(at_commande);                                      // on envoie la commande au module LoraE5
  delay(100);                                                     // on attend un petit peu
  temps = millis();                                               // on récupère le temps du compteur millis
  if (check_reponse == NULL) { return 0; }                        // si le paramètre check_reponse est null on retourne 0 
  do {                                                            // on fait (tant qu'on est pas au timout ou qu'on ne retourne pas 1)
      while (LoraE5.available() > 0) {                              // pour chaque byte qui arrive dans la com série avec le module LoraE5
          nouveau_byte = LoraE5.read();                               // on le récupère
          recv_buf[place++] = nouveau_byte;                           // on se décale dans le buffer et remplace le 0 par le nouveau byte 
          Serial.print((char)nouveau_byte);                           // on affiche dans la com série le byte reçu sous forme de char
          delay(2);                                                   // on attend un petit peu
      }
      if (strstr(recv_buf, check_reponse) != NULL) { return 1; }    // si recv_buf reçu correspond à check_reponse on retourne 1
  } while (millis() - temps < timeout_ms);                        // tant qu'on est pas au timout  
  return 0;                                                       // si on est au timout on retourn 0
}

// fonction pour extraire les datas du message et agir en fonction
void decode_message(char delimiter, String msg){      // deux paramètres: le délimiteur et le message a décoder
  int i1 = msg.indexOf(delimiter);                    // on récupère la place du premier délimiteur dans la string
  String nom = msg.substring(0, i1);                  // on extrait le contenu du début de la string au premier délimiteur (le nom)  
  if (nom == "LOGO") {                           // si le nom est LOGO ( msg sous forme: LOGO|0| ou LOGO|1| )
    int i2 = msg.indexOf(delimiter, i1+1);            // on récupère la place du deuxième délimiteur dans la string
    int choix = (msg.substring(i1 + 1, i2)).toInt();  // on extrait le contenu entre le premier et le deuxième délimiteur (0 ou 1) converti en int
    logos(choix);                                   // on affiche le logo correspondant au choix sur l'écran 
  }
}

// la fonction qui attend et traite les messages reçus
int attend_msg(void) {
  int nouveau_byte, place = 0;                                            // on déclare 3 variables de type int 
  memset(recv_buf, 0, sizeof(recv_buf));                                  // on rempli le buffer de 0
  while (LoraE5.available() > 0) {                                        // pour chaque byte qui arrive dans la com série avec le module LoraE5
    nouveau_byte = LoraE5.read();                                           // on le récupère
    recv_buf[place++] = nouveau_byte;                                       // on se décale dans le buffer et remplace le 0 par le nouveau byte 
    Serial.print((char)nouveau_byte);                                       // on affiche dans la com série le byte reçu sous forme de char
    delay(2);                                                               // on attend un petit peu
  }
  if (place) {                                                             // si place > 0, c'est qu'un message est arrivé
    String inputString = String(recv_buf);                                   // on recupère le buffer dans une string
    int first_place = inputString.indexOf('"');                              // on cherche la place du premier guillemet dans la string
    int second_place = inputString.lastIndexOf('"');                         // on cherche la place du dernier guillemet dans la string
    String msgHEX = inputString.substring(first_place + 1, second_place);    // on extrait ce qui est entre guillemet dans la string
    Serial.println(msgHEX);                                                  // on affiche dans la com série le message extrait (il est toujours en hexadécimale)
    String message = hexToString(msgHEX);                                    // on convertie en ASCII le message et le place dans une string    
    Serial.println(message);                                                 // on affiche dans la com série le message extrait en ASCII
    decode_message('|', message);
    return 1;                                                                // on retourne 1
  }
  return 0;                                                                // sinon on retourne 0
}

// fonction pour convertir une string en hexadécimale vers une string en ASCII
String hexToString(String hex_text) {                  
  String ascii_text = "";                                     // on déclare une string vide pour réceptionner les caractères convertis
  for (int i = 0; i < hex_text.length(); i++) {               // pour chaque caractère du message à convertir :
    if (i % 2 != 0) {                                           // si la place de ce caractère dans le message est impaire :
      char temp[3];                                               // on créé un tableau vide de 3 (FF = 255)
      sprintf(temp, "%c%c", hex_text[i - 1], hex_text[i]);        // on y place le caractère précédent et ce caractère
      int number = (int)strtol(temp, NULL, 16);                   // on converti cette partie héxadécimale en int
      ascii_text += char(number);                                 // on ajoute dans notre string de réception le char correspondant
    }
  }
  return ascii_text;                                          // on retourne notre string de réception en ASCII
}

void setup(void) {                
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);                                      // initialise l'écran, adresse I2C : 0x3C 
  display.clearDisplay();                                                         // on efface l'écran   
  logos(0);delay(500);logos(1);       
  delay(1500);
  logos(0);delay(500);logos(2); 
  delay(1500);      
  clear_display();                
  Serial.begin(9600);                                                                             // on démarre la com série avec le pc
  LoraE5.begin(9600);                                                                             // on démarre la com série avec le module LoraE5
  Serial.println("****  Recepteur STM32WB55 LoraE5 ****");                                        // on écrit dans la com série avec le pc pour infos   
  Serial.println("Configuration du module LoraE5 en mode récéption");                             // on écrit dans la com série avec le pc pour infos        
  if (commande_AT_LoraE5("+AT: OK", 1500, "AT\r\n")) {                                            // si la fonction commande_AT_LoraE5 retourne 1 :
    is_exist = true;
    delay(500);                                                                                     // on attend un petit peu
    commande_AT_LoraE5("+MODE: TEST", 1500, "AT+MODE=TEST\r\n");                                    // on passe le module LoraE5 en mode test
    delay(500);                                                                                     // on attend un petit peu
    commande_AT_LoraE5("+TEST: RFCFG", 1500, "AT+TEST=RFCFG,866,SF12,125,12,15,14,ON,OFF,OFF\r\n"); // on configure la partie RF du module LoraE5
    delay(500);                                                                                     // on attend un petit peu      
    commande_AT_LoraE5("+TEST: RXLRPKT", 1500, "AT+TEST=RXLRPKT\r\n");                              // on passe le module LoraE5 en mode récéption
    delay(500);                                                                                     // on attend un petit peu 
    Serial.println();                                                                               // on espace d'une ligne       
    Serial.println("Module LoraE5 en mode récéption");                                              // on écrit dans la com série avec le pc pour infos 
  } else {                                                                                        // sinon (la fonction commande_AT_LoraE5 retourne 0) :
    is_exist = false;                                                                               // le module LoraE5 est non présent ou défaillant 
    Serial.println("Aucun module LoraE5 de trouvé");                                                // on écrit dans la com série avec le pc pour infos
  }
  Serial.println();
}

void loop(void) {
  if (is_exist)  {                                                      // si le module LoraE5 est présent et fonctionnel :
    if (attend_msg()) {                                                   // si la fonction attend_msg retourne 1
      Serial.println("**** réception, convertion et traitement ok ****");   // on écrit dans la com série
      Serial.println();                                                     // on espace d'une ligne 
    }
  }
}
