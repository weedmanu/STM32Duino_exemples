/***** EMETTEUR (STM32WB55 LORAE5) *****/
                    /****** Ajout des librairies nécessaires ******/
# include <Arduino.h>
#include <SoftwareSerial.h>              // pour ouvrir une deuxième com série

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

// pour communiquer avec entre la STM32WB55 et le LoraE5
SoftwareSerial LoraE5(A0, A1);      // on ouvre une 2ème com série sur la STM32WB55 en A0 A1 (on y branche le loraE5 RX TX)

char recv_buf[512];                 // un buffer de 512 places    
bool is_exist = false;              // un flag pour valider que le module LoraE5 répond

                    /****** Déclaration des fonctions ******/
// fonction qui sert a envoyer des commandes AT au module LoraE5 et retourne la réponse du module
int commande_AT_LoraE5(char *check_reponse, int timeout_ms, String at_commande) {
  int temps, nouveau_byte, place = 0;                             // on déclare 3 variables de type int 
  memset(recv_buf, 0, sizeof(recv_buf));                          // on rempli le buffer de 0
  Serial.print(at_commande);                                      // on affiche la commande AT qu'on envoie au module LoraE5
  LoraE5.print(at_commande);                                      // on envoie la commande au module LoraE5
  delay(100);                                                     // on attend un petit peu
  temps = millis();                                               // on récupère le temps du compteur millis
  if (check_reponse == NULL) { return 0; }                        // si le paramètre check_reponse est null on retourne 0 
  do {                                                            // on fait (tant qu'on est pas au timeout ou qu'on ne retourne pas 1)
    while (LoraE5.available() > 0) {                              // pour chaque byte qui arrive dans la com série avec le module LoraE5
      nouveau_byte = LoraE5.read();                               // on le récupère
      recv_buf[place++] = nouveau_byte;                           // on se décale dans le buffer et remplace le 0 par le nouveau byte 
      Serial.print((char)nouveau_byte);                           // on affiche dans la com série le byte reçu sous forme de char
      delay(2);                                                   // on attend un petit peu
    }
    if (strstr(recv_buf, check_reponse) != NULL) { return 1; }    // si recv_buf reçu correspond à check_reponse on retourne 1
  } while (millis() - temps < timeout_ms);                        // tant qu'on est pas au timeout  
  return 0;                                                       // si on est au timeout on retourne 0
}

// la fonction qui envoie un message
void envoie_msg(char* message) {
  Serial.println("**** envoie du message ****");        // on écrit dans la com série
  int result = 0;                                                                   // on déclare 1 variables de type int 
  char cmd[128];                                                                    // on déclare un buffer de type char de 128 places
  sprintf(cmd, "AT+TEST=TXLRSTR,\"%s\"\r\n", message);                              // on construit notre commande et la place dans le buffer
  result = commande_AT_LoraE5("+TEST: TX DONE", 2000, cmd);                         // on récupère le retour de la fonction commande_AT_LoraE5 d'envoie
  result ? Serial.println("Envoie réussi") : Serial.println("Échec de l'envoie");   // si retourne 1 c'est ok sinon c'est ko
  Serial.println();
}

                    /****** setup ******/
void setup(void) {
  Serial.begin(9600);                                                                             // on démarre la com série avec le pc
  LoraE5.begin(9600);                                                                             // on démarre la com série avec le module LoraE5
  Serial.println("****  Émetteur STM32WB55 LoraE5 ****");                                         // on écrit dans la com série avec le pc pour infos   
  Serial.println("Configuration du module LoraE5 en mode émission");                              // on écrit dans la com série avec le pc pour infos        
  if (commande_AT_LoraE5("+AT: OK", 1500, "AT\r\n")) {                                            // si la fonction commande_AT_LoraE5 retourne 1 :
    is_exist = true;                                                                                // le module LoraE5 est présent et fonctionnel
    delay(500);                                                                                     // on attend un petit peu
    commande_AT_LoraE5("+MODE: TEST", 1500, "AT+MODE=TEST\r\n");                                    // on passe le module LoraE5 en mode test
    delay(500);                                                                                     // on attend un petit peu
    commande_AT_LoraE5("+TEST: RFCFG", 1500, "AT+TEST=RFCFG,866,SF12,125,12,15,14,ON,OFF,OFF\r\n"); // on configure la partie RF du module LoraE5
    delay(500);                                                                                     // on attend un petit peu      
    Serial.println();                                                                               // on espace d'une ligne
    Serial.println("Module LoraE5 en mode émission");                                               // on écrit dans la com série avec le pc pour infos 
  } else {                                                                                        // sinon (la fonction commande_AT_LoraE5 retourne 0) :
    is_exist = false;                                                                               // le module LoraE5 est non présent ou défaillant 
    Serial.println("Aucun module LoraE5 de trouvé");                                                // on écrit dans la com série avec le pc pour infos
  }
  for (int i=0; i < sizeof(sw); i++){         // pour chaque boutons et LED
    pinMode(sw[i], INPUT_PULLUP);             // on met les boutons en mode INPUT
    pinMode(led[i], OUTPUT);                  // on met les LED en mode OUPUT
  }
  Serial.println();                                                                               // on espace d'une ligne
}

                    /****** loop ******/
void loop(void) {
  if (is_exist)  {                   // si le module LoraE5 est présent et fonctionnel :
    for (int i=0; i<sizeof(sw); i++) {                        // pour chaque boutons et LED
      int reading = digitalRead(sw[i]);                       // on lit l'état du bouton
      if (reading != lastButtonState[i]) {                    // si la lecture est différente de l'ancien état
        lastDebounceTime = millis();                          // on active le temps du rebond 
      }
      if ((millis() - lastDebounceTime) > debounceDelay) {    // après le temps du rebond
        if (reading != buttonState[i]) {                      // si la lecture est différente de l'état du bouton
          buttonState[i] = reading;                           // l'état du bouton prend la valeur de la lecture
          Serial.print("SW"); Serial.print(i+1);              // on écrit la lecture dans la com série avec le pc pour infos         
          buttonState[i] ? Serial.println(" --> relaché") : Serial.println(" --> appuyé");                    
          if (buttonState[i] == HIGH) {                       // si l'état est HIGH
            ledState[i] = !ledState[i];                       // on change l'état de la LED
            digitalWrite(led[i], ledState[i]);                // on met la LED dans le nouvel état (allumé)
            if (i == 0) {envoie_msg("LOGO|0|");}                // si on à appuyer sur le SW1, on envoi le msg qui demande le logo 0
            if (i == 1) {envoie_msg("LOGO|1|");}                // si on à appuyer sur le SW2, on envoi le msg qui demande le logo 1
            if (i == 2) {envoie_msg("LOGO|2|");}                // si on à appuyer sur le SW2, on envoi le msg qui demande le logo 2
            ledState[i] = !ledState[i];                       // on remet l'état de la LED dans sont état initial
          }
        }
      }
      digitalWrite(led[i], ledState[i]);                      // on met la LED dans le nouvel état (éteint)
      lastButtonState[i] = reading;                           // l'ancien état du bouton prend la valeur de la lecture
    }
  }
}
