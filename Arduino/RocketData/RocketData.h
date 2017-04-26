#include "spieeprom/spieeprom.cpp"

#define LED   6
#define BTLED 8
#define INTMPU 7

#define timeTransfer 5 // *10ms
#define localPressure   1026.4  // hPa

SPIEEPROM eeprom;

bool mpuInterrupt = false;
bool startRecord = false;
bool startSend = false;
bool BTconnected = false;
bool armed = false;

struct
{
  float sensor[2];
}data;

int tickDebug = 0;
int tickBT = 0;
int tickSample = 0;
int tickTransfer = 0;
long loopEndTime = 0;
long loopStartTime = 0;
long loopTime = 0;

String rxData = "";
String txData = "";

long address = 0;
int numSample = 0;

//float angleX, angleY, angleZ;
float accX, accY, accZ;
//float accX, accY, accZ;
//float tempMPU;
//float pression, tempBMP;
float alt, altFiltered, altInit;
