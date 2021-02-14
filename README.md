# AD5593R-Library-Arduino-ESP32
This library allows for the configuration, writing, and reading of the 8 ADC/DAC/GPIO channels of the [AD5593R](https://www.analog.com/media/en/technical-documentation/data-sheets/AD5593R.pdf) by analog instruments.


## Connecting To the Microcontroller
- The [AD5593R](https://www.analog.com/media/en/technical-documentation/data-sheets/AD5593R.pdf) is connected to the master device through an I2C connection. Any board capable of programming through the Arduino framework with an I2C bus will work.
- By attaching a GPIO pin to the a0 pin of the AD5593R multiple chips can be addressed on the same bus.
  - Assigning this pin in the class constructor maps the microcontroller's gpio to the ADC
