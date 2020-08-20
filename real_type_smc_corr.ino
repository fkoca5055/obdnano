//Build date 18 April 2020

/**************************************************************************
  ST7735s     --->     ARDUINO
  VCC         --->     5V
  GND         --->     GND
  CS          --->     10
  RESET       --->     8
  A0(DC)      --->     9
  MOSI(SDA)   --->     11
  SCK(CLK)    --->     13
  LED         --->     3.3V

  ELM327
  TX          --->     3
  RX          --->     2

 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "images.h"
SoftwareSerial BTserial(2, 3); // RX | TX
const long baudRate = 38400;

#define CYAN     0x07FF
#define MAGENTA  0xF81F

#define BACKCOLOR 0x18E3
#define BARCOLOR 0x0620
#define SCALECOLOR 0xFFFF

#define TFT_CS        10
#define TFT_RST       8
#define TFT_DC        9
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define LED 6
#define BUTTON 4
int piezoPin = 7;

char rxData[128];
char rxIndex = 0;
int ADDR_PAGE_COUNTER = 0, LDR, SMC, LDRMAX, LDRMIN, EGT, turboRAW, COOLANT, x, LDRold, LastPercent = 0, INTEMP, LastBATT = 0, DPFL, LastSMC = 0, rpm;
byte pageCounter = 0;
bool pageChanged = false;
byte CACT, type;
float BATTERY, turboPRESS, turboMAX,  BATTERYf; //SMCF gr cinsinden hesaplamak için
bool coolantStatus = false, EGTStatus = false;

unsigned long time_now = 0;
unsigned long period = 1800; //1.8sn battery
unsigned long time_now2 = 0;
unsigned long period2 = 4100; //4.1sn BRIGHTNESS
unsigned long time_now3 = 0;
unsigned long period3 = 3300; //3.3sn coolant
unsigned long time_now4 = 0;
unsigned long period4 = 1000; //1sn egt
unsigned long time_now5 = 0;
unsigned long period5 = 60300; // 60.3sn DPF
unsigned long time_now6 = 0;
unsigned long period6 = 1550; //1.5sn cact
unsigned long time_now7 = 0;
unsigned long period7 = 5300; //5.3sn intemp

unsigned long delayTime = 50; //50 çalışıyor


char iBuf[5] = {};
char eBuf[5] = {};
char cBuf[5] = {};
char oBuf[5] = {};
char ldrBuf[5] = {};
char dpfBuf[5] = {};
char rpmBuf[5] = {};

int buttonStatePrevious = LOW;
unsigned long minButtonLongPressDuration = 3000;
unsigned long buttonLongPressMillis;
bool buttonStateLongPress = false;
const int intervalButton = 100;
unsigned long previousButtonMillis;
unsigned long buttonPressDuration;
unsigned long currentMillis;

void setup() {
  //Serial.begin(115200);
  BTserial.begin(baudRate);
  //Serial.println("Serial Begin...");
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);

  LDR = analogRead(A1);
  if (LDR >= 1024) {
    LDR = 1023;
  }
  LDR = map(LDR, 0, 1023, 10, 255);

  tft.drawBitmap(16, 0, vw_logo, 128, 128, ST77XX_WHITE);

  for (int i = 1; i < LDR; i++) {
    analogWrite(LED, i);
    delay(1);
  }
  delay(100);
  ODB_init();

  tft.fillScreen(ST77XX_BLACK);
  pageCounter = EEPROM.read(ADDR_PAGE_COUNTER);
  tone(piezoPin, 2000, 20);
}

void loop()
{
  //type=0 küçük type=1 büyük
  currentMillis = millis();
  readButtonState();
  if (pageCounter > 0) type = 1;
  switch (pageCounter) {
    case 0:
      type = 0;
      AutoBrightness();
      getCOOLANT();
      getBATT();
      getEGT();
      getSMC();
      getCACT();
      getINTEMP();
      break;

    case 1:
      AutoBrightness();
      getEGT();
      getSMC();
      break;

    case 2:
      AutoBrightness();
      getCACT();
      getINTEMP();
      break;

    case 3:
      AutoBrightness();
      getTURBOPRESS();
      break;

    case 4:
      AutoBrightness();
      getCOOLANT();
      getBATT();
      break;

    case 5:
      AutoBrightness();
      getRPM();
      break;

    case 6:
      //SCREEN OFF
      break;
  }

  savePageChange();
  pageChanged = false;

}



void ODB_init(void)
{
  /*tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(2);*/

  ///////////////////ATD////////////////////////////
  /*BTserial.print("ATD\r");
    tft.setCursor(5, 5);
    tft.print("ATD ");
    delay(200);
    OBD_read();
    tft.print(rxData);
    Serial.println(" ");
    Serial.print("ATD ");
    Serial.println(rxData);*/

  ///////////////////ATZ////////////////////////////
  BTserial.print("ATZ\r");
  /*tft.setCursor(5, 0);
    tft.print("ATZ ");*/
  delay(2000);  //2000
  OBD_read();
  /*tft.setTextSize(1);
    tft.print(rxData);
    Serial.print("ATZ ");
    Serial.println(rxData);
    tft.setTextSize(2);*/
  ///////////////////ATE0////////////////////////////
  BTserial.print("ATE0\r");
  /*tft.setCursor(5, 20);
    tft.print("ATE0 ");*/
  delay(500); //1000
  OBD_read();
  /*tft.print(rxData);
    Serial.print("ATE0 ");
    Serial.println(rxData);*/

  ///////////////////ATL0////////////////////////////
  BTserial.print("ATL0\r");
  /*tft.setCursor(5, 40);
    tft.print("ATL0 ");*/
  delay(500); //1000
  OBD_read();
  /*tft.print(rxData);
    Serial.print("ATL0 ");
    Serial.println(rxData);*/

  ///////////////////ATH0/////////////////////////////
  BTserial.print("ATH0\r");
  /*tft.setCursor(5, 60);
    tft.print("ATH0 ");*/
  delay(500); //1000
  OBD_read();
  /*tft.print(rxData);
    Serial.print("ATH0 ");
    Serial.println(rxData);*/

  ///////////////////ATSP6////////////////////////////
  BTserial.print("ATSP6\r");
  /*tft.setCursor(5, 80);
    tft.print("ATSP6 ");*/
  delay(500); //1000
  OBD_read();
  /*tft.print(rxData);
    Serial.print("ATSP6 ");
    Serial.println(rxData);*/

  ///////////////////ATSH7E0///////////////////////////
  BTserial.print("ATSH 7E0\r");
  /*tft.setCursor(5, 100);
    tft.print("ATSH7E0 ");*/
  delay(500);
  OBD_read();
  /*tft.print(rxData);
    Serial.print("ATSH7E0 ");
    Serial.println(rxData);*/

  /*if (ATSH != "OK") {
    ODB_init();
    }*/
  //delay(2000);
}


