#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

#define SLAVE_ID 1

int adc_count;
float _calculated_three_volt_scale;
float _calculated_twenty_four_volt_scale;
float _calculated_mils;
const int analogPin = A0;
const int num_holding_registers = 7;
// 0   | 16-bit integer        | adc_count
// 1-2 | 32-bit floating point | 3.3 Volt Scaled Output
// 3-4 | 32-bit floating point | 24.0 Volt Scaled Output
// 5-6 | 32-bit floating point | mils (sensitivity assumed to be 200mV/mil)


void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Modbus 24V to 3.3V Digital Voltage Meter");
  // Begin the Modbus Server
  if (!ModbusRTUServer.begin(SLAVE_ID, 115200)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
  }
  // configure holding registers at address 0x00
  ModbusRTUServer.configureHoldingRegisters(0x00, num_holding_registers);
}

void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  // Read the analog value from A0 pin
  adc_count = analogRead(analogPin);
  ModbusRTUServer.holdingRegisterWrite(0, adc_count);
  Serial.print("ADC Count: ");
  Serial.println(adc_count);

  _calculated_three_volt_scale = (float)adc_count * (3.3 / 1023.0);
  Serial.print("3.3V Scale: ");
  Serial.println(_calculated_three_volt_scale);
  // Convert float to a 32-bit integer
  uint32_t _three_volt_float = *(uint32_t*)(&_calculated_three_volt_scale);
  // Extract high and low words from the 32-bit integer
  uint16_t highWord = _three_volt_float >> 16;  // Get the high word
  uint16_t lowWord = _three_volt_float & 0xFFFF; // Get the low word
  ModbusRTUServer.holdingRegisterWrite(1, highWord);
  ModbusRTUServer.holdingRegisterWrite(2, lowWord);

  _calculated_twenty_four_volt_scale = (_calculated_three_volt_scale / 0.1375);
  Serial.print("24.0V Scale: ");
  Serial.println(_calculated_twenty_four_volt_scale);
  // Convert float to a 32-bit integer
  uint32_t _twentyfour_volt_float = *(uint32_t*)(&_calculated_twenty_four_volt_scale);
  // Extract high and low words from the 32-bit integer
  uint16_t highWord_1 = _twentyfour_volt_float >> 16;  // Get the high word
  uint16_t lowWord_1 = _twentyfour_volt_float & 0xFFFF; // Get the low word
  ModbusRTUServer.holdingRegisterWrite(3, highWord_1);
  ModbusRTUServer.holdingRegisterWrite(4, lowWord_1);
  
  _calculated_mils = (_calculated_twenty_four_volt_scale / 0.200);
    // Convert float to a 32-bit integer
  uint32_t _mils = *(uint32_t*)(&_calculated_mils);
  // Extract high and low words from the 32-bit integer
  uint16_t highWord_2 = _mils >> 16;  // Get the high word
  uint16_t lowWord_2 = _mils & 0xFFFF; // Get the low word
  ModbusRTUServer.holdingRegisterWrite(5, highWord_2);
  ModbusRTUServer.holdingRegisterWrite(6, lowWord_2);
  Serial.print("Calculated Mils: ");
  Serial.println(_calculated_mils);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");

  delay(1000);
}