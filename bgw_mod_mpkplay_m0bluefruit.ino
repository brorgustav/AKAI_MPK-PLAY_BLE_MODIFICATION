/*
 *******************************************************************************
   Legacy Serial MIDI and USB Host bidirectional converter
   Copyright (C) 2013-2017 Yuuichi Akagawa

   for use with Arduino MIDI library
   https://github.com/FortySevenEffects/arduino_bleMidi_library/

   Note:
   - If you want use with Leonardo, you must choose Arduino MIDI library v4.0 or higher.
   - This is sample program. Do not expect perfect behavior.
 *******************************************************************************
*/

bool activity = 0;
#include <Arduino.h>
#include <SPI.h>
#include <MIDI.h>
#include <usbh_midi.h>
#include <usbhub.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEMIDI.h"


#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.7.0"
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   false  // If set to 'true' enables debug output
#define BLUEFRUIT_UART_MODE_PIN        -1    // Set to -1 if unused
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7  //Bluefruit pin config:
#define BLUEFRUIT_SPI_RST              4  //https://learn.adafruit.com/bluefruit-le-micro-atmega32u4-microcontroller-usb-bluetooth-le-in-one/configuration
#define _MIDI_SERIAL_PORT Serial1
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
#define debug SERIAL_PORT_MONITOR
#else
#define debug Serial1
#endif
#define dbgEnabled 1
//#endif
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Adafruit_BLEMIDI bleMidi(ble);

USBHost UsbH;
USBHub  Hub1(&UsbH);
USBH_MIDI midiHost(&UsbH);
//USBH_MIDI  hostMidi2(&UsbH);
//USBH_MIDI hostMidi3(&UsbH);
//USBH_MIDI  hostMidi4(&UsbH);
struct MySettings : public midi::DefaultSettings
{
  static const unsigned SysExMaxSize = 1024; // Accept SysEx messages up to 1024 bytes long.
};
bool bleState = false;
bool debugNote = false; //if enabled sends an extra "default" note when sending note on
int current_note = 60;
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, _MIDI_SERIAL_PORT, MIDI, MySettings);
//Arduino MIDI library v4.2 compatibility
unsigned long activityCount = 0;
uint32_t debounceTime = 1000; //to not flood everything

void midiHost_poll();

//If you want handle System Exclusive message, enable this #define otherwise comment out it.
//#define USBH_MIDI_SYSEX_ENABLE

#ifdef USBH_MIDI_SYSEX_ENABLE
//SysEx:
void handle_sysex( byte* sysexmsg, unsigned sizeofsysex) {
  midiHost.SendSysEx(sysexmsg, sizeofsysex);
}
#endif
// Delay time (max 16383 us)
void noblockDelay(uint32_t t1, uint32_t t2, uint32_t delayTime);
void noblockDelay(uint32_t t1, uint32_t t2, uint32_t delayTime)
{
  uint32_t t3;

  if ( t1 > t2 ) {
    t3 = (0xFFFFFFFF - t1 + t2);
  } else {
    t3 = t2 - t1;
  }

  if ( t3 < delayTime ) {
    delayMicroseconds(delayTime - t3);
  }
}


void midiHost_poll()
{
  uint8_t size;
#ifdef USBH_MIDI_SYSEX_ENABLE
  uint8_t recvBuf[MIDI_EVENT_PACKET_SIZE];
  uint8_t rcode = 0;     //return code
  uint16_t  rcvd;
  uint8_t   readPtr = 0;
  rcode = midiHost.RecvData( &rcvd, recvBuf);
  //data check
  if (rcode != 0) return;
  if ( recvBuf[0] == 0 && recvBuf[1] == 0 && recvBuf[2] == 0 && recvBuf[3] == 0 ) {
    return ;
  }

  uint8_t *p = recvBuf;
  while (readPtr < MIDI_EVENT_PACKET_SIZE)  {
    if (*p == 0 && *(p + 1) == 0) break; //data end

    uint8_t outbuf[3];
    uint8_t rc = midiHost.extractSysExData(p, outbuf);
    if ( rc == 0 ) {
      p++;
      size = midiHost.lookupMsgSize(*p);
      _MIDI_SERIAL_PORT.write(p, size);
      p += 3;
    } else {
      _MIDI_SERIAL_PORT.write(outbuf, rc);
      p += 4;
    }
    readPtr += 4;
  }
#else
  uint8_t outBuf[ 3 ];
  //  midi::midiHostType mtype = (midi::midiHostType)outBuf[0];
  do {
    if ( (size = midiHost.RecvData(outBuf)) > 0 ) {
      bleMidiTX(outBuf[0], outBuf[1], outBuf[2]);
      //_MIDI_SERIAL_PORT.write(outBuf, size);
      //debug.print("[");
      //debug.print(outBuf[0]);
      //debug.print("]");
      //debug.print("[");
      //debug.print(outBuf[1]);
      //debug.print("]");
      //debug.print("[");
      //debug.print(outBuf[2]);
      //debug.print("]");
      //debug.print("[");
      //debug.print(outBuf[3]);
      //debug.println("]");
      //debug.println("-----------------------------------------");
    }
  } while (size > 0);
  #endif
}

