#include <Wire.h>

#define MPR121_ADDR 0x5A
#define IRQ_PIN 2
#define SPEAKER_PIN 3 // аудіо вихід
#define TOU_THRESH 0x06
#define REL_THRESH 0x0A

byte midiNotes[12] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79}; // міді ноти що відправляє на комп
int toneFreqs[12] = {65, 73, 82, 87, 98, 110, 123, 131, 139, 147, 156, 165}; // частоти нот що видає ардуінка
bool touchStates[12] = {0};

void setup() {
  pinMode(IRQ_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  Wire.begin();
  Serial.begin(57600); // для Hairless MIDI

  mpr121_setup();
}

void loop() {
  if (!digitalRead(IRQ_PIN)) {
    Wire.requestFrom(MPR121_ADDR, 2);
    if (Wire.available() >= 2) {
      uint16_t touched = Wire.read();
      touched |= (Wire.read() << 8);

      for (int i = 0; i < 12; i++) {
        bool isTouched = touched & (1 << i);
        if (isTouched && !touchStates[i]) {
          touchStates[i] = true;
          sendNoteOn(midiNotes[i], 127);
          tone(SPEAKER_PIN, toneFreqs[i]); // Почати відтворення тону
        } else if (!isTouched && touchStates[i]) {
          touchStates[i] = false;
          sendNoteOff(midiNotes[i], 0);
          noTone(SPEAKER_PIN); // Зупинити звук
        }
      }
    }
  }
}

void sendNoteOn(byte note, byte velocity) {
  Serial.write(0x90); // Note On, channel 1
  Serial.write(note);
  Serial.write(velocity);
}

void sendNoteOff(byte note, byte velocity) {
  Serial.write(0x80); // Note Off, channel 1
  Serial.write(note);
  Serial.write(velocity);
}

void mpr121_setup() {
  writeRegister(0x5E, 0x00); // Stop mode

  writeRegister(0x2B, 0x01);
  writeRegister(0x2C, 0x01);
  writeRegister(0x2D, 0x00);
  writeRegister(0x2E, 0x00);
  writeRegister(0x2F, 0x01);
  writeRegister(0x30, 0x01);
  writeRegister(0x31, 0xFF);
  writeRegister(0x32, 0x02);

  for (byte i = 0; i < 12; i++) {
    writeRegister(0x41 + i * 2, TOU_THRESH);
    writeRegister(0x42 + i * 2, REL_THRESH);
  }

  writeRegister(0x5D, 0x04);
  writeRegister(0x5E, 0x0C); // Enable electrodes
}

void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(MPR121_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
