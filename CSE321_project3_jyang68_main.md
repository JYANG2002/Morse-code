# Joseph Yang CSE321 Project 3

## Project Overiew

---

The purpose of this project is to translate morse code into text displayed on the LCD. User input will be determined by the matrix keypad and the IR sensor. With every input the IR detects the buzzer will make a noise to sound like a telegraph. Keypads are there to give the user options to clear the LCD or display text.

## Required Materials

---

- NUCLEO-L552ZE-Q
- Matrix Keypad
- LCD
- 1 Breadboard
- Jumper Wires
- Buzzer
- 2 IR sensors

## Resources and References

- https://os.mbed.com/docs/mbed-os/v6.15/introduction/index.html
- https://www.st.com/resource/en/user_manual/dm00368330-stm32-nucleo-144-boards-mb1312-stmicroelectronics.pdf
- https://www.st.com/resource/en/reference_manual/dm00310109-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

---

## Getting Started

1. Connect first row pin of matrix keypad to PD7 using jumper wires.
2. Connect column one pin to PD6, column two pin to PD5, and column three pin to PD4 using jumper wires.
3. Connect 5V from the nucleo to the breadboard's red power rail.
4. Connect the GND from the nucelo to the breadboard's blue power rail.
5. Connect the LCD's SDA to PB9, and SCL to PB8. Finally connect the the GND to the breadboard's blue power rail and Vcc to the red power rail.
6. Place the buzzer on the breadboard. Connect Vcc to PD10, it's I/O to PC11, and it's GND to blue power rail.
7. Place two IR sensors on the breadboard. Connect both IR sensor's GND to blue power rail and Vcc to red power rail.
8. Connect the left IR sensor's OUT to PC8 and the right sensor to PC9.

---

## Declarations

- map<string, char>code; //Map to hold all morse code translations
- string user_input = ""; //Place to hold user input code
- string decode = ""; //Place to hold the decoded char
- string msg = ""; //message to the user
- int num = 0; //Indicate what values are given from keypad

---

## APIs and Elements

- Mbed API
  - Ticker
  - Timer
  - Thread
  - EventQueue
  - Watchdog
  - Mutex
- LCD API
  - lcd1602.cpp
  - lcd1602.h files

---

## Custom Function

- void isr1(void); //When left IR senses input, append to user_input '.' and make buzzer make noise
- void isr2(void); //When right IR senses input, append to user_input '-' and make buzzer make noise
- void isr3(void); //When keypad 1 is pressed, translate user input to alphanumeric characters
- void isr4(void); //When keypad 2 is pressed, clear user_input in case user made a mistake
- void isr5(void); //When keypad 3 is pressed, clear user input and message in the LCD
- void app_run(); //Prints in serial monitor how long the program has been running for

---

## Things to improve

Reduce the required IR sensor to be 1 in order to make the project more accurate as a morse code telegraph machine
