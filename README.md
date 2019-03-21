# AD5593R-Library-Arduino-ESP32
This library allows for the configuration, writing, and reading of the 8 ADC/DAC channels of the AD5593R by analog instruments.
The AD5593R is connected to the master device through an i2c connection, so any board capable of programming through the Arduino IDE with an i2c bus will work.
By attaching a GPIO pin to the a0 pin of the AD5593R multiple chips can be addressed on the same bus.