void getCOOLANT() {
  if (millis() > time_now3 + period3) {
    BTserial.print("0105\r");
    delay(delayTime);
    OBD_read();
    COOLANT = strtol(&rxData[6], 0, 16) - 40;

    switch (type) {
      case 0:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(0, 5);
        tft.setTextSize(2);
        tft.print("COOLANT: ");
        if (COOLANT < 50) {
          tft.setTextColor(ST77XX_BLACK, ST77XX_CYAN);
        }
        else if (COOLANT > 95) {
          tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        }
        
        tft.setCursor(100, 5);
        snprintf(oBuf, sizeof(oBuf), "%3d", COOLANT);
        tft.print(oBuf);
        tft.print("c");
        break;

      case 1:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(35, 0);
        tft.setTextSize(2);
        tft.print("COOLANT");
        if (COOLANT < 50) {
          tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
          //tft.drawCircle(75, 25, 3, ST77XX_CYAN);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          //tft.drawCircle(75, 25, 3, ST77XX_GREEN);
        }
        tft.setTextSize(6);
        tft.setCursor(5, 20);
        snprintf(oBuf, sizeof(oBuf), "%3d", COOLANT);
        tft.print(oBuf);
        tft.setTextSize(2);
        tft.print("o");
        tft.setTextSize(4);
        tft.print("C");


        if ((COOLANT > 40) && (COOLANT < 50)) {
          if (coolantStatus == false) {
            tone(piezoPin, 1000, 40);
            coolantStatus = true;
          }
        }
        break;
    }


    time_now3 = millis();
  }
}

