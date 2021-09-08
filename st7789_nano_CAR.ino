//Build date 05 April 2021

/**************************************************************************
  ST7789                ------------->     NANO
  GND                   ------------->     GND
  VCC                   ------------->     3.3V !!!
  SCL                   ---220 ohm--->     13
  SDA                   ---220 ohm--->     11
  RESET                 ---220 ohm--->     8
  DC                    ---220 ohm--->     9
  CS                    ---220 ohm--->     10
  BLK brightness        ---220 ohm--->     6
  LDR                   ------------->     A1
  BUTTON                ------------->     4

  ELM327
  TX                    ------------->     4
  RX                    ------------->     3


 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

SoftwareSerial BTserial(4, 3); // RX | TX
const long baudRate = 38400;

#define TFT_CS         10
#define TFT_RST        8
#define TFT_DC         9

#define CYAN           0x07FF
#define MAGENTA        0xF81F
#define BACKCOLOR      0x18E3
#define BARCOLOR       0x0620
#define SCALECOLOR     0xFFFF

#define LED 6

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

char rxData[128];
char rxIndex = 0;
const PROGMEM uint16_t ADDR_PAGE_COUNTER = 0;
int  LDR, SMC, LDRMAX, LDRMIN, EGT, turboRAW, COOLANT, LDRold, LastPercent = 0, INTEMP, DPFL, symbol;
byte pageCounter, CACT, type ;
bool pageChanged = false , screenCLEAR = false, SystemStatus = false;
float BATTERY, turboPRESS, turboMAX,  BATTERYf; //SMCF gr cinsinden hesaplamak için

unsigned long time_now = 0;
unsigned long period = 100;//1800; //1.8sn battery
unsigned long time_now2 = 0;
unsigned long period2 = 4100; //4.1sn BRIGHTNESS
unsigned long time_now3 = 0;
unsigned long period3 = 150;//3300; //3.3sn coolant
unsigned long time_now4 = 0;
unsigned long period4 = 200;//1000; //1sn egt
unsigned long time_now5 = 0;
unsigned long period5 = 250;//60300; // 60.3sn DPF
unsigned long time_now6 = 0;
unsigned long period6 = 300;//1550; //1.5sn cact
unsigned long time_now7 = 0;
unsigned long period7 = 350;//5300; //5.3sn intemp

unsigned long delayTime = 50; //50 çalışıyor

unsigned long currentMillis;

void setup() {

  BTserial.begin(baudRate);
  Serial.begin(9600);
  //tft.init(240, 320, SPI_MODE2);    // Init ST7789 display 240x320 pixel
  tft.init(240, 320);
  tft.setRotation(1);

  pinMode(LED, OUTPUT);

  LDR = analogRead(A1);
  if (LDR >= 1024) {
    LDR = 1023;
  }
  LDR = map(LDR, 0, 1024, 10, 255);
  analogWrite(LED, LDR);

  ODB_init();
  pageCounter = EEPROM.read(ADDR_PAGE_COUNTER);

  while (SystemStatus == false) {
    ODB_init();
  }

  attachInterrupt(digitalPinToInterrupt(2), ButtonState, RISING);

}



void ODB_init(void)
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.print("initializing");
  delay(1000);
  OBD_read();
  ///////////////////ATZ////////////////////////////
  BTserial.print("ATZ\r");
  delay(500);  //2000
  OBD_read();
  tft.print(".");
  ///////////////////ATE0////////////////////////////
  BTserial.print("ATE0\r");
  delay(200); //500
  OBD_read();
  tft.print(".");
  ///////////////////ATL0////////////////////////////
  BTserial.print("ATL0\r");
  delay(200); //500
  OBD_read();
  tft.print(".");
  ///////////////////ATH0/////////////////////////////
  BTserial.print("ATH0\r");
  delay(200); //500
  OBD_read();
  tft.print(".");
  ///////////////////ATSP6////////////////////////////
  BTserial.print("ATSP6\r");
  delay(200); //500
  OBD_read();
  tft.print(".");
  ///////////////////ATSH7E0///////////////////////////
  BTserial.print("ATSH 7E0\r");
  delay(200);//500
  OBD_read();
  tft.print(".");

  BTserial.print("0105\r");
  delay(delayTime);
  OBD_read();
  COOLANT = strtol(&rxData[6], 0, 16) - 40;
  if (COOLANT > -25) {
    SystemStatus = true;
    tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN);
    tft.print("OK");
    tft.fillScreen(ST77XX_BLACK);
  }
}

void loop() {
  //type=0 küçük type=1 büyük
  currentMillis = millis();

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
      getTURBOPRESS();
      break;

    case 2:
      //SCREEN OFF
      //AutoBrightness();
      analogWrite(LED, 0);
      tft.fillScreen(ST77XX_BLACK);
      break;

  }
  if (screenCLEAR == true) {
    tft.fillScreen(ST77XX_BLACK);
    screenCLEAR = false;
  }
  savePageChange();
  pageChanged = false;
  //loops();

}

/*void loops() {
  symbol++;

  if (symbol >= 4) {
    symbol = 0;
  }
  if (symbol == 0) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 110);
    tft.setTextSize(3);
    tft.print("|");
  }
  else if (symbol == 1) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 110);
    tft.setTextSize(3);
    tft.print(" ");
  }
  else if (symbol == 2) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 110);
    tft.setTextSize(3);
    tft.print("-");
  }
  else if (symbol == 3) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(10, 110);
    tft.setTextSize(3);
    tft.print(" ");
  }
}*/

