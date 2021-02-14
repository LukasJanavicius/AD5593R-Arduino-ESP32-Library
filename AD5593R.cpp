#include "AD5593R.h"
#include <Wire.h>

//Definitions
#define _ADAC_NULL           B00000000
#define _ADAC_ADC_SEQUENCE   B00000010 // ADC sequence register - Selects ADCs for conversion
#define _ADAC_GP_CONTROL     B00000011 // General-purpose control register - DAC and ADC control register
#define _ADAC_ADC_CONFIG     B00000100 // ADC pin configuration - Selects which pins are ADC inputs
#define _ADAC_DAC_CONFIG     B00000101 // DAC pin configuration - Selects which pins are DAC outputs
#define _ADAC_PULL_DOWN      B00000110 // Pull-down configuration - Selects which pins have an 85 kO pull-down resistor to GND
#define _ADAC_LDAC_MODE      B00000111 // LDAC mode - Selects the operation of the load DAC
#define _ADAC_GPIO_WR_CONFIG B00001000 // GPIO write configuration - Selects which pins are general-purpose outputs
#define _ADAC_GPIO_WR_DATA   B00001001 // GPIO write data - Writes data to general-purpose outputs
#define _ADAC_GPIO_RD_CONFIG B00001010 // GPIO read configuration - Selects which pins are general-purpose inputs
#define _ADAC_POWER_REF_CTRL B00001011 // Power-down/reference control - Powers down the DACs and enables/disables the reference
#define _ADAC_OPEN_DRAIN_CFG B00001100 // Open-drain configuration - Selects open-drain or push-pull for general-purpose outputs
#define _ADAC_THREE_STATE    B00001101 // Three-state pins - Selects which pins are three-stated
#define _ADAC_RESERVED       B00001110 // Reserved
#define _ADAC_SOFT_RESET     B00001111 // Software reset - Resets the AD5593R

/**
 * @name     ADAC Configuration Data Bytes
 ******************************************************************************/
 ///@{
 //write into MSB after _ADAC_POWER_REF_CTRL command to enable VREF
#define _ADAC_VREF_ON     B00000010
#define _ADAC_SEQUENCE_ON B00000010



/**
 * @name   ADAC Write / Read Pointer Bytes
******************************************************************************/
///@{
#define _ADAC_DAC_WRITE       B00010000
#define _ADAC_ADC_READ        B01000000
#define _ADAC_DAC_READ        B01010000
#define _ADAC_GPIO_READ       B01110000
#define _ADAC_REG_READ        B01100000






//Class constructor
AD5593R::AD5593R(int a0) {

  _a0 = a0;
  _GPRC_msbs = 0x00;
  _GPRC_lsbs = 0x00;
  _PCR_msbs = 0x00;
  _PCR_lsbs = 0x00;
  //intializing the configuration struct.
  for (int i = 0; i < _num_of_channels; i++) {
    config.ADCs[i] = 0;
    config.DACs[i] = 0;
  }

  for (int i = 0; i < _num_of_channels; i++) {
    values.ADCs[i] = -1;
    values.DACs[i] = -1;
  }

  //this allows for multiple devices on the same bus, see header.
  if (_a0 > -1) {
    pinMode(_a0, OUTPUT);
    digitalWrite(_a0, HIGH);
  }
  Wire.begin();
}


//int AD5593R::configure_pins(*configuration config){

//}

void AD5593R::enable_internal_Vref() {
  //Enable selected device for writing
  _Vref = 2.5;
  _ADC_max = _Vref;
  _DAC_max = _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);

  //check if the on bit is already fliped on
  if ((_PCR_msbs & 0x02) != 0x02) {
    _PCR_msbs = _PCR_msbs ^ 0x02;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_POWER_REF_CTRL);
  Wire.write(_PCR_msbs);
  Wire.write(_PCR_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("Internal Reference on.");
}

void AD5593R::disable_internal_Vref() {
  //Enable selected device for writing
  _Vref = -1;
  _ADC_max = _Vref;
  _DAC_max = _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);
  //check if the on bit is already fliped off
  if ((_PCR_msbs & 0x02) == 0x02) {
    _PCR_msbs = _PCR_msbs ^ 0x02;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_POWER_REF_CTRL);
  Wire.write(_PCR_msbs);
  Wire.write(_PCR_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("Internal Reference off.");
}

void AD5593R::set_ADC_max_2x_Vref() {
  //Enable selected device for writing
  _ADC_max = 2 * _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);
  //check if 2x bit is on in the general purpose register
  if ((_GPRC_lsbs & 0x20) != 0x20) {
    _GPRC_lsbs = _GPRC_lsbs ^ 0x20;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GP_CONTROL);
  Wire.write(_GPRC_msbs);
  Wire.write(_GPRC_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("ADC max voltage = 2xVref");
  _ADC_2x_mode = 1;
}

