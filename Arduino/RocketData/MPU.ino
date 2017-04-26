#include "mpu6050/MPU6050_6Axis_MotionApps20.h"
#include "mpu6050/MPU6050.h"
#include "mpu6050/MPU6050.cpp"
#include "mpu6050/I2Cdev.h"
#include "mpu6050/I2Cdev.cpp"
#include "mpu6050/helper_3dmath.h"

MPU6050 mpu;

uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

Quaternion q;
VectorFloat gravity;
//int16_t gyro[3];
//float ypr[3];
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements

volatile bool dmpReady = false;

void dmpDataReady()
{
  mpuInterrupt = true;
  loopEndTime = micros() - loopStartTime;
  loopTime = loopEndTime;
  loopStartTime = micros();
  tickSample++;
  tickBT++;
  tickDebug++;
  tickTransfer++;
}

void initMPU6050()
{
  Wire.begin();
  //Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  TWBR = 6;
  // initialize device
#ifdef debugMPU
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
#endif
  // verify connection
#ifdef debugMPU
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
#endif
  // load and configure the DMP
#ifdef debugMPU
  Serial.println(F("Initializing DMP..."));
#endif
  devStatus = mpu.dmpInitialize();
  // supply your own gyro offsets here, scaled for min sensitivity
  //mpu.setXGyroOffset(511);
  //mpu.setYGyroOffset(255);
  //mpu.setZGyroOffset(255);
  //mpu.setXAccelOffset(-1281);
  //mpu.setYAccelOffset(-3841);
  //mpu.setZAccelOffset(2559);
  mpu.setI2CBypassEnabled(true);
  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
#ifdef debugMPU
    Serial.println(F("Enabling DMP..."));
#endif
    mpu.setDMPEnabled(true);
    // enable Arduino interrupt detection
#ifdef debugMPU
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
#endif
    mpuIntStatus = mpu.getIntStatus();
    // set our DMP Ready flag so the main loop() function knows it's okay to use it
#ifdef debugMPU
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
#endif
    dmpReady = true;
    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
#ifdef debugMPU
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
#endif
  }
}

void readSensors()
{
  boolean fifoError = false;
  // if programming failed, don't try to do anything
  //if (!dmpReady) return;
  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize)
  {
    readAltitude();
  }
  //while (mpu.getFIFOCount() < packetSize);
  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();
  // get current FIFO count
  fifoCount = mpu.getFIFOCount();
  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024)
  {
    // reset so we can continue cleanly
    mpu.resetFIFO();
#ifdef debugMPU
    Serial.println(F("FIFO overflow!"));
    fifoError = true;
#endif
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if ((mpuIntStatus & 0x02) && (!fifoError))
  {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;
    //Get sensor data
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    //mpu.dmpGetGyro(gyro, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    //mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    // angle and angular rate
    //angleX = ypr[2] * RAD_TO_DEG;
    //angleY = ypr[1] * RAD_TO_DEG;
    //angleZ = ypr[0] * RAD_TO_DEG;
    accX = aaReal.x * 0.000488; // accelerometer full scale = 16g -> acc = 1/2048 /g 
    accY = aaReal.y * 0.000488;
    accZ = aaReal.z * 0.000488;
    //tempMPU = ((float)mpu.getTemperature() / 340) + 36.53;
  }
}