void getCOOLANT() {
  if (millis() > time_now3 + period3) {
    BTserial.print("0105\r");
    delay(delayTime);
    OBD_read();
    COOLANT = strtol(&rxData[6], 0, 16) - 40;

    switch (type) {
      case 0:

        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(10, 20);
        tft.setTextSize(3);
        tft.print("AQUA");
        tft.setTextColor(MAGENTA, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.print("o");
        tft.setTextSize(3);
        tft.print("c");
        if (COOLANT < 50) {
          tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK );
          tft.drawRoundRect(0, 10, 110, 90, 10, ST77XX_CYAN);
          tft.drawRoundRect(1, 11, 108, 88, 10, ST77XX_CYAN);
          tft.drawRoundRect(2, 12, 106, 86, 10, ST77XX_CYAN);
        }
        else if (COOLANT > 95) {
          tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK );
          tft.drawRoundRect(0, 10, 110, 90, 10, ST77XX_ORANGE);
          tft.drawRoundRect(1, 11, 108, 88, 10, ST77XX_ORANGE);
          tft.drawRoundRect(2, 12, 106, 86, 10, ST77XX_ORANGE);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.drawRoundRect(0, 10, 110, 90, 10, ST77XX_GREEN);
          tft.drawRoundRect(1, 11, 108, 88, 10, ST77XX_GREEN);
          tft.drawRoundRect(2, 12, 106, 86, 10, ST77XX_GREEN);
        }
        tft.setTextSize(4);
        tft.setCursor(30, 60);
        tft.print(COOLANT);

        if (COOLANT < 10) {
          tft.print(" ");
        }
        else if (COOLANT < 100) {
          tft.print(" ");
        }
        break;
    }

    period3 = 3300;
    time_now3 = millis();
  }
}

