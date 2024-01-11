// Author: Leo Bach
// Email: bach_leo@whirli.io
// Wiring Instructions: https://gitlab.com/machinesaver/modbus_motor_controller
// Hardware: Arduino MKR 1010 + MKR485 Shield + CL57T + 17HS13-0404D-E1000

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <stdio.h>
#include <stdint.h>

// Arduino Modbus Variables
int slave_id = 5;
long baudrate = 115200;
long serial_number = 2021012101;

// Motor 1 Pin Settings
int m1_dir_plus = 1;
int m1_pul_plus = 2;
int pulse_delay_us = 200; //200 microseconds minimum - manufacturer

//Voltage Divider Circuit
int analogPin = 1;

uint16_t MSB_16bit_of_float32 ( float float_number){
  union
  {
    float f_number;
    uint16_t uint16_arr[2];
  } union_for_conv;  
  union_for_conv.f_number = float_number;
  uint16_t MSB_uint16 = union_for_conv.uint16_arr[1];
  return MSB_uint16;
}

uint16_t LSB_16bit_of_float32 (float float_number){
  union
  {
    float f_number;
    uint16_t uint16_arr[2];
  } union_for_conv;  
  union_for_conv.f_number = float_number;
  uint16_t LSB_uint16 = union_for_conv.uint16_arr[0];
  return LSB_uint16;
}

//Functions to extract the HighWord(16bits) and LowWord(16-bits)from a long(32bit)
#define highWord(w) ((w) >> 16)
#define lowWord(w) ((w) & 0xffff)

void setup() {
  // set pins on arduino nano for driving motor #1 
  pinMode(m1_dir_plus,OUTPUT); //A DIGITAL PIN ---> DIR+
  pinMode(m1_pul_plus,OUTPUT); //A PWM PIN ---> PUL+

  // start the Modbus RTU server, with (slave) id 5
  if (!ModbusRTUServer.begin(slave_id, baudrate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
  }
  // configure holding registers addresses starting at 0x00, how many to setup
  ModbusRTUServer.configureHoldingRegisters(0x00, 124);
  ModbusRTUServer.holdingRegisterWrite(0x00, 0); // target_steps
  ModbusRTUServer.holdingRegisterWrite(0x01, 0); // position_steps
  ModbusRTUServer.holdingRegisterWrite(0x02, 0); // MSW_position_mils
  ModbusRTUServer.holdingRegisterWrite(0x03, 0); // LSW_position_mils
  ModbusRTUServer.holdingRegisterWrite(0x04, highWord(serial_number)); // MSW_serial_number
  ModbusRTUServer.holdingRegisterWrite(0x05, lowWord(serial_number)); // LSW_serial_number
  ModbusRTUServer.holdingRegisterWrite(0x06, 0); // MSW_voltage
  ModbusRTUServer.holdingRegisterWrite(0x07, 0); // LSW_voltage
}

void loop() {
  ModbusRTUServer.poll();
  int target_m1 = ModbusRTUServer.holdingRegisterRead(0x00);
  int position_m1 = ModbusRTUServer.holdingRegisterRead(0x01);
  if(target_m1>position_m1){
  //    Set Motor Direction to Clockwise
  digitalWrite(m1_dir_plus,LOW);
  //  STEP
  digitalWrite(m1_pul_plus,HIGH);
  delayMicroseconds(pulse_delay_us);
  digitalWrite(m1_pul_plus,LOW);
  delayMicroseconds(pulse_delay_us);
  //    Update Position
  position_m1+=1;
  }
  if(target_m1<position_m1){
  //    Set Motor Direction to CounterClockwise    
  digitalWrite(m1_dir_plus,HIGH);
  //  STEP
  digitalWrite(m1_pul_plus,HIGH);
  delayMicroseconds(pulse_delay_us);
  digitalWrite(m1_pul_plus,LOW);
  delayMicroseconds(pulse_delay_us);
  //    Update Position
  position_m1-=1;
  }
  ModbusRTUServer.holdingRegisterWrite(0x01, position_m1);
  float position_mils = position_m1*0.005;
  ModbusRTUServer.holdingRegisterWrite(0x02, MSB_16bit_of_float32(position_mils));
  ModbusRTUServer.holdingRegisterWrite(0x03, LSB_16bit_of_float32(position_mils));
  int voltageDivider = analogRead(analogPin);
  float voltage = voltageDivider * (24.0 / 1024.0);
  ModbusRTUServer.holdingRegisterWrite(0x06, MSB_16bit_of_float32(voltage));
  ModbusRTUServer.holdingRegisterWrite(0x07, LSB_16bit_of_float32(voltage));
}
