# AKAI_MPK-PLAY_BLE_MODIFICATION
BLE-MIDI Modification for the "AKAI MPK PLAY" MIDI-keyboard/synthesizer with Arduino.
A internal modification of the MPK play which adds MIDI over BLE for wireless connectivity
More information can be read at my blog: http://synthbror.com/2019/1034/

Main code is "the bgw_mod_mpkplay_m0bluefruit.ino" file, the rest of the sketches are tabs inside the main code.

Right now the project is based on a Adafruit feather M0 Bluefruit microcontroller, it works the same was as the newer arduinos (based on M0 chip).
The arduino (Feather M0) acts as a USB-host and converts the USB data and sends it over bluetooth.
It uses the USB-host SAMD library and the Bluefruit BLE library.

Credits:
 https://github.com/adafruit/Adafruit_BluefruitLE_nRF51
 https://github.com/gdsports/usbh_midi_samd
