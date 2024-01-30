### Communication entre 2 cartes STM32WB55 avec des modules LoraE5

## Une partie émetteur et une partie récepteur

**Pour la partie émetteur** : on utilise les boutons de la STM32WB55 pour envoyer des instructions à la partie récepteur, le LoraE5 est connecté comme suit :

| STM32 | LoraE5 | 
| :---: | :----: | 
| GND   | GND    |
| 3.3V  | VCC    | 
| A0    | TX     |
| A1    | RX     |

On utilise la librairie **SoftwareSerial** pour ouvrir une 2ème com série. 

**Pour la partie Récepteur** : le LoraE5 est connecté comme suit :

| STM32 | LoraE5 | 
| :---: | :----: | 
| GND   | GND    |
| 3.3V  | VCC    | 
| A0    | TX     |
| A1    | RX     |

On utilise la librairie **SoftwareSerial** pour ouvrir une 2ème com série. 

et l'écran OLED 128x64 I2C est branché comme suit :

| STM32 | OLED   | 
| :---: | :----: | 
| GND   | GND    |
| 3.3V  | VCC    | 
| A4    | SDA    |
| A5    | SCL    |
