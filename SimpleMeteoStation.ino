#include <LiquidCrystal_I2C.h>
#include <SFE_BMP180.h>
#include <DHT.h>
#define DHTPin A1
#define fw_version 0.0.0.1

DHT dht(DHTPin, DHT22);
SFE_BMP180 pressure;
LiquidCrystal_I2C lcd(0x27, 16, 2);

long delayTime;

void PrintLCD (String str1, String str2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);
}

void GetMeteoData() {
  char status;
  double TBMP180, PBMP180, a;

  float HDHT22 = dht.readHumidity(); //Измеряем влажность
  float TDHT22 = dht.readTemperature(); //Измеряем температуру

  if (isnan(HDHT22) || isnan(TDHT22)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    PrintLCD("Error DHT22", "");
    return;
  }

  PrintLCD("Temperature", String(TDHT22) + " t*");
  delay(5000);

  PrintLCD("Humidity", String(HDHT22) + " %");
  delay(5000);

  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(TBMP180);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(PBMP180, TBMP180);
        if (status != 0)
        {
          //          Serial.print("absolute pressure: ");
          //          Serial.print(PBMP180,2);
          //          Serial.print(" mb, ");
          //          Serial.print(PBMP180*0.0295333727,2);
          //          Serial.println(" inHg");
          PrintLCD("Pressure", String((PBMP180*0.750062)) + " mm.Hg.");
          delay(5000);
        }
      }
    }
  }


}

void setup() {
  pressure.begin();
  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея
  dht.begin();
  delayTime=0;

  lcdDraw("Simple Meteostation", fwVersion);
  
}

void loop() {
  GetMeteoData();
}
