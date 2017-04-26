/****************************************/
/***          CONFIGURATION           ***/
/****************************************/
#define sampleTime      1       // *10ms
#define nbSample        500
#define freqFiltreAlt   1.0     // Hz
#define altStartAcqu    1.0     // m
const String BTName = "RocketData_01";
/****************************************/

#include "RocketData.h"

//#define debugGlobal
//#define debugData
//#define debugBMP
//#define debugMPU
//#define debugRxBT
//#define debugTxBT

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BTLED, INPUT);
  pinMode(INTMPU, INPUT);
  eeprom.begin();
  initMPU6050();
  initBMP280();
  initBT();
  attachInterrupt(digitalPinToInterrupt(INTMPU), dmpDataReady, RISING);
#ifdef debugGlobal
  Serial.println("RocketData ready...");
#endif
}

void loop()
{
  /***      Sensors data reading        ***/
  if (!startSend)
  {
    readSensors();
  }
  /***        Bluetooth processing      ***/
  if (!startRecord)
  {
    testBT();
    rxBT();
  }
  /*** Wait for Arm acquition command   ***/
  if (BTconnected && rxData == "arm")
  {
    armed = true;
    rxData = "";
    tickTransfer = 0;
    tickSample = 0;
    initAltitude();
    txAck("armed");
    txConfig();
    digitalWrite(LED, HIGH);
#ifdef debugGlobal
    Serial.println("Acquisition armed...");
#endif
  }
  /***    Wait for rocket launching    ***/
  if (altFiltered >= altStartAcqu && armed)
  {
    address = 0;
    tickSample = 0;
    tickTransfer = 0;
    numSample = 0;
    startRecord = true;
    armed = false;
#ifdef debugGlobal
    Serial.println("Start data acquisition...");
#endif
  }
  /***   Data acquisition in progress   ***/
  if (startRecord && tickSample >= sampleTime)
  {
    data.sensor[0] = altFiltered;
    data.sensor[1] = accY;
    eeprom.write(address, data);
    address += sizeof(data);
    digitalWrite(LED, millis() % 150 > 50);
    if (numSample >= nbSample - 1)
    {
      txAck("acqend");
      startRecord = false;
      digitalWrite(LED, LOW);
#ifdef debugGlobal
      Serial.println("Data acquisition finished");
#endif
    }
    numSample++;
    tickSample = 0;
  }
  /***  Wait for data transfer command  ***/
  if (BTconnected && rxData == "transfer")
  {
    startSend = true;
    address = 0;
    numSample = 0;
    rxData = "";
    tickTransfer = 0;
    digitalWrite(LED, HIGH);
#ifdef debugGlobal
    Serial.println("Start data transmission...");
#endif
  }
  /***   Data transfer in progress      ***/
  if (startSend && tickTransfer > timeTransfer)
  {
    float dataSaved[sizeof(data) / 4];
    eeprom.read(address, dataSaved);
    txData = String(dataSaved[0], 1) + "," +
             String(dataSaved[1], 1);
    int checksum = 0;
    checksum = checkSum(txData);
    txData = txData + "," + String(checksum, DEC);
    Serial1.print(txData);
#ifdef debugGlobal
    Serial.print(numSample);
    Serial.print(":");
    Serial.print(txData);
    Serial.println();
#endif
    address += sizeof(data);
    digitalWrite(LED, millis() % 500 > 200);
    if (numSample >= nbSample - 1)
    {
      delay(2000);
      txAck("txend");
      startSend = false;
      digitalWrite(LED, LOW);
#ifdef debugGlobal
      Serial.println("Data transmission finished");
#endif
    }
    numSample++;
    tickTransfer = 0;
  }
  /***    Sensors data debugging     ***/
#ifdef debugData
  if (tickDebug > 5)
  {
    //Serial.print(data.timeStamp); Serial.print(F(", "));
    //Serial.print(altFiltered); Serial.print(F(", "));
    //Serial.print(pression); Serial.print(F(", "));
    //Serial.print(tempBMP); Serial.print(F(", "));
    //Serial.print(angleX); Serial.print(F(", "));
    //Serial.print(angleY); Serial.print(F(", "));
    //Serial.print(angleZ); Serial.print(F(", "));
    //Serial.print(accX); Serial.print(F(", "));
    Serial.print(accY); Serial.print(F(", "));
    //Serial.print(accZ); Serial.print(F(", "));
    //Serial.print(tempMPU); Serial.print(F(", "));
    Serial.println();
    tickDebug = 0;
  }
#endif
}
