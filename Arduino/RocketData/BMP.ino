#include "bmp280/BMP280.h"
#include "bmp280/BMP280.cpp"
#include "filtre/filtre.cpp"

BMP280 bmp280;
LPF filtre(freqFiltreAlt, IS_BANDWIDTH_HZ, 3);

void initBMP280()
{
  if (!bmp280.begin()) {
#ifdef debugBMP
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
#endif      
    while (1);
  }
  delay(100);
  initAltitude();
}

void initAltitude()
{
  #define nbVal 200
  float altTab[nbVal];
  for (int i = 0; i < nbVal; i++)
  {
    alt = bmp280.readAltitude(localPressure);
    altTab[i] = alt;
    delay(10);
  }
  float sum = 0;
  for (int i = 0; i < nbVal; i++) sum += altTab[i];
  altInit = sum / nbVal;
}

void readAltitude()
{
  //tempBMP = bmp280.readTemperature();
  //pression = bmp280.readPressure();
  alt = bmp280.readAltitude(localPressure);
  alt -= altInit;
  altFiltered = max(filtre.NextValue(alt), 0);
}
