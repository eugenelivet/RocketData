void initBT()
{
  Serial1.print("AT+BAUD4");  // §13 -> speed : 115200 bauds
  delay(50);
  Serial1.print("AT+NAME" + BTName);  // §38 -> module name
  delay(50);
  Serial1.write("AT+TYPE0");  // §61 -> param 1 : Auth not need PIN
  delay(50);
  Serial1.write("AT+SHOW1");  // §57 -> param 1 : Show name
  delay(50);
  Serial1.write("AT+RESET");  // §47 -> reset device
  delay(500);
}

void testBT()
{
  if (digitalRead(BTLED))
  {
    if ((tickBT >= 100) && (!BTconnected))
    {
#ifdef debugRxBT
      if (!BTconnected) Serial.println("BLE connected");
#endif
      BTconnected = true;
      tickBT = 100;
      txConfig();
    }
  }
  else
  {
#ifdef debugRxBT
    if (BTconnected) Serial.println("BLE disconnected");
#endif
    tickBT = 0;
    BTconnected = false;
  }
}

void rxBT()
{
  if (BTconnected)
  {
    if (Serial1.available())
    {
      rxData = Serial1.readStringUntil('\n');
#ifdef debugRxBT
      Serial.print("rxData -> ");
      Serial.println(rxData);
#endif
    }
  }
}

void txConfig()
{
  for (int i = 0; i < 3; i++)
  {
    int checksum = 0;
    txData = "nbspl," + String(nbSample);
    checksum = checkSum(txData);
    txData = txData + "," + String(checksum, DEC);
    Serial1.print(txData);
    delay(100);
    txData = "tmspl," + String(sampleTime * 10);
    checksum = checkSum(txData);
    txData = txData + "," + String(checksum, DEC);
    Serial1.print(txData);
    delay(100);
  }
}

void txAck(String ack)
{
  int checksum = 0;
  txData = ack;
  checksum = checkSum(txData);
  txData = txData + "," + String(checksum, DEC);
  for (int i = 0; i < 3; i++)
  {
    Serial1.print(txData);
    delay(100);
  }
}

int checkSum(String string)
{
  int checksum = 0;
  for (int i = 0; i < string.length(); i++)
  {
    checksum = int(checksum ^ string.charAt(i));
  }
  return checksum;
}
