
#include "AD5593R.h"

AD5593R AD5593R(23);
bool my_DACs[8] = {1,1,1,1,0,0,0,0};
bool my_ADCs[8] = {0,0,0,0,1,1,1,1};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  AD5593R.enable_internal_Vref();
  AD5593R.set_DAC_max_2x_Vref();
  AD5593R.set_ADC_max_2x_Vref();
  AD5593R.configure_DACs(my_DACs);
  AD5593R.write_DAC(0,1.25);
  AD5593R.write_DAC(1,2.5);
  AD5593R.write_DAC(2,3.75);
  AD5593R.write_DAC(3,5);
  AD5593R.configure_ADCs(my_ADCs);
  AD5593R.read_ADCs();
}


void loop() {

  
}