void AD5593R::set_ADC_max_1x_Vref() {
  //Enable selected device for writing
  _ADC_max = _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);

  if ((_GPRC_lsbs & 0x20) == 0x20) {
    _GPRC_lsbs = _GPRC_lsbs ^ 0x20;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GP_CONTROL);
  Wire.write(_GPRC_msbs);
  Wire.write(_GPRC_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("ADC max voltage = 1xVref");
  _ADC_2x_mode = 0;
}

void AD5593R::set_DAC_max_2x_Vref() {
  //Enable selected device for writing
  _DAC_max = 2 * _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);

  if ((_GPRC_lsbs & 0x10) != 0x10) {
    _GPRC_lsbs = _GPRC_lsbs ^ 0x10;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GP_CONTROL);
  Wire.write(_GPRC_msbs);
  Wire.write(_GPRC_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("DAC max voltage = 2xVref");
  _DAC_2x_mode = 1;
}

void AD5593R::set_DAC_max_1x_Vref() {
  //Enable selected device for writing
  _DAC_max = _Vref;
  if (_a0 > -1) digitalWrite(_a0, LOW);

  if ((_GPRC_lsbs & 0x10) == 0x10) {
    _GPRC_lsbs = _GPRC_lsbs ^ 0x10;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GP_CONTROL);
  Wire.write(_GPRC_msbs);
  Wire.write(_GPRC_lsbs);
  Wire.endTransmission();

  //Disable selected device for writing
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINTLN("ADC max voltage = 1xVref");
  _DAC_2x_mode = 0;
}

void AD5593R::set_Vref(float Vref) {
  _Vref = Vref;
  if (_ADC_2x_mode == 0) {
    _ADC_max = Vref;
  }
  else {
    _ADC_max = 2 * Vref;
  }

  if (_DAC_2x_mode == 0) {
    _DAC_max = Vref;
  }
  else {
    _DAC_max = 2 * Vref;
  }

}

void AD5593R::configure_DAC(byte channel) {
  if (_a0 > -1) digitalWrite(_a0, LOW);
  config.DACs[channel] = 1;
  byte channel_byte = 1 << channel;
  //check to see if the channel is a DAC already
  if ((_DAC_config & channel_byte) != channel_byte) {
    _DAC_config = _DAC_config ^ channel_byte;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_DAC_CONFIG);
  Wire.write(0x0);
  Wire.write(_DAC_config);
  Wire.endTransmission();

  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINTLN(" is configured as a DAC");
}


void AD5593R::configure_DACs(bool* channels) {
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (channels[i] == 1) {
      configure_DAC(i);
    }
  }
}


int AD5593R::write_DAC(byte channel, float voltage) {
  //error checking
  if (config.DACs[channel] == 0) {
    AD5593R_PRINT("ERROR! Channel ");
    AD5593R_PRINT(channel);
    AD5593R_PRINTLN(" is not a DAC");
    return -1;
  }
  if (_DAC_max == -1) {
    AD5593R_PRINTLN("Vref, or DAC_max is not defined");
    return -2;
  }
  if (voltage > _DAC_max) {
    AD5593R_PRINTLN("Vref, or DAC_max is lower than set voltage");
    return -3;
  }

  if (_a0 > -1) digitalWrite(_a0, LOW);

  //find the binary representation of the
  unsigned int data_bits = (voltage / _DAC_max) * 4095;

  //extract the 4 most signifigant bits, and move them down to the bottom
  byte data_msbs = (data_bits & 0xf00) >> 8;
  byte lsbs = (data_bits & 0x0ff);
  //place the channel data in the most signifigant bits
  byte msbs = (B10000000 | (channel << 4)) | data_msbs;

  Wire.beginTransmission(_i2c_address);
  Wire.write((_ADAC_DAC_WRITE | channel));
  Wire.write(msbs);
  Wire.write(lsbs);
  Wire.endTransmission();

  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINT(" is set to ");
  AD5593R_PRINT(voltage);
  AD5593R_PRINTLN(" Volts");
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  values.DACs[channel] = voltage;
  return 1;
}

void AD5593R::configure_ADC(byte channel) {
  if (_a0 > -1) digitalWrite(_a0, LOW);
  config.ADCs[channel] = 1;
  byte channel_byte = 1 << channel;
  //check to see if the channel is a ADC already
  if ((_ADC_config & channel_byte) != channel_byte) {
    _ADC_config = _ADC_config ^ channel_byte;
  }
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_ADC_CONFIG);
  Wire.write(0x0);
  Wire.write(_ADC_config);
  Wire.endTransmission();

  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINTLN(" is configured as a ADC");
}

void AD5593R::configure_ADCs(bool* channels) {
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (channels[i] == 1) {
      configure_ADC(i);
    }
  }
}