void getEGT() {
  if (millis() > time_now4 + period4) {
    BTserial.print("0178\r");
    delay(delayTime);
    OBD_read();

    EGT = (((strtol(&rxData[30], 0, 16) * 256) + strtol(&rxData[33], 0, 16)) / 10) - 40;

    switch (type) {
      case 0:
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(125, 20);
        tft.setTextSize(3);
        tft.print("EGT");
        tft.setTextColor(MAGENTA, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.print("o");
        tft.setTextSize(3);
        tft.print("c");
        tft.setTextSize(4);
        if (EGT < 400) {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.drawRoundRect(115, 10, 90, 90, 10, ST77XX_GREEN);
          tft.drawRoundRect(116, 11, 88, 88, 10, ST77XX_GREEN);
          tft.drawRoundRect(117, 12, 86, 86, 10, ST77XX_GREEN);
        }
        else if ((EGT >= 400) && (EGT < 550)) {
          tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
          tft.drawRoundRect(115, 10, 90, 90, 10, ST77XX_YELLOW);
          tft.drawRoundRect(116, 11, 88, 88, 10, ST77XX_YELLOW);
          tft.drawRoundRect(117, 12, 86, 86, 10, ST77XX_YELLOW);
        }
        else if (EGT >= 550) {
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          tft.drawRoundRect(115, 10, 90, 90, 10, ST77XX_RED);
          tft.drawRoundRect(116, 11, 88, 88, 10, ST77XX_RED);
          tft.drawRoundRect(117, 12, 86, 86, 10, ST77XX_RED);
        }

        tft.setCursor(125, 60);
        tft.print(EGT);
        if (EGT < 10) {
          tft.print(" ");
        }
        else if (EGT < 100) {
          tft.print(" ");
        }

        break;
    }
    period4 = 1000;
    time_now4 = millis();
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
        tft.setCursor(218, 20);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(3);
        tft.print("CACT");
        tft.setTextColor(MAGENTA, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.print("o");
        tft.setTextSize(3);
        tft.print("c");
        tft.setTextSize(4);
        tft.setCursor(230, 60);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        tft.drawRoundRect(210, 10, 110, 90, 10, ST77XX_GREEN);
        tft.drawRoundRect(211, 11, 108, 88, 10, ST77XX_GREEN);
        tft.drawRoundRect(212, 12, 106, 86, 10, ST77XX_GREEN);
        tft.print(CACT);
        if (CACT < 10) {
          tft.print(" ");
        }
        else if (CACT < 100) {
          tft.print(" ");
        }

        break;
    }
    period6 = 1550;
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
        tft.setCursor(10, 150);
        tft.setTextSize(3);
        tft.print("BATT");
        tft.setTextColor(MAGENTA, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.print(" ");
        tft.setTextSize(3);
        tft.print("v");
        tft.setTextSize(4);
        if (BATTERYf < 12) {
          tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK );
          tft.drawRoundRect(0, 140, 110, 90, 10, ST77XX_ORANGE);
          tft.drawRoundRect(1, 141, 108, 88, 10, ST77XX_ORANGE);
          tft.drawRoundRect(2, 142, 106, 86, 10, ST77XX_ORANGE);
        }
        else {
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.drawRoundRect(0, 140, 110, 90, 10, ST77XX_GREEN);
          tft.drawRoundRect(1, 141, 108, 88, 10, ST77XX_GREEN);
          tft.drawRoundRect(2, 142, 106, 86, 10, ST77XX_GREEN);
        }
        tft.setCursor(10, 190);
        tft.print(BATTERYf, 1);

        break;
    }
    time_now = millis();
    period = 1800;
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
        tft.setCursor(125, 150);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(3);
        tft.print("INT");
        tft.setTextColor(MAGENTA, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.print("o");
        tft.setTextSize(3);
        tft.print("c");
        tft.setTextSize(4);
        tft.setCursor(130, 190);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);

        tft.print(INTEMP);
        tft.drawRoundRect(115, 140, 90, 90, 10, ST77XX_GREEN);
        tft.drawRoundRect(116, 141, 88, 88, 10, ST77XX_GREEN);
        tft.drawRoundRect(117, 142, 86, 86, 10, ST77XX_GREEN);

        break;
    }
    period7 = 5300;
    time_now7 = millis();
  }
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
            tft.setTextSize(3);
            tft.setCursor(218, 150);
            tft.print("DPFL");
            tft.setTextColor(MAGENTA, ST77XX_BLACK);
            tft.setTextSize(1);
            tft.print(" ");
            tft.setTextSize(3);
            tft.print("%");
            tft.setTextSize(4);
            tft.setCursor(230, 190);
            if (DPFL > 89) {
              tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK );
              tft.drawRoundRect(210, 140, 110, 90, 10, ST77XX_YELLOW);
              tft.drawRoundRect(211, 141, 108, 88, 10, ST77XX_YELLOW);
              tft.drawRoundRect(212, 142, 106, 86, 10, ST77XX_YELLOW);
            }
            else {
              tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
              tft.drawRoundRect(210, 140, 110, 90, 10, ST77XX_GREEN);
              tft.drawRoundRect(211, 141, 108, 88, 10, ST77XX_GREEN);
              tft.drawRoundRect(212, 142, 106, 86, 10, ST77XX_GREEN);
            }

            tft.print(DPFL);

            break;
        }
        period5 = 30300; //60300
        time_now5 = millis();
      }
    }
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
    tft.setCursor(90, 5);
    tft.setTextSize(4);
    tft.print("VACUUM");
  }
  else {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(90, 5);
    tft.setTextSize(4);
    tft.print("BOOST ");
  }
  turboPRESS = turboRAW;
  turboPRESS = (turboPRESS * 0.01);

  graphTURBO();

  tft.drawRoundRect(15, 155, 290, 30, 10, ST77XX_WHITE);
  tft.drawRoundRect(14, 154, 292, 32, 10, ST77XX_WHITE);

  if (turboRAW < 139) {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  }
  else {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  }

  tft.setTextSize(8);
  tft.setCursor(30, 70);
  tft.print(turboPRESS, 2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.print("bar");

  if (turboPRESS > turboMAX) {
    (turboMAX = turboPRESS);
  }
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  tft.setCursor(150, 40);
  tft.setTextSize(3);
  tft.print("peak:");
  tft.print(turboMAX, 2);

  tft.drawFastHLine(20, 190, 280, SCALECOLOR ); // Vertical Scale Line
  tft.drawFastVLine(20, 190, 15, SCALECOLOR); // Major Division
  tft.setCursor(0, 210);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("0.0");
  tft.drawFastVLine(90, 190, 10, SCALECOLOR); // Minor Division
  //tft.setCursor(39, 113);
  //tft.print("0.4");
  tft.drawFastVLine(160, 190, 15, SCALECOLOR); // Major Division
  tft.setCursor(135, 210);
  tft.print("0.8");
  tft.drawFastVLine(230, 190, 10, SCALECOLOR); // Minor Division
  //tft.setCursor(101, 113);
  //tft.print("1.2");
  tft.drawFastVLine(300, 190, 15, SCALECOLOR);  // Major Division
  tft.setCursor(245, 210);
  tft.print("1.55");
}