void getINTEMP() {
  if (millis() > time_now7 + period7) {
    BTserial.print("010F\r");
    delay(delayTime);
    OBD_read();
    INTEMP = strtol(&rxData[6], 0, 16) - 40;

    switch (type) {
      case 0:
        tft.setCursor(0, 105);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2);
        tft.print("INTAKE: ");
        tft.setCursor(100, 105);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        snprintf(iBuf, sizeof(iBuf), "%3d", INTEMP);
        tft.print(iBuf);
        tft.print("c");
        break;

      case 1:
        tft.drawFastHLine(0, 90, 160, MAGENTA ); // Vertical SPERATOR

        tft.setCursor(5, 100);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2);
        tft.print("INTAKE: ");
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        snprintf(iBuf, sizeof(iBuf), "%3d", INTEMP);
        tft.print(iBuf);
        tft.setTextSize(0);
        tft.print("o");
        tft.setTextSize(2);
        tft.print("C");
        break;
    }
    time_now7 = millis();
  }
}

void getCACT() {
  if (millis() > time_now6 + period6) {
    BTserial.print("0177\r");
    delay(delayTime);
    OBD_read();
    CACT = strtol(&rxData[9], 0, 16) - 40;

    switch (type) {
      case 0:
        tft.setCursor(0, 85);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2);
        tft.print("CACT: ");
        tft.setCursor(100, 85);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        snprintf(cBuf, sizeof(cBuf), "%3d", CACT);
        tft.print(cBuf);
        tft.print("c");
        break;

      case 1:
        tft.setCursor(50, 4);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2);
        tft.print("CACT");
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        tft.setTextSize(5);
        tft.setCursor(10, 40);
        snprintf(cBuf, sizeof(cBuf), "%3d", CACT);
        tft.print(cBuf);
        tft.setTextSize(2);
        tft.print("o");
        tft.setTextSize(4);
        tft.print("C");
        break;
    }
    time_now6 = millis();
  }
}

void getBATT() {
  if (millis() > time_now + period) {
    BTserial.print("0142\r");
    delay(delayTime);
    OBD_read();
    BATTERY = ((strtol(&rxData[6], 0, 16) * 256) + strtol(&rxData[9], 0, 16)); //((A*256)+B)/1000
    float BATTERYf = BATTERY / 1000;

    switch (type) {
      case 0:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(0, 25);
        tft.setTextSize(2);
        tft.print("BATT: ");
        if (BATTERYf < 12) {
          tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        }
        tft.setCursor(100, 25);
        tft.print(BATTERYf, 1);
        tft.print("v");
        break;

      case 1:
        tft.drawFastHLine(0, 68, 160, MAGENTA ); // Vertical SPERATOR
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(15, 72);
        tft.setTextSize(2);
        tft.print("BATT: ");
        if (BATTERYf < 12) {
          tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        }
        tft.print(BATTERYf, 1);
        tft.print("v");
        graphBATT();
        tft.drawFastHLine(16, 107, 125, SCALECOLOR ); // Vertical Scale Line
        tft.drawFastVLine(16, 107, 8, SCALECOLOR); // Major Division
        tft.setCursor(6, 117);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.print("10.0");
        tft.drawFastVLine(47, 107, 5, SCALECOLOR); // Minor Division
        tft.setCursor(35, 117);
        tft.print("11.5");
        tft.drawFastVLine(78, 107, 8, SCALECOLOR); // Major Division
        tft.setCursor(66, 117);
        tft.print("13.0");
        tft.drawFastVLine(109, 107, 5, SCALECOLOR); // Minor Division
        tft.setCursor(97, 117);
        tft.print("14.5");
        tft.drawFastVLine(140, 107, 8, SCALECOLOR);  // Major Division
        tft.setCursor(128, 117);
        tft.print("16.0");
        break;
    }
    time_now = millis();
  }
}

void graphBATT() {
  int newBATT = map(BATTERY, 11000, 16000, 0, 125);
  if (newBATT != LastBATT) {
    drawBarBATT(newBATT);
  }
}

