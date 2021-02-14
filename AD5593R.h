/*
This library is for the AD5593R by analog instruments please refer to the datasheet
https://www.analog.com/media/en/technical-documentation/data-sheets/AD5593R.pdf

The AD5593R is a powerful 12-bit configurable DAC/ADC/GPIO chip connected through an I2C bus.
The chip also has an additional addressing pin dubbed a0, which will change the device's effectively
I2C address, by connecting this pin and specifying its location in the class construction it is possible to
connect several AD5593Rs on the same I2C bus and independently control them.

To enable the ADC/DAC functionality of the chip a reference voltage MUST be set, as all set voltages
must fall into the range 0-Vref, or 0-2xVref if set_(ADC/DAC)_max_2x_Vref is called. If these functions
return negative values please read their description as the cause will be reported by its value.

GPIO capabilities have not yet been added.

Lukas Janavicius, 2019
For contact information, projects, or more about me, please visit my GitHub or website linked below.

Janavicius.org
https://github.com/LukasJanavicius

*/

//////Definitions and imports//////

// To enable debugging please place the following before the library import,
// and be sure to use Serial.begin() in the setup.
//  AD5593R_DEBUG
#pragma once
//comment this line to disable debugging
//#define AD5593R_DEBUG

#ifdef AD5593R_DEBUG
#define AD5593R_PRINT(...)    Serial.print(__VA_ARGS__)
#define AD5593R_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#define AD5593R_PRINT(...)
#define AD5593R_PRINTLN(...)
#endif

#ifndef AD5593R_h
#define AD5593R_h
#endif
#include <Arduino.h>


//////Classes//////
class AD5593R {
public:

  // This configuration structure contains arrays of booleans,
  // each array follows the form [channel0,...,channel7]
  // where a  1 indicates the channel should be configured as the name implies
  // for example an array ADCs[8] = {1,1,0,0,0,0,0,0} will configure channels 0 and 1 as ADCs.
  // a declaration of this structure should be defined in your code, and passed into configure().
  // You should not double assign pins, as only the first declaration will be assigned.
  struct configuration {
    bool ADCs[8]; //ADC pins
    bool DACs[8]; //DAC pins
    bool GPIs[8]; //input pins
    bool GPOs[8]; //output pins
  };
  configuration config;

  // This structure contains arrays of
  struct Read_write_values {
    float ADCs[8];
    float DACs[8];
    bool GPI_reads[8];
    bool GPO_writes[8];
  };
  Read_write_values values;
  // constructor for the class, a0 is the digital pin connected to the AD5593R
  // if no pin is specified it is assumed only one AD5593R is connected
  AD5593R(int a0 = -1);

  // enables the internal reference voltage of 2.5 V
  void enable_internal_Vref();

  // disables the internal reference voltage of 2.5 V
  void disable_internal_Vref();

  // sets the maximum ADC input to 2x Vref
  void set_ADC_max_2x_Vref();

  // sets the maximum ADC input to 1x Vref
  void set_ADC_max_1x_Vref();

  // sets the maximum DAC output to 2x Vref
  void set_DAC_max_2x_Vref();

  // sets the maximum DAC output to 2x Vref
  void set_DAC_max_1x_Vref();

  // If you use an external reference voltage you should call this function. Failure to set the reference voltage,
  // or enable the internal reference will mean that any DAC/ADC function call will result in an error!
  void set_Vref(float Vref);

  //configures the selected channel as a DAC
  void configure_DAC(byte channel);

  void configure_DACs(bool* channels);
  // Sets the output voltage value of a given channel, returns 1 if the write is completed
  // if the function returns -1 if the specified channel is not an DAC,
  // if no reference voltage is specified a -2 will be returned,
  // and if the voltage exceeds the maximum allowable voltage a -3 will be returned.
  int write_DAC(byte channel, float voltage);

  void write_DACs(float* voltages);

  //configures the selected channel as a ADC
  void configure_ADC(byte channel);


  void configure_ADCs(bool* channels);

  // Reads the voltage value of a given ADC channel, returns the Voltage if the write is completed
  // if the function returns -1 if the specified channel is not an ADC,
  // and if no reference voltage is specified a -2 will be returned.
  float read_ADC(byte channel);

  float* read_ADCs();


  void configure_GPI(byte channel);
  void configure_GPIs(bool* channels);

  void configure_GPO(byte channel);
  void configure_GPOs(bool* channels);

  bool* read_GPIs();
  void write_GPOs(bool* pin_states);



  /*

  // By passing in the configuration structure this function assigns the functionality
  // to each pin, as described in the configuration. As stated above, you should not
  // assign multiple functionalities to a single pin. In this event the function will return
  // -1 and print an error if debug is enabled. A 1 will be returned if the configuration is successful
  int configure_pins(*configuration config);

  //call this function in
  void update(DAC_Writes[8],ADC_Reads[8]);

  // performs a software reset, generally a good idea to call in the setup
  void reset();


  // Reads the set value of a given DAC channel, -1 will be returned if
  float read_DAC(int channel);

  //This follows the same functionality as read_ADC(), but instead of reading from a single channel
  //the configuration is passed in, so that all of the ADCs are read.
  // In order to extract the values read you should pass in your Read_write_values struct.
  float read_DACs(int channels[8], *Read_write_values output);

  //power down a specific channel, if none is specified all channels are powered down.
  void power_down(int channel = -1);
  */
private:
  // checks if the given channel is configured as an ADC
  // returns 1 if the channel is configured, 0 if the channel is not
  bool is_ADC(int channel);

  // checks if the given channel is configured as a DAC
  // returns 1 if the channel is configured, 0 if the channel is not
  bool is_DAC(int channel);

  // pointers for configuring the control registers, refer to the data sheet for functionality
  // These structures are adapted from
  // https://github.com/MikroElektronika/HEXIWEAR/blob/master/SW/Click%20Examples%20mikroC/examples/ADAC/library/__ADAC_Driver.h


  int _num_of_channels = 8;

  int _a0;

  //general purpose control register data Bytes
  byte _GPRC_msbs;
  byte _GPRC_lsbs;

  // power control register data bytes;
  byte _PCR_msbs;
  byte _PCR_lsbs;

  byte _DAC_config;
  byte _ADC_config;
  byte _GPI_config;
  byte _GPO_config;

  //default address of the AD5593R, multiple devices are handled by setting the desired device's a0 to LOW
  //by default the a0 pin will be pulled high, effectively changing its address. For more information on the addressing please
  //refer to the data sheet in the introduction
  byte _i2c_address = 0x10;

  //Value of the reference voltage, if none is specified then all ADC/DAC functions will throw errors
  float _Vref = -1;

  //flag for 2xVref mode
  bool _ADC_2x_mode = 0;

  //flag for 2xVref mode
  bool _DAC_2x_mode = 0;

  float _ADC_max = -1;

  float _DAC_max = -1;
};