float AD5593R::read_ADC(byte channel) {
  if (config.ADCs[channel] == 0) {
    AD5593R_PRINT("ERROR! Channel ");
    AD5593R_PRINT(channel);
    AD5593R_PRINTLN(" is not an ADC");
    return -1;
  }
  if (_ADC_max == -1) {
    AD5593R_PRINTLN("Vref, or ADC_max is not defined");
    return -2;
  }
  if (_a0 > -1) digitalWrite(_a0, LOW);

  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_ADC_SEQUENCE);
  Wire.write(0x02);
  Wire.write(byte(1 << channel));
  Wire.endTransmission();


  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_ADC_READ);
  Wire.endTransmission();

  unsigned int data_bits = 0;

  Wire.requestFrom(int(_i2c_address), int(2), int(1));
  if (Wire.available()) data_bits = (Wire.read() & 0x0f) << 8;
  if (Wire.available()) data_bits = data_bits | Wire.read();
  if (_a0 > -1) digitalWrite(_a0, HIGH);
  float data = _ADC_max * (data_bits) / 4095;

  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINT(" reads ");
  AD5593R_PRINT(data);
  AD5593R_PRINTLN(" Volts");
  return data;
}

float* AD5593R::read_ADCs() {
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (config.ADCs[i] == 1) {
      read_ADC(i);
    }
  }
  return values.ADCs;
}


void AD5593R::configure_GPI(byte channel) {
  if (_a0 > -1) digitalWrite(_a0, LOW);
  config.DACs[channel] = 1;
  byte channel_byte = 1 << channel;
  //check to see if the channel is a gpi already
  if ((_GPI_config & channel_byte) != channel_byte) {
    _GPI_config = _GPI_config ^ _GPI_config;
  }
  Wire.beginTransmission(_i2c_address);
  // write  to gpio-read register
  Wire.write(_ADAC_GPIO_RD_CONFIG);
  Wire.write(0x0);
  Wire.write(_GPI_config);
  Wire.endTransmission();

  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINTLN(" is configured as a GPI");
}

void AD5593R::configure_GPIs(bool* channels) {
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (channels[i] == 1) {
      configure_GPI(i);
    }
  }
}


void AD5593R::configure_GPO(byte channel) {
  if (_a0 > -1) digitalWrite(_a0, LOW);
  config.DACs[channel] = 1;
  byte channel_byte = 1 << channel;
  //check to see if the channel is a gpo already
  if ((_GPO_config & channel_byte) != channel_byte) {
    _GPO_config = _GPO_config ^ _GPO_config;
  }
  Wire.beginTransmission(_i2c_address);
  // write  to gpio-write register
  Wire.write(_ADAC_GPIO_WR_CONFIG);
  Wire.write(0x0);
  Wire.write(_GPI_config);
  Wire.endTransmission();

  if (_a0 > -1) digitalWrite(_a0, HIGH);
  AD5593R_PRINT("Channel ");
  AD5593R_PRINT(channel);
  AD5593R_PRINTLN(" is configured as a GPO");
}

void AD5593R::configure_GPOs(bool* channels) {
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (channels[i] == 1) {
      configure_GPO(i);
    }
  }
}


// bool AD5593R::read_GPI(byte channel) {


//   AD5593R_PRINT("Channel ");
//   AD5593R_PRINT(channel);
//   AD5593R_PRINT(" reads ");
//   AD5593R_PRINTLN(data);
//   return data;
// }

bool* AD5593R::read_GPIs() {

  if (_a0 > -1) digitalWrite(_a0, LOW);
  // request the data
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GPIO_READ);
  Wire.endTransmission();

  uint16_t data_bits = 0;
  Wire.requestFrom(int(_i2c_address), int(2), int(1));
  // mask bits, build the word
  if (Wire.available()) data_bits = (Wire.read() & 0x0f) << 8;
  if (Wire.available()) data_bits = data_bits | Wire.read();
  if (_a0 > -1) digitalWrite(_a0, HIGH);

  for (size_t i = 0; i < _num_of_channels; i++) {
    if (config.GPIs[i] == 1) {
      values.GPI_reads[i] = bool(data_bits & 0x01);
    }
    data_bits >> 1;
  }
  return values.GPI_reads;
}

void AD5593R::write_GPOs(bool* pin_states) {
  byte data_bits = 0;
  for (size_t i = 0; i < _num_of_channels; i++) {
    if (config.GPOs[i] == 1) {
      values.GPO_writes[i] = pin_states[i];
      data_bits = data_bits & pin_states[i];
    }
    data_bits << 1;
  }
  if (_a0 > -1) digitalWrite(_a0, LOW);
  Wire.beginTransmission(_i2c_address);
  Wire.write(_ADAC_GPIO_WR_DATA);
  Wire.write(0x00);
  Wire.write(data_bits);
  Wire.endTransmission();
  if (_a0 > -1) digitalWrite(_a0, HIGH);
}