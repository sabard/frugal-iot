# frugal-iot
A platform to enable affordable sensor networks

For now see the Google Doc: [Frugal IoT - enabling low cost sensor networks](https://docs.google.com/document/d/1hOeTFgbbRpiKB_TN9R2a2KtBemCyeMDopw9q_b0-m2I/edit?usp=sharing)
Scroll down to the bottom for links to other docs. 

Also see the subdirectory ./docs


# Prerequisites

Install Arduino IDE (https://www.arduino.cc/en/software)

Tools->Boards Manager->
  - search esp32
  - install esp32 by Espressif Systems (tested version 3.0.4)

Tools->Board: ESP32 Dev Module or your specific chip

Set JTAG Adapter if using FTDI adapter: (https://www.amazon.com/DSD-TECH-SH-U09C2-Debugging-Programming/dp/B07TXVRQ7V)

# Notes

Needed to manually set ACTUATOR_BLINKIN_PIN