void drawBarBATT (int nPerBATT) {

  if (nPerBATT < LastBATT) {
    tft.fillRect(5 + (10 + LastBATT), 90 , nPerBATT - LastBATT , 15 ,  BACKCOLOR);
  }
  else if (BATTERY < 12000)  {
    tft.fillRect(5 + (10 + nPerBATT), 90 , LastBATT - nPerBATT, 15 ,  ST77XX_YELLOW);
  }
  else {
    tft.fillRect(5 + (10 + nPerBATT), 90 , LastBATT - nPerBATT, 15 ,  ST77XX_GREEN);
  }
  LastBATT = nPerBATT;
}

//SOOT MASS CALC
void getSMC() {
  if (millis() > time_now5 + period5) {
    BTserial.print("22114F\r");
    delay(delayTime);
    OBD_read();

    if ((strtol(&rxData[0], 0, 16) == 98) and (strtol(&rxData[3], 0, 16) == 17)) {
      SMC = ((strtol(&rxData[9], 0, 16) * 256) + strtol(&rxData[12], 0, 16)); //((A*256)+B)/100
      if (SMC < 3000) {
        /*SMCF = SMC; //gram olarak hesaplamak için
        SMCF = SMCF / 100;*/
        DPFL = map(SMC, 0, 2400, 0, 100);

        switch (type) {
          case 0:
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setTextSize(2);
            tft.setCursor(0, 65);
            tft.print("DPFL: ");
            tft.setCursor(100, 65);
            if (DPFL > 89) {
              tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
            }
            else {
              tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            }
            snprintf(dpfBuf, sizeof(dpfBuf), "%3d", DPFL);
            tft.print(dpfBuf);
            tft.print("%");
            break;

          case 1:
            tft.drawFastHLine(0, 65, 160, MAGENTA ); // Vertical SPERATOR


            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setTextSize(2);
            tft.setCursor(25, 70);
            tft.print("DPFL: ");
            tft.print("%");
            if (DPFL > 89) {
              tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
            }
            else {
              tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            }
            snprintf(dpfBuf, sizeof(dpfBuf), "%3d", DPFL);
            tft.print(dpfBuf);
            graphSMC();

            tft.drawFastHLine(16, 105, 125, SCALECOLOR ); // Vertical Scale Line
            tft.drawFastVLine(16, 105, 8, SCALECOLOR); // Major Division
            tft.setCursor(14, 115);
            tft.setTextSize(1);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.print("0");
            tft.drawFastVLine(47, 105, 5, SCALECOLOR); // Minor Division
            tft.setCursor(43, 115);
            tft.print("25");
            tft.drawFastVLine(78, 105, 8, SCALECOLOR); // Major Division
            tft.setCursor(74, 115);
            tft.print("50");
            tft.drawFastVLine(109, 105, 5, SCALECOLOR); // Minor Division
            tft.setCursor(105, 115);
            tft.print("75");
            tft.drawFastVLine(140, 105, 8, SCALECOLOR);  // Major Division
            tft.setCursor(132, 115);
            tft.print("100");

            break;
        }
        time_now5 = millis();
      }
    }
  }
}

void graphSMC() {
  int newSMC = map(DPFL, 0, 100, 0, 125);
  if (newSMC != LastSMC) {
    drawBarSMC(newSMC);

  }
}

void drawBarSMC (int nPerSMC) {

  if (nPerSMC < LastSMC) {
    tft.fillRect(5 + (10 + LastSMC), 88 , nPerSMC - LastSMC , 15 ,  BACKCOLOR);
  }
  else if (DPFL > 89)  {
    tft.fillRect(5 + (10 + nPerSMC), 88 , LastSMC - nPerSMC, 15 ,  ST77XX_YELLOW);
  }
  else {
    tft.fillRect(5 + (10 + nPerSMC), 88 , LastSMC - nPerSMC, 15 ,  ST77XX_GREEN);
  }
  LastSMC = nPerSMC;
}

