#include "DHT.h"
#include "Wire.h"
#include "SFE_BMP180.h"
#include "IRremote.h"
#include "LiquidCrystal_I2C.h"

#define dht22Pin A1 //Pinout DHT
#define irPin 5 //Pinout IRrecive
#define getDataInt 10000 //интервал опроса датчиков
#define sensorsCount 5 //количество подключенных датчиков
#define lcdRefInt 5000 // интервал переключения между выводом значений датчиов
#define fwVersion "v. 0.0.0.1"

DHT dht22(dht22Pin, DHT22);
IRrecv irrecv(irPin);
SFE_BMP180 pressure;
decode_results results;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long getDataTimer; //переменная для часов
unsigned long lcdRefTimer; //переменная для часов
boolean lcdShowSensor[sensorsCount];  //показ значений датчиков
byte screenMode;

void GetMeteoData() { //опрос датчиков
  float h22 = dht22.readHumidity();
  float t22 = dht22.readTemperature();
  double t180, p180;
  String string1, string2;
  char status;
  byte enabledSensorsCount = 0; //кол-во отображаемых сенсоров
  byte tmpVal = 0;
  lcdRefTimer = millis();

  for (byte i = 0; i < sensorsCount; i++) { //рассчет кол-ва элементов для обзора
    if (lcdShowSensor[i])
      enabledSensorsCount++;
  }

  byte scrollSensors[enabledSensorsCount] {8};

  for (byte i = 0; i < sensorsCount; i++) { //забиваем массив для обзора номерами датчиков
    if (lcdShowSensor[i]) {
      scrollSensors[tmpVal] = i;
      tmpVal++;
    }
  }

  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(t180);
    if (status != 0) {
      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);
        status = pressure.getPressure(p180, t180);
      }
    }
  }

  switch (screenMode) {

    case 0:
      for (byte i = 0; i < enabledSensorsCount; i++) { //enabledSensorsCount  ->sizeof(scrollSensors)

        switch (scrollSensors[i]) {

          case 0: //средняя температура
            string1 = String("Temperature avg.");
            string2 = String((t22 + t180) * 0.5, 2)  + String(" *C");
            break;

          case 1: //температура с DHT22
            string1 = String("Temp. 1 (DHT22)");
            string2 = String(t22)  + String(" *C");
            break;

          case 2: //влажность с DHT22
            string1 = String("Humidity");
            string2 = String(h22)  + String("%");
            break;

          case 3: // температура с Bmp180
            string1 = String("Temp. 2 (BMP180)");
            string2 = String(t180, 2)  + String(" *C");
            break;

          case 4: //Давление абсолют с Bmp180
            string1 = String("Pressure abs.");
            string2 = String(p180 * 0.750063755419211, 2) + String(" mm.hg.st.");
            break;
        }

        if (screenMode == 0)  { //дополнительная проверка
          lcdDraw(string1, string2);
          while (millis() - lcdRefTimer < lcdRefInt) {
            if (irrecv.decode( &results )) {
              irProcessing();
              irrecv.resume();
            }
          }
          lcdRefTimer = millis();
        } //дополнительная проверка

      }

      break;

    case 11:
      string1 = String("View temp. avg");
      string2 = String((t22 + t180) * 0.5, 2)  + String(" *C");
      lcdDraw(string1, string2);
      break;

    case 12:
      string1 = String("View temp.DHT22");
      string2 = String(t22)  + String(" *C");
      lcdDraw(string1, string2);
      break;

    case 13:
      string1 = String("View humidity");
      string2 = String(h22)  + String("%");
      lcdDraw(string1, string2);
      break;

    case 14:
      string1 = String("View temp.BMP180");
      string2 = String(t180, 2)  + String(" *C");
      lcdDraw(string1, string2);
      break;

    case 15:
      string1 = String("View pressure abs.");
      string2 = String(p180 * 0.750063755419211, 2) + String(" mm.hg.st.");
      lcdDraw(string1, string2);
      break;

  }
}

void lcdDraw(String lcdString1, String lcdString2) { //отрисовка на экране
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lcdString1);
  lcd.setCursor(0, 1);
  lcd.print(lcdString2);
}

void lcdDraw() { // отрисовка меню

  switch (screenMode) {

    case 0:
      lcdDraw("exit menu", " ");
      break;

    case 10:
      lcdDraw("      MENU", "^    Review    v" );
      break;

    case 20:
      lcdDraw("      MENU", "^   Settings   v" );
      break;

    case 21:
      if (lcdShowSensor[0])
        lcdDraw("    Settings", "^ Temp. avg. * v");
      else
        lcdDraw("    Settings", "^ Temp. avg.   v");
      break;

    case 22:
      if (lcdShowSensor[1])
        lcdDraw("    Settings", "^ Temp.DHT22 * v");
      else
        lcdDraw("    Settings", "^ Temp.DHT22   v");
      break;

    case 23:
      if (lcdShowSensor[2])
        lcdDraw("    Settings", "^ Humidity   * v");
      else
        lcdDraw("    Settings", "^ Humidity     v");
      break;

    case 24:
      if (lcdShowSensor[3])
        lcdDraw("    Settings", "^ Temp.BP180 * v");
      else
        lcdDraw("    Settings", "^ Temp.BP180   v");
      break;

    case 25:
      if (lcdShowSensor[4])
        lcdDraw("    Settings", "^ Press. abs.* v");
      else
        lcdDraw("    Settings", "^ Press. abs.  v");
      break;

    case 26:
      lcdDraw("Firmware version", fwVersion);
      break;

  }
}