void graphTURBO() {
  int newPercent = map(turboRAW, 0, 155, 0, 280);
  if (newPercent != LastPercent) {
    drawBarTURBO(newPercent);
  }
}

void drawBarTURBO (int nPer) {

  if (nPer < LastPercent) {
    tft.fillRect(10 + (10 + LastPercent), 160 , nPer - LastPercent , 20 ,  BACKCOLOR);
  }
  else if (turboRAW < 139)  {
    tft.fillRect(10 + (10 + nPer), 160 , LastPercent - nPer, 20 ,  ST77XX_GREEN);
  }
  else {
    tft.fillRect(10 + (10 + nPer), 160 , LastPercent - nPer, 20 ,  ST77XX_RED);
  }
  LastPercent = nPer;

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


void ButtonState() {
  //delay(500);
  Serial.println("BUTTON DETECTED!!!");

  pageCounter++;

  pageChanged = true;
  if (pageCounter >= 3) {
    pageCounter = 0;
  }

  screenCLEAR = true;
}

void savePageChange() {
  if (pageChanged) {
    EEPROM.write(ADDR_PAGE_COUNTER, pageCounter);
  }
}

void AutoBrightness() {
  if (millis() > time_now2 + period2) {
    LDR = analogRead(A1);

    if (LDR >= 1024) {
      LDR = 1023;
    }
    LDR = map(LDR, 0, 1024, 10, 255);
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

    /*Serial.print("LDR: ");
      Serial.println(LDR);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(150, 120);
    tft.setTextSize(2);
    tft.print("Brightness:");
    tft.print(LDR);*/
    time_now2 = millis();
  }
}