void getEGT() {
  if (millis() > time_now4 + period4) {
    BTserial.print("0178\r");
    delay(delayTime);
    OBD_read();
    //if ((strtol(&rxData[0], 0, 16) == 65) and (strtol(&rxData[3], 0, 16) == 120)) {
    EGT = (((strtol(&rxData[30], 0, 16) * 256) + strtol(&rxData[33], 0, 16)) / 10) - 40;


    switch (type) {
      case 0:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(0, 45);
        tft.setTextSize(2);
        tft.print("EGT:");
        if (EGT < 400) {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        }
        else if ((EGT >= 400) && (EGT < 550)) {
          tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        }
        else if (EGT >= 550) {
          tft.setTextColor(ST77XX_WHITE, ST77XX_RED);
          
          if (EGTStatus == false) {
            tone(piezoPin, 2000, 30);

            EGTStatus = true;
          }
        }
        tft.setTextSize(2);
        tft.setCursor(100, 45);
        tft.print(" ");
        snprintf(eBuf, sizeof(eBuf), "%3d", EGT);
        tft.print(eBuf);
        tft.print("c");
        break;

      case 1:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(65, 0);
        tft.setTextSize(2);
        tft.print("EGT");
        if (EGT < 400) {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          //tft.drawCircle(185, 25, 3, ST77XX_GREEN);
          //tft.fillCircle(190, 7, 7, ST77XX_BLACK);
        }
        else if ((EGT >= 400) && (EGT < 550)) {
          tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
          //tft.drawCircle(185, 25, 3, ST77XX_YELLOW);
          //tft.fillCircle(190, 7, 7, ST77XX_YELLOW);
        }
        else if (EGT >= 550) {
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          //tft.drawCircle(185, 25, 3, ST77XX_RED);
          //tft.fillCircle(190, 7, 7, ST77XX_RED);
        }
        tft.setTextSize(5);
        tft.setCursor(25, 25);
        snprintf(eBuf, sizeof(eBuf), "%3d", EGT);
        tft.print(eBuf);
        tft.setTextSize(2);
        tft.print("o");
        tft.setTextSize(4);
        tft.print("C");
        break;
    }
    time_now4 = millis();
  }
}

int getINTAKEPRESS(void)
{

  BTserial.print("010B\r");
  //delay(delayTime);
  OBD_read();
  return strtol(&rxData[6], 0, 16);
}

int getBAROPRESS(void)
{

  BTserial.print("0133\r");
  //delay(delayTime);
  OBD_read();
  return strtol(&rxData[6], 0, 16);
}

void getTURBOPRESS() {
  turboRAW = (getINTAKEPRESS() - getBAROPRESS());
  if (turboRAW < 0) {
    turboRAW = abs(turboRAW);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setCursor(45, 4);
    tft.setTextSize(2);
    tft.print("VACUUM");
  }
  else {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(45, 4);
    tft.setTextSize(2);
    tft.print("BOOST ");
  }
  turboPRESS = turboRAW;
  turboPRESS = (turboPRESS * 0.01);

  graphTURBO();

  if (turboRAW < 139) {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  }
  else {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  }

  if (turboRAW <= 0) {
    tft.setTextSize(5);
  }
  else {
    tft.setTextSize(5);
  }
  tft.setCursor(10, 35);
  tft.print(turboPRESS, 2);
  tft.setTextSize(1);
  tft.print("bar");

  if (turboPRESS > turboMAX) {
    (turboMAX = turboPRESS);
  }
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(100, 22);
  tft.setTextSize(1);
  tft.print("PEAK:");
  tft.print(turboMAX, 2);

  tft.drawFastHLine(16, 103, 125, SCALECOLOR ); // Vertical Scale Line
  tft.drawFastVLine(16, 103, 8, SCALECOLOR); // Major Division
  tft.setCursor(8, 113);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("0.0");
  tft.drawFastVLine(47, 103, 5, SCALECOLOR); // Minor Division
  tft.setCursor(39, 113);
  tft.print("0.4");
  tft.drawFastVLine(78, 103, 8, SCALECOLOR); // Major Division
  tft.setCursor(70, 113);
  tft.print("0.8");
  tft.drawFastVLine(109, 103, 5, SCALECOLOR); // Minor Division
  tft.setCursor(101, 113);
  tft.print("1.2");
  tft.drawFastVLine(140, 103, 8, SCALECOLOR);  // Major Division
  tft.setCursor(132, 113);
  tft.print("1.55");
}

void graphTURBO() {
  int newPercent = map(turboRAW, 0, 155, 0, 125);
  if (newPercent != LastPercent) {
    drawBarTURBO(newPercent);
  }
}

