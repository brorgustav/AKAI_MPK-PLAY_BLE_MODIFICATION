
// A small helper

void bleMidi_rx(uint16_t timestamp, uint8_t status, uint8_t data1, uint8_t data2);



void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// callback
void connected(void)
{
  bleState = true;
  //debug.println(F(" CONNECTED!"));
  delay(1000);
  //debug.println(bleState);
}
void disconnected(void)
{
  //debug.println("disconnected");
  bleState = false;
  //debug.println(bleState);
}

void setupBLE(void)
{
  //debug.println(F("BGW "));
  //debug.println(F("Adafruit Bluefruit MIDI Example"));
  //debug.println(F("---------------------------------------"));

  /* Initialise the module */
  //debug.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  //debug.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    //debug.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }
  ble.sendCommandCheckOK(F("AT+GAPDEVNAME=BGW_AKAI_MPKPLAY_BLE"));
                           ble.sendCommandCheckOK(F("ATZ"));
                           delay(1000);
                           //ble.sendCommandCheckOK(F("AT + uartflow = off"));
                               ble.echo(false);

                               //debug.println("Requesting Bluefruit info: ");
                               /* Print Bluefruit information */
                               ble.info();

                               /* Set BLE callbacks */
                               ble.setConnectCallback(connected);
                               ble.setDisconnectCallback(disconnected);

                               // Set MIDI RX callback
                               bleMidi.setRxCallback(bleMidi_rx);

                               //debug.println(F("Enable MIDI: "));
                               if ( ! bleMidi.begin(true) )
                               {
                               error(F("Could not enable MIDI"));
                             }

                               ble.verbose(false);
                               //debug.print(F("Waiting for a connection..."));
                             }