void bleMidiTX(uint8_t status, uint8_t data1, uint8_t data2)
{
  bleMidi.send(status, data1, data2); //Forward to BLE
  //debug.println("Send BLE midi:");
  //debug.print(status);
  //debug.print("|");
  //debug.print(data1);
  //debug.print("|");
  //debug.print(data2);
  //debug.println("");
  if (debugNote == true)    //<- If declared true we will send a note to more easily keep track of how many times this function is polled
    //this makes it easier to debug since you can easily tell if the function was triggered,
  { // and no other notes passed through as MIDI
    bleMidi.send(0x90, 20, 20);
  }

  if (activity == false)
  {
    activity = true;
  }
}



void bleMidi_rx(uint16_t timestamp, uint8_t status, uint8_t data1, uint8_t data2)
{
  hostMidiTX(status, data1, data2);
  activity = true;
  //debug.println("RECIEVED ON BLUETOOTH END:");
  //debug.print("[");
  //debug.print(status);
  //debug.print("]");
  //debug.print("[");
  //debug.print(data1);
  //debug.print("]");
  //debug.print("[");
  //debug.print(data2);
  //debug.println("]");
  //debug.println("-----------------------------------------");
}
void hostMidiTX(byte status, byte data1, byte data2)
{
  byte tx[] = {status, data1, data2};
  midiHost.SendData(tx, 0);
  //  hostMidi2.SendData(tx, 0);
}
void setup()
{
  //  #ifdef dbgEnabled
  debug.begin(115200);
  //  #endif
  delay( 1500 );
  setupBLE();
  pinMode(LED_BUILTIN, OUTPUT);
  //MIDI.turnThruOff();
  //MIDI.begin(MIDI_CHANNEL_OMNI);
#ifdef USBH_MIDI_SYSEX_ENABLE
  //  MIDI.setHandleSystemExclusive(handle_sysex);
#endif
  if (UsbH.Init()) {
    // digitalWrite(LED_BUILTIN, HIGH);
    while (1);     //debug.println(F("USB host failed to start")); // halt
  }
    delay( 200 );
    //debug.println("BLE STATE:");
        //debug.println(bleState);
}

void loop()
{
    ble.update(500);
//    if (! bleState)
//    return;
    uint32_t t1 = (uint32_t)micros();
  UsbH.Task();
 if ( UsbH.getUsbTaskState() == USB_STATE_RUNNING )
{
  midiHost_poll();
 }
   noblockDelay(t1, (uint32_t)micros(), debounceTime); //slow down loop so we dont skip messages
  //    if (MIDI.read()) {
  //      msg[0] = MIDI.getType();
  //      switch (msg[0]) {
  //        case midi::ActiveSensing :
  //          break;
  //        case midi::SystemExclusive :
  //          //SysEx is handled by event.
  //          break;
  //        default :
  //          msg[1] = MIDI.getData1();
  //          msg[2] = MIDI.getData2();
  //          midiHost.SendData(msg, 0);
  //          break;
  //      }
  // interval for each scanning ~ 500ms (non blocking)
  //delay(1ms)

  // bail if not connected
}

void led(bool onoff)
{
  digitalWrite(LED_BUILTIN, onoff);
}