void drawBarTURBO (int nPer) {

  if (nPer < LastPercent) {
    tft.fillRect(5 + (10 + LastPercent), 80 , nPer - LastPercent , 20 ,  BACKCOLOR);
  }
  else if (turboRAW < 139)  {
    tft.fillRect(5 + (10 + nPer), 80 , LastPercent - nPer, 20 ,  ST77XX_GREEN);
  }
  else {
    tft.fillRect(5 + (10 + nPer), 80 , LastPercent - nPer, 20 ,  ST77XX_RED);
  }
  LastPercent = nPer;

}

void getRPM() {

  BTserial.print("010C\r");
  delay(delayTime);
  OBD_read();
  rpm = ((strtol(&rxData[6], 0, 16) * 256) + strtol(&rxData[9], 0, 16)) / 4;
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(60, 4);
  tft.setTextSize(2);
  tft.print("RPM");
  tft.setCursor(40, 40);
  tft.setTextSize(3);
  snprintf(rpmBuf, sizeof(rpmBuf), "%4d", rpm);
  tft.print(rpmBuf);
  //graphRPM();
}

void OBD_read(void)
{
  char c;
  do {
    if (BTserial.available() > 0)
    {
      c = BTserial.read();
      //Serial.print("obd_read :");
      //Serial.println(c);
      if ((c != '>') && (c != '\r') && (c != '\n')) //Keep these out of our buffer
      {
        rxData[rxIndex++] = c; //Add whatever we receive to the buffer
      }
    }
  } while (c != '>'); //The ELM327 ends its response with this char so when we get it we exit out.
  rxData[rxIndex++] = '\0';//Converts the array into a string
  rxIndex = 0; //Set this to 0 so next time we call the read we get a "clean buffer"
  //Serial.println(rxData);
  String ERR = rxData;
  if (ERR == "CAN ERROR") {
    //Serial.println("CAN ERROR");
    tft.setCursor(100, 115);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setTextSize(0);
    tft.print("CAN ERROR");
  }
  if (ERR == "NO DATA") {
    //Serial.println("NO DATA");
    tft.setCursor(100, 115);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setTextSize(0);
    tft.print("NO DATA");
  }
}

void readButtonState() {
  if (currentMillis - previousButtonMillis > intervalButton) {


    int buttonState = digitalRead(BUTTON);

    if (buttonState == HIGH && buttonStatePrevious == LOW && !buttonStateLongPress) {
      buttonLongPressMillis = currentMillis;
      buttonStatePrevious = HIGH;
      //Serial.println("Button pressed");

      tft.fillScreen(ST77XX_BLACK);
      pageCounter++;
      pageChanged = true;
      if (pageCounter >= 6) {
        pageCounter = 0;
      }
      tone(piezoPin, 3200, 30);
    }


    buttonPressDuration = currentMillis - buttonLongPressMillis;

    if (buttonState == HIGH && !buttonStateLongPress && buttonPressDuration >= minButtonLongPressDuration) {
      buttonStateLongPress = true;
      //Serial.println("Button long pressed");
      analogWrite(LED, 0);
      pageCounter = 6;
    }

    if (buttonState == LOW && buttonStatePrevious == HIGH) {
      buttonStatePrevious = LOW;
      buttonStateLongPress = false;
      //Serial.println("Button released");

      if (buttonPressDuration < minButtonLongPressDuration) {
        //Serial.println("Button pressed shortly");
      }
    }

    previousButtonMillis = currentMillis;

  }
}

void savePageChange() {
  if (pageChanged) {
    EEPROM.write(ADDR_PAGE_COUNTER, pageCounter);
  }
}

/*
   0 COOLANT / BATTERY
   1 EGT / SMC
   2 CACT / INTAKE
   3 TURBO
   4 AFR
   5 Fuel Press
   6 SCREEN OFF
*/

void AutoBrightness() {
  if (millis() > time_now2 + period2) {
    LDR = analogRead(A1);

    if (LDR >= 1024) {
      LDR = 1023;
    }
    LDR = map(LDR, 0, 1023, 10, 255);
    if (LDR > LDRold) {
      for (LDRold; LDRold < LDR; LDRold++) {
        analogWrite(LED, LDRold);
        delay(1);
      }
    }

    if (LDR < LDRold) {
      for (LDRold; LDRold > LDR; LDRold--) {
        analogWrite(LED, LDRold);
        delay(1);
      }
    }
    LDRold = LDR;

    time_now2 = millis();
  }
}
