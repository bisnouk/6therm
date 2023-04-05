#define TIMER_INTERRUPT_DEBUG         1
#define _TIMERINTERRUPT_LOGLEVEL_     4

// extern "C" {
#include <hardware/watchdog.h>
// };
#include <TFT_eSPI.h> 
#include <TFT_eWidget.h>
#include <SPI.h>
#include "RPi_Pico_TimerInterrupt.h"
#include "OneWire.h"
#include "DallasTemperature.h"

#include "Aclonica15.h"
#include "Aclonica45.h"
#include "Aclonica75.h"

#include "DASH_01.h"
#include "DASH_02.h"
#include "DASH_03.h"
#include "DASH_04.h"
#include "DASH_05.h"
#include "DASH_06.h"

#include "ETAT_01.h"
#include "ETAT_02.h"
#include "ETAT_03.h"
#include "ETAT_04.h"
#include "ETAT_05.h"
#include "ETAT_06.h"

////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                    Declaration des variables globales                  ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
#define REL_PIN  14
#define CMD_PIN  2
#define TEMP_PIN 1
#define RPM_PIN  4
#define PWM_PIN  6
#define VENT_PIN 7
#define X_TEMP   0 //48
#define Y_TEMP   80
#define X_CONS   65 //90
#define Y_CONS   185 //176
#define X_ETAT   60
#define Y_ETAT   5
#define X_VENT   210
#define Y_VENT   5
#define X_TEXT   110
#define Y_TEXT   5
#define X_DEBUG  50
#define Y_DEBUG  90
#define X_BA     0 //-11
#define Y_BA     0
#define X_BB     0 //-11
#define Y_BB     180
#define X_BX     260 //71
#define Y_BX     0
#define X_BY     260 //71
#define Y_BY     180
#define BLMAX    255
#define BLMIN    5
#define TVEILLE  3600
#define TCOMMAND 180 // nombre de seconde avant mise à l'arrêt de la chaudière
#define WTFT     320
#define HTFT     240

#define AA_FONT_TEXT  ACLONICA15
#define AA_FONT_CONS  ACLONICA45
#define AA_FONT_TEMP  ACLONICA75

static float          bufferTemp[10];

// rom_address_t         address{};
bool                  topSeconde = false;
uint8_t               modeArret = 0;
uint8_t               mode1     = 0;
uint16_t              tempoChaud = 0;    // Temporisation Alimentation Chaudiere
uint16_t              tempoCmdChaud = 0; // Temporisation Commande Chaudiere 
uint16_t              tempoMode = 0;     // Temporisation avant activation mode fonctionnement
uint16_t              tempoVeille = 0;   // Temporisation arret automatique en mode nuit
// volatile bool         topRpm = false;
// absolute_time_t       topRpm1;
// absolute_time_t       topRpm2;
// uint16_t              rpmFan;

float                 valTemperature;
float                 valConsigne;
int8_t                modeFonc;
int8_t                modeVentil;
int8_t                modeVeille;
uint8_t               consigneVentil;
uint8_t               sveConsigneVentil = 255;
uint8_t               cmdChaudiere;     // Commande du relais signal de mise en chauffe
uint8_t               alimChaudiere;    // Commande du relais alimentation generale 12V du chauffage
uint8_t               etatVentilateur;  // Commande du relais alimentation du ventilateur
int8_t                minVentil = 10;
int8_t                defautVentil = 0;
int                   valBL = BLMAX;    // Niveau du retroeclairation ecran
uint16_t              tempoBL = 0;      // temporisation retroeclairage ecran
uint8_t               modeDebug = 0;

int8_t                bouton;
int8_t                incrementFonc = 1;
int8_t                incrementVent = 1;
int8_t                i = 0;
int8_t                ind = 0;
static uint32_t       scanTime = millis();


TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
TFT_eSprite spt = TFT_eSprite(&tft); // Sprite class needs to be invoked
TFT_eSprite spc = TFT_eSprite(&tft); // Sprite class needs to be invoked
TFT_eSprite spl = TFT_eSprite(&tft);

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

ButtonWidget btnHG  = ButtonWidget(&tft);
ButtonWidget btnBG  = ButtonWidget(&tft);
ButtonWidget btnHD  = ButtonWidget(&tft);
ButtonWidget btnBD  = ButtonWidget(&tft);
ButtonWidget btnLog = ButtonWidget(&tft);

#define BUTTON_W 60
#define BUTTON_H 60