void irProcessing() { //обработка нажатий пульта

  switch ( results.value ) {
    /*   case 0xFFA25D: //1
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFFA25D - 1" );
       }
       break;

      case 0xFF629D: //2
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF629D - 2" );
       }
       break;

      case 0xFFE21D: //3
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFFE21D - 3" );
       }
       break;

      case 0xFF22DD: //4
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF22DD - 4" );
       }
       break;

      case 0xFF02FD: //5
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF02FD - 5" );
       }
       break;

      case 0xFFC23D: //6
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFFC23D - 6" );
       }
       break;

      case 0xFFE01F: //7
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFFE01F - 7" );
       }
       break;

      case 0xFFA857: //8
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFFA857 - 8" );
       }
       break;

      case 0xFF906F: //9
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF906F - 9" );
       }
       break;

      case 0xFF9867: //0
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF9867 - 0" );
       }
       break;

      case 0xFF6897: //*
       if (screenMode == 17) {
         lcdDraw("Was received IR command:", "0xFF6897 - *" );
       }
       break;*/

    case 0xFFB04F: //#
      screenMode = 0;
      lcdDraw();
      break;

    case 0xFF38C7: //OK
      switch (screenMode) {

        case 10: //выбор подменю Ревью
          screenMode = 11;
          GetMeteoData();
          break;

        case 20: //Выбор подменю настройки
          screenMode = 21;
          lcdDraw();
          break;

        case 21:
          if (lcdShowSensor[0]) {
            lcdShowSensor[0] = 0;
            lcdDraw();
          }
          else {
            lcdShowSensor[0] = 1;
            lcdDraw();
          }
          break;

        case 22:
          if (lcdShowSensor[1]) {
            lcdShowSensor[1] = 0;
            lcdDraw();
          }
          else {
            lcdShowSensor[1] = 1;
            lcdDraw();
          }
          break;

        case 23:
          if (lcdShowSensor[2]) {
            lcdShowSensor[2] = 0;
            lcdDraw();
          }
          else {
            lcdShowSensor[2] = 1;
            lcdDraw();
          }
          break;

        case 24:
          if (lcdShowSensor[3]) {
            lcdShowSensor[3] = 0;
            lcdDraw();
          }
          else {
            lcdShowSensor[3] = 1;
            lcdDraw();
          }
          break;

        case 25:
          if (lcdShowSensor[4]) {
            lcdShowSensor[4] = 0;
            lcdDraw();
          }
          else {
            lcdShowSensor[4] = 1;
            lcdDraw();
          }
          break;

      }
      break;


    case 0xFF18E7: //^ "- пункт меню"
      switch (screenMode) {

        case 20:
          screenMode = 10;
          lcdDraw();
          break;

        case 0:
        case 10:
          screenMode = 20;
          lcdDraw();
          break;

        case 11:
          screenMode = 15;
          GetMeteoData();
          break;

        case 12:
        case 13:
        case 14:
        case 15:
          screenMode--;
          GetMeteoData();
          break;

        case 21:
          screenMode = 26;
          lcdDraw();
          break;

        case 22:
        case 23:     
        case 24:
        case 25:  
        case 26:
          screenMode--;
          lcdDraw();
          break;

      }
      break;

    case 0xFF4AB5: //V "+ пункт меню"
      switch (screenMode) {

        case 10:
          screenMode = 20;
          lcdDraw();
          break;

        case 11:
        case 12:
        case 13:
        case 14:
          screenMode++;
          GetMeteoData();
          break;

        case 15:
          screenMode = 11;
          GetMeteoData();
          break;

        case 0:
        case 20:
          screenMode = 10;
          lcdDraw();
          break;

        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
          screenMode++;
          lcdDraw();
          break;

        case 26:
          screenMode = 21;
          lcdDraw();
          break;

      }
      break;

    case 0xFF10EF: //<
      switch (screenMode) {
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
          screenMode = 10;
          lcdDraw();
          break;

        case 10:
        case 20:
          screenMode = 0;
          lcdDraw();
          break;

      }
      break;

      //   case 0xFF5AA5: //>
      //     break;
  }
}

void setup() {
  dht22.begin();
  pressure.begin();
  irrecv.enableIRIn();
  lcd.init();
  lcd.backlight();

  lcdShowSensor[0] = true; //средняя тепература (0)
  lcdShowSensor[1] = false;//температура DHT22  (1)
  lcdShowSensor[2] = true; // влажность         (2)
  lcdShowSensor[3] = false;//температура BMP180 (3)
  lcdShowSensor[4] = true; //давление абс       (4)

  screenMode = 0; //режим экрана - 0 - основной список

  lcdDraw("My meteostation", fwVersion);
}

void loop() {

  if ((millis() - getDataTimer > getDataInt) && (screenMode == 0 ))
  {
    getDataTimer = millis();
    GetMeteoData();
  }

  if (irrecv.decode( &results )) {
    irProcessing();
    irrecv.resume();
  }
}
