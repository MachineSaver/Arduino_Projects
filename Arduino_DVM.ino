#include <ArduinoRS485.h>
// ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Arduino.h>
#include <cstring>

#define SLAVE_ID 1

float combine_to_float(uint16_t a, uint16_t b) {
    uint32_t combined = ((uint32_t)a << 16) | b;
    float f;
    memcpy(&f, &combined, sizeof f);
    return f;
}

uint16_t high_word_of_float(float value) {
    uint32_t combined;
    memcpy(&combined, &value, sizeof value);
    return (combined >> 16) & 0xFFFF; // Extract the upper 16 bits
}

uint16_t low_word_of_float(float value) {
    uint32_t combined;
    memcpy(&combined, &value, sizeof value);
    return combined & 0xFFFF; // Extract the lower 16 bits
}

int adc_count;
float _calculated_three_volt_scale;
float _calculated_twenty_four_volt_scale;
float _calculated_mils;
const int analogPin = A0;
const int num_holding_registers = 8;
// 0-1 | 16-bit integer      | adc_count
// 2-3 | 32-bit floating point | 3.3 Volt Scaled Output
// 4-5 | 32-bit floating point | 24.0 Volt Scaled Output
// 6-7 | 32-bit floating point | mils (sensitivity assumed to be 200mV/mil)

void setup() {
  Serial.begin(115200);

  if (!ModbusRTUServer.begin(SLAVE_ID, 115200)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
  }
  ModbusRTUServer.configureHoldingRegisters(0x00, num_holding_registers);
}

void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  // Read the analog value from A0 pin
  adc_count = analogRead(analogPin);
  ModbusRTUServer.holdingRegisterWrite(0, adc_count);
  ModbusRTUServer.holdingRegisterWrite(1, 0);
  Serial.print("ADC Count: ");
  Serial.println(ModbusRTUServer.holdingRegisterRead(0));

  _calculated_three_volt_scale = (float)adc_count*(3.3/1023.0);
  ModbusRTUServer.holdingRegisterWrite(2, high_word_of_float(_calculated_three_volt_scale));
  ModbusRTUServer.holdingRegisterWrite(3, low_word_of_float(_calculated_three_volt_scale));  
  Serial.print("3.3V Scale: ");
  Serial.println(_calculated_three_volt_scale);

  _calculated_twenty_four_volt_scale = (_calculated_three_volt_scale / 0.1375);
  ModbusRTUServer.holdingRegisterWrite(4, high_word_of_float(_calculated_twenty_four_volt_scale));
  ModbusRTUServer.holdingRegisterWrite(5, low_word_of_float(_calculated_twenty_four_volt_scale));  
  Serial.print("24.0V Scale: ");
  Serial.println(_calculated_twenty_four_volt_scale);
  
  _calculated_mils = (_calculated_twenty_four_volt_scale / 0.200);
  ModbusRTUServer.holdingRegisterWrite(6, high_word_of_float(_calculated_mils));
  ModbusRTUServer.holdingRegisterWrite(7, low_word_of_float(_calculated_mils));  
  Serial.print("Calculated Mils: ");
  Serial.println(_calculated_mils);
  Serial.println(" ");
  Serial.println(" ");

  delay(100);
}