ButtonWidget* btn[] = {&btnHG , &btnBG, &btnHD, &btnBD, &btnLog};;
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

OneWire oneWire(TEMP_PIN);
DallasTemperature ds(&oneWire);

////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////               Routine interruption                                     ////
bool TimerHandler0(struct repeating_timer *t)
{
  (void) t;
  
  static bool toggle0 = false;

  topSeconde = true;

  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(LED_BUILTIN, toggle0);
  toggle0 = !toggle0;

  return true;
}


#define TIMER0_INTERVAL_MS        1000
RPI_PICO_Timer ITimer0(0);


////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                       Configuration initiale                           ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void setup() {



  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0);
  initialisation();

  valTemperature = lectureTemp();
  for (i = 0 ; i < 10 ; ++i) bufferTemp[i] = valTemperature;
  
  valConsigne = 18;
  consigneVentil = 50;
  modeFonc = 0; 
  modeVentil = 2;
  valBL = BLMAX;
  tempoVeille = 0;
  modeVeille = 0;

  bouton = 0;

  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  setBackLight(valBL);

  initButtons();
  renderDash(5,X_BA,Y_BA);
  renderEtat(modeFonc, X_ETAT, Y_ETAT);
}

////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                           Programme principal                          ////
////                                                                        ////
////     0.1 Version initiale                                               ////
////     0.2 Abandon de la gestion de l'alimentation de la chaudière        ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void loop() {


  if (topSeconde) {
      watchdog_update();
      bufferTemp[ind] = lectureTemp();
      ind++;
      if (ind == 10) ind = 0;
      valTemperature = 0;
      for (i = 0 ; i < 10 ; ++i) valTemperature += bufferTemp[i];
      valTemperature /= 10;
      // if (modeArret) tempoChaud++;
      if (modeFonc == 2) {
        if (modeVeille != 0) tempoVeille++;
        tempoCmdChaud++;
      }
      tempoBL++;
      tempoMode++;
      topSeconde = false;
  }
  
  // spt.loadFont(AA_FONT_TEXT); // Must load the font first into the sprite class
  
  // spt.setTextColor(TFT_YELLOW, TFT_BLACK); // Set the sprite font colour and the background colour

  // tft.setCursor(150, 10);          // Set the tft cursor position, yes tft position!
  // spt.printToSprite("mode "+(String)modeFonc + "-");    // Prints to tft cursor position, tft cursor NOT moved
  // spt.unloadFont(); // Remove the font from sprite class to recover memory used


  switch (modeFonc)
  {
  case 0:
    /* Mode STOP */ 
    // if (modeArret) {
    //   if (tempoChaud > 500) {
    //     tempoChaud = 0;
    //     alimChaudiere = 0;
    //     modeArret = 0;
    //   }
    // } else {
    //   alimChaudiere = 0;
    // }
    modeVeille = 0;
    tempoVeille = 0;
    cmdChaudiere = 0; // Arret chauffage
    etatVentilateur = 0;
    // alimentationChaudiere(alimChaudiere);
    commandeChaudiere(cmdChaudiere); 
    commandeVentilateur(etatVentilateur);
    renderTemperature(valTemperature, X_TEMP, Y_TEMP);
    break;
  case 1:
    /* Mode Eau Chaude */
    if (mode1 == 1) {
      tft.fillScreen(TFT_BLACK);
      renderDash(5,X_BA,Y_BA);
      renderEtat(modeFonc, X_ETAT, Y_ETAT);
      mode1 = 0;
      sveConsigneVentil = 255;
    }
    if (tempoMode > 5) {
      tempoMode = 100;
      // alimChaudiere = 1;
      cmdChaudiere = 1;
      etatVentilateur = 0;
      // alimentationChaudiere(alimChaudiere);
      commandeChaudiere(cmdChaudiere);           
    }
    modeVeille = 0;
    tempoVeille = 0;
    // pwmVentilateur(10); // Ventilateur à 10%
    commandeVentilateur(etatVentilateur);     
    renderTemperature(valTemperature, X_TEMP, Y_TEMP);      
    break;
  case 2:
    /* Mode Chauffage Ventilation Automatique */
    consigneVentil = regulVentil(valTemperature, valConsigne, modeVentil);  
        
    if (tempoMode > 5) {
      tempoMode = 100;
      // alimChaudiere = 1;
      // alimentationChaudiere(alimChaudiere);
      if (tempoVeille > TVEILLE) {
        tempoVeille = 0;
        modeFonc = 0;
        // modeArret = 1;
        modeVeille = 0;
      }
      if (consigneVentil == 0) {
        // etatVentilateur = 0;
        if (tempoCmdChaud > TCOMMAND) {
          tempoCmdChaud = 0;
          cmdChaudiere = 0;
        }          
      } 
      else {
        tempoCmdChaud = 0;
        cmdChaudiere = 1;
        // etatVentilateur = 1;
      }
      commandeChaudiere(cmdChaudiere);   
      etatVentilateur = asservVentilateur(valTemperature, valConsigne);             
      commandeVentilateur(etatVentilateur);  
      pwmVentilateur(consigneVentil);
    }
    renderTemperature(valTemperature, X_TEMP, Y_TEMP);
    renderConsigne(valConsigne, X_CONS, Y_CONS);
    renderVentil(consigneVentil ,X_VENT, Y_VENT);      
    renderModeVentil(modeVentil, TFT_ORANGE);
    renderDash(4,X_BX,Y_BX);
    break;
  case 3:
    /* Mode Chauffage Ventilation Manuelle */
    if (tempoMode > 5) {
      tempoMode = 100;
      // alimChaudiere = 1;
      cmdChaudiere = 1;
      etatVentilateur = 1;

      // alimentationChaudiere(alimChaudiere);
      commandeChaudiere(cmdChaudiere);  

      if (consigneVentil == 0) {
        // etatVentilateur = 0;
        if (tempoCmdChaud > TCOMMAND) {
          tempoCmdChaud = 0;
          cmdChaudiere = 0;
        }          
      } 
      else {
        tempoCmdChaud = 0;
        cmdChaudiere = 1;
        // etatVentilateur = 1;
      }
      etatVentilateur = asservVentilateur(valTemperature, valConsigne);             
      commandeVentilateur(etatVentilateur);         
      pwmVentilateur(consigneVentil);
    }
    modeVeille = 0;
    tempoVeille = 0;

    renderModeVentil(modeVentil, TFT_BLACK);
    renderTemperature(valTemperature, X_TEMP, Y_TEMP);
    renderConsigne(valConsigne, X_CONS, Y_CONS);
    renderVentil(consigneVentil , X_VENT, Y_VENT); 
    renderDash(4,X_BX,Y_BX);
    break;
  default:
    break;
  }

  uint16_t t_x = 9999, t_y = 9999; // To store the touch coordinates
  if (millis() - scanTime >= 50) {
  // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (pressed) {
        tempoBL = 0;
        if (valBL != BLMAX) {
          valBL = BLMAX;
          setBackLight(valBL);
        } 
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        if (tempoBL > 10) {
          if (tempoBL <= (BLMAX - BLMIN)) {
            valBL = BLMAX - tempoBL;
          }
          else {
            valBL = BLMIN;
            tempoBL = 255;
          }
          setBackLight(valBL);
        }
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }

  if (bouton == 1) {
    modeFonc = modeFonc + incrementFonc;
    if (modeFonc  > 2) incrementFonc = -1;
    if (modeFonc == 1) {
      incrementFonc = +1;
      mode1 = 1;
    }
    tempoMode = 0;
    renderEtat(modeFonc, X_ETAT, Y_ETAT);
  }

  if (bouton == 3) {
    switch (modeFonc)
    {
    case 2:
      /* Mode Ventilation Automatique */
      modeVentil = modeVentil + 1;
      if (modeVentil  > 4) modeVentil = 0;
      break;
    case 3:
      /* Mode Chauffage Ventilation Manuelle */
      consigneVentil = consigneVentil + (incrementVent * 20);
      if (consigneVentil >= 100) {
        incrementVent = -1;
        consigneVentil = 100;
      }
      if (consigneVentil == 0)   {
        incrementVent = +1;
        consigneVentil = 0;
      }
      renderVentil(consigneVentil, X_VENT, Y_VENT);
      break;
    }

  }
  if ((bouton == 4) && (modeFonc >= 2)) {
    valConsigne += 0.5;
    if (valConsigne > 39) valConsigne = 39;
    renderConsigne(valConsigne, X_CONS, Y_CONS);
  }
  if ((bouton == 2) && (modeFonc >=2)) {
    valConsigne -= 0.5;
    if (valConsigne < 10) valConsigne = 10;
    renderConsigne(valConsigne, X_CONS, Y_CONS);
  }
  if (modeDebug) { // Fenetre de debug

    watchdog_update();
    spl.createSprite(120, 80);
    spl.loadFont(AA_FONT_TEXT);
    spl.fillSprite(TFT_DARKGREY);
    spl.setTextColor(TFT_GREEN, TFT_DARKGREEN);
    spl.setTextDatum(TL_DATUM);

    spl.drawString("REL Chaud", 2, 2);
    spl.drawString("CMD Chaud", 2, 22);
    spl.drawString("REL Vent ", 2, 42);
  
    for (int i = 0; i < 3; i++) {
      spl.drawCircle(106, 10 + (20 * i), 9, TFT_LIGHTGREY);
      spl.drawCircle(106, 10 + (20 * i), 7, TFT_DARKGREY);
    }
    // if (alimChaudiere)   spl.drawSpot(106,  10, 7, TFT_RED, TFT_DARKGREEN);
    if (cmdChaudiere)    spl.drawSpot(106, 30, 7, TFT_RED, TFT_DARKGREEN);
    if (etatVentilateur) spl.drawSpot(106, 50, 7, TFT_RED, TFT_DARKGREEN);

    float valT = valTemperature;
    if (valT > 99.9) valT = 99.9;
    if (valT < -99.9) valT = -99.9;
    int valInt = int(valT);
    int valDec = abs((int(valT * 10) - (valInt * 10)));
 
    spl.drawString((String) "TEMP " + (valInt) + "," + (String) (valDec) + " c", 2, 62);
    
    spl.pushSprite(X_DEBUG, Y_TEMP);
    spl.unloadFont();
    spl.deleteSprite();
  }
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                              Fonctions                                 ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void initialisation(void)
{
  watchdog_enable(2500,1);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TFT_BL, OUTPUT);
 // pinMode(TEMP_PIN, INPUT);
  // pinMode(REL_PIN, OUTPUT);
  pinMode(CMD_PIN, OUTPUT);
  pinMode(VENT_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  // digitalWrite(REL_PIN, 1);
  digitalWrite(CMD_PIN, 1);
  digitalWrite(VENT_PIN, 1);


  ds.begin();
  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(1);

  spr.setColorDepth(16); // 16 bit colour needed to show anti-aliased fonts
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

  tft.fillScreen(TFT_BLACK);

  digitalWrite(LED_BUILTIN, HIGH);  
  delay(1000);                      
  digitalWrite(LED_BUILTIN, LOW);   

  watchdog_update();


  // touch_calibrate();


}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Règle le rétroéclairage de l'écran                                ////
void setBackLight(int val)
{
  analogWrite(TFT_BL, val);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Lit la température d'une sonde DS18B20                            ////
float lectureTemp(void)
{
  watchdog_update();
  ds.requestTemperaturesByIndex(0);
  return(ds.getTempCByIndex(0));
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Asservissement du ventilateur en PWM                              ////
void pwmVentilateur(int d)
{
  float valD = ((float)d /100) * 255;
  analogWrite(PWM_PIN, (int)valD);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Commande l'alimentaion du ventilateur                            ////
void commandeVentilateur(int8_t cmdVentil)
{
  digitalWrite(VENT_PIN, !cmdVentil);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Calcule l'asservissement du ventilateur 0 à 100%                  ////
int regulVentil(float temperature, float consigne, int8_t modeVent)
{
  float    k = 200;
  float  coefNorme = 1;
  float  deltaTemp;
  float  cmdVentil;

  watchdog_update();

  deltaTemp = consigne - temperature;
  cmdVentil = k * deltaTemp;
  switch (modeVent)
  {
  case 0: /* Silence */
    coefNorme = 0.4;
    modeVeille = 0;
    tempoVeille = 0;   
    break;
  case 1: /* Cool */
    coefNorme = 0.6;
    modeVeille = 0;
    tempoVeille = 0;  
    break;
  case 2: /* Normal */
    coefNorme = 0.8;
    modeVeille = 0;
    tempoVeille = 0;   
    break;
  case 3: /* Maxi */
    coefNorme = 1;
    modeVeille = 0;
    tempoVeille = 0;  
    break;
  case 4: /* 60 minutes */
    coefNorme = 0.3;
    modeVeille = 1;
    break;
  }
  if (cmdVentil > 100) cmdVentil = 100;
  if (cmdVentil < 0)   cmdVentil = 0;
  watchdog_update();

  return (int)(cmdVentil * coefNorme);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Calcule l'asservissement du ventilateur en tout ou rien           ////
int8_t asservVentilateur(float temperature, float consigne)
{
  float coefDelta = 0.5; 
  static int8_t alimVentilateur = 1;
  watchdog_update();
  if (temperature > (consigne + coefDelta)) {
    alimVentilateur = 0;
  }
  if (temperature <= (consigne - coefDelta)) {
    alimVentilateur = 1;
  }
  watchdog_update();
  return alimVentilateur;
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Active la commande de mise en route de la chaudière               ////
void commandeChaudiere(int8_t cmdChaud)
{
  digitalWrite(CMD_PIN, !cmdChaud);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Active la commande de mise sous tension de la chaudière           ////
// void alimentationChaudiere(int8_t alimChaud)
// {
//   digitalWrite(REL_PIN, !alimChaud);
// }
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Affiche la température mesurée                                    ////
void renderTemperature(float temperature, int xVal, int yVal)
{
  watchdog_update();

  spt.createSprite(WTFT, 80);
  spt.loadFont(AA_FONT_TEMP);
  spt.fillSprite(TFT_BLACK);
  spt.setTextColor(TFT_WHITE, TFT_BLACK);
  spt.setTextDatum(MC_DATUM);

  if (temperature > 99.9) temperature = 99.9;
  if (temperature < -99.9) temperature = -99.9;

  int valInt = int(temperature);
  int valDec = abs((int(temperature * 10) - (valInt * 10)));
 
  spt.drawString((String) (valInt) + "," + (String) (valDec) + "°c", WTFT / 2, 40);
  if (modeDebug == 0) spt.pushSprite(xVal, yVal);

  spt.unloadFont();
  spt.deleteSprite();

  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////      Affiche la température de consigne                                ////
void renderConsigne(float temperature, int xVal, int yVal)
{
  watchdog_update();

  spc.createSprite(190, 50);
  spc.loadFont(AA_FONT_CONS);
  spc.fillSprite(TFT_BLACK);
  spc.setTextColor(TFT_GOLD, TFT_BLACK);
  spc.setTextDatum(MC_DATUM);

  if (temperature > 99.9) temperature = 99.9;
  if (temperature < 10.0) temperature = 10.0;

  int valInt = int(temperature);
  int valDec = abs((int(temperature * 10) - (valInt * 10)));
 
  spc.drawString((String) (valInt) + "," + (String) (valDec) + "°c", 95, 25);
  spc.pushSprite(xVal, yVal);

  spc.unloadFont();
  spc.deleteSprite();
  
  watchdog_update();

  renderDash(1, X_BB, Y_BB);
  renderDash(0, X_BY, Y_BY);
 
  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void renderEtat(int8_t valEtat, int xVal, int yVal)
{
  watchdog_update();

  switch (valEtat)
  {
  case 0:
    tft.pushImage(xVal, yVal, etat01Width, etat01Height, ETAT_01);
    break;
  case 1:
    tft.pushImage(xVal, yVal, etat02Width, etat02Height, ETAT_02);
    break;
  case 2:
    tft.pushImage(xVal, yVal, etat03Width, etat03Height, ETAT_03);
    break;
  case 3:
    tft.pushImage(xVal, yVal, etat04Width, etat04Height, ETAT_04);
    break;
  case 4:
    tft.pushImage(xVal, yVal, etat05Width, etat05Height, ETAT_05);
    break;
  case 5:
    tft.pushImage(xVal, yVal, etat06Width, etat06Height, ETAT_06);
    break;
  }  
  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void renderDash(int8_t valDash, int xVal, int yVal)
{
  watchdog_update();

  switch (valDash)
  {
  case 0:
    tft.pushImage(xVal, yVal, dash01Width, dash01Height, DASH_01);
    break;
  case 1:
    tft.pushImage(xVal, yVal, dash02Width, dash02Height, DASH_02);
    break;
  case 2:
    tft.pushImage(xVal, yVal, dash03Width, dash03Height, DASH_03);
    break;
  case 3:
    tft.pushImage(xVal, yVal, dash04Width, dash04Height, DASH_04);
    break;
  case 4:
    tft.pushImage(xVal, yVal, dash05Width, dash05Height, DASH_05);
    break;
  case 5:
    tft.pushImage(xVal, yVal, dash06Width, dash06Height, DASH_06);
    break;
  }  
  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void renderVentil(int8_t valVentil, int xVal, int yVal)
{

  watchdog_update();

  if (valVentil != sveConsigneVentil) {
    int yd_ventil = (int)(50 * ((float)valVentil / 100));

    tft.pushImage(xVal, yVal, etat06Width, etat06Height, ETAT_06);
    tft.pushImage(xVal, yVal, etat05Width, etat05Height - yd_ventil , ETAT_05);
    sveConsigneVentil = valVentil;
  }

  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
void renderModeVentil(int8_t modeVent, uint16_t coulTexte)
{
  watchdog_update();  

  spr.createSprite(100, 50);
  spr.loadFont(AA_FONT_TEXT);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(coulTexte, TFT_BLACK);
  spr.setTextDatum(MC_DATUM);

  switch (modeVent)
  {
  case 0:
    spr.drawString("Silence",50,25);
    break;
  case 1:
    spr.drawString("Cool",50,25);
    break;
  case 2:
    spr.drawString("Normal",50,25);
    break;
  case 3:
    spr.drawString("Maxi",50,25);
    break;
  case 4:
    spr.drawString("60 Minutes",50,25);
    break;
  }  

  spr.pushSprite(X_TEXT, Y_TEXT);
  spr.unloadFont();
  spr.deleteSprite();

  watchdog_update();
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                      Initialisation des boutons                        ////
////                                                                        ////
void initButtons() {
  btnHG.initButtonUL(X_BA, Y_BA, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "BHG", 1);
  btnHG.setPressAction(btnHG_pressAction);
  btnHG.setReleaseAction(btnHG_releaseAction);
  // btnHG.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  btnBG.initButtonUL(X_BB, Y_BB, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "BBG", 1);
  btnBG.setPressAction(btnBG_pressAction);
  btnBG.setReleaseAction(btnBG_releaseAction);
  // btnBG.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  btnHD.initButtonUL(X_BX, Y_BX, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "BHD", 1);
  btnHD.setPressAction(btnHD_pressAction);
  btnHD.setReleaseAction(btnHD_releaseAction);
  // btnHD.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  btnBD.initButtonUL(X_BY, Y_BY, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "BBD", 1);
  btnBD.setPressAction(btnBD_pressAction);
  btnBD.setReleaseAction(btnBD_releaseAction);
  // btnBD.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing
 
  btnLog.initButtonUL(X_BA, 90, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "Log", 1);
  btnLog.setPressAction(btnLog_pressAction);
  btnLog.setReleaseAction(btnLog_releaseAction);
  
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                      Bouton haut gauche                                ////
////                                                                        ////
void btnHG_pressAction(void)
{
  if (btnHG.justPressed()) {
    // btnHG.drawSmoothButton(true);
    btnHG.setPressTime(millis());
  }
  if (millis() - btnHG.getPressTime() >= 3000) {
    bouton = 5;
    modeArret = 1;
    tempoChaud = 0;
    incrementFonc = +1;
    if (modeFonc != 0) {
      tft.fillScreen(TFT_BLACK);
      modeFonc = 0;
      renderDash(5,X_BA,Y_BA);
      renderEtat(modeFonc, X_ETAT, Y_ETAT);
    }
  }

}
void btnHG_releaseAction(void)
{
  if ((bouton != 5) && (btnHG.justReleased())) {
    bouton = 1;
  }
  else bouton = 0;
  // btnHG.drawSmoothButton(false);
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                      Bouton bas gauche                                 ////
////                                                                        ////
void btnBG_pressAction(void)
{
  // btnBG.drawSmoothButton(true);

}
void btnBG_releaseAction(void)
{
  // btnBG.drawSmoothButton(false);
  if (btnBG.justReleased()) {
    bouton = 2;
  }
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                      Bouton haut droite                                ////
////                                                                        ////
void btnHD_pressAction(void)
{
  // btnHD.drawSmoothButton(true);
 
}
void btnHD_releaseAction(void)
{
  // btnHD.drawSmoothButton(false);
  if (btnHD.justReleased()) {
    bouton = 3;
    sveConsigneVentil = 255;
  }
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                      Bouton bas droite                                 ////
////                                                                        ////
void btnBD_pressAction(void)
{
  // btnBD.drawSmoothButton(true);
}
void btnBD_releaseAction(void)
{
  // btnBD.drawSmoothButton(false);
  if (btnBD.justReleased()) {
    bouton = 4;
  }
}
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                           Bouton Menu Debug - Log                     ////
void btnLog_pressAction(void)
{
  if (btnLog.justPressed()) {
    if (modeDebug == 0) modeDebug = 1;
    else modeDebug = 0;
  }
}
void btnLog_releaseAction(void)
{

}
////                                                                        ////

////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////                                ////
////                                                                        ////

////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
////              Calibration de l'écran                                    ////
////                                                                        ////
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
