/*
Name: Joseph Yang
Date: 12/15/2022
Assignment: CSE341 Project 3
Purpose: Translate morse code to LCD. User input will be given from the matrixkeypad and 2 IR sensors
File: main.cpp, lcd1602.cpp, lcd1602.h
Inputs: Matrix Keypad, and 2 IR sensors
Output: Buzzer making noise and LCD display text

Pin Configurations:
Keypad: Pin1 to PD7, Pin5 to PD6, Pin6 to PD5, Pin7 to PD4
Buzzer: Vcc to PC10, I/O to PC11
Left IR: PC8
Right IR: PC9,
LCD: SDA to PB9, SCL to PB8
*/

//My Libraries
#include "Callback.h"
#include "EventQueue.h"
#include "PinNamesTypes.h"
#include "mbed.h"
#include <cstdio>
#include <string>
#include <map>
#include "lcd1602.h"

//Create Interrupt object with PullUp to default to 1
InterruptIn inter1(PC_8, PullUp);//trigger when first IR 
InterruptIn inter2(PC_9, PullUp);//trigger when second IR

//Create Interrupt object with PullDown to default to 0
InterruptIn inter3(PD_6, PullDown);//trigger when keypad 1 
InterruptIn inter4(PD_5, PullDown);//trigger when keypad 2
InterruptIn inter5(PD_4, PullDown);//trigger when keypad 3

CSE321_LCD lcd( 16, 2, LCD_5x8DOTS, PB_9, PB_8); //construct LCD Object

///////////////////////////////////////////////////////
//Global Variables
map<string, char>code;//Map to hold all morse code translations
string user_input = "";//From user input
string decode = "";//decode a single char
string msg = "";//message to the user
int num = 0; //Indicate what values are given from keypad

int signal = -1; //LCD output what signal is it
///////////////////////////////////////////////////////

//Create ISR, for input signal
void isr1(void);//Left IR
void isr2(void);//Right IR
void isr3(void);//Keypad 1
void isr4(void);//Keypad 2
void isr5(void);//Keypad 3

//////////////////////////////////////////////////////////////////////
//This section is to incorporate a thread using timer 
Ticker tick1;//create timeout object, interrupt driven ideas
Timer t; //user timer API
void app_run();//Function used for the timer
EventQueue q1(32 * EVENTS_EVENT_SIZE);//Create EventQueue for time
Thread tq1;//Create thread
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//This section is to use the event queue to protect the critical section
//This is done so they don't access the same variable called user_input
EventQueue q2(32 * EVENTS_EVENT_SIZE);//Create EventQueue, for isr1
EventQueue q3(32 * EVENTS_EVENT_SIZE);//Create EventQueue , for isr2
Thread t1; //thread for isr1
Thread t2; //thread for isr2
//////////////////////////////////////////////////////////////////////


//Create the watchdog, use keypad3 to reset the entire system, or 
//if the user hasn't done anything for 30 seconds then they are inactive
Watchdog &watchMe = Watchdog::get_instance();

Mutex stdio_mutex;//create mutex object to protect user input

int main()
{
    //Enable the watchdog
    watchMe.start();

    //Setup the the translator
    code = {{".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
            {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
            {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
            {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
            {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
            {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
            {"-.--", 'Y'}, {"--..", 'Z'}, {".----", '1'}, {"..---", '2'},
            {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'},
            {"--...", '7'}, {"---..", '8'}, {"----.", '9'}, {"-----", '0'}};

    //set the clock to port C
    RCC->AHB2ENR|=0x4;

    //set the clock to port D
    RCC->AHB2ENR|=0x8;

    //Set PC8 & 9 to input
    GPIOC->MODER&=~(0xF0000);

    //Set PD7 to output, provide power to the first row
    GPIOD->MODER &=~(0x8000);
    GPIOD->MODER |=(0x4000);
    GPIOD->ODR = 0x80;

    //Set PD6,5 & 4 to read input from keypad's 1, 2, and 3
    GPIOD->MODER &=~(0x3F00);

    lcd.begin();//Initalize LCD
    lcd.clear();//Clear display current cursor
    lcd.setCursor(0, 0);//put cursor at 0,0
    lcd.print("MSG:");
    lcd.setCursor(0, 1);//put cursor at 0,1
    lcd.print("Code:");

    //Timer
    tq1.start(callback(&q1, &EventQueue::dispatch_forever));
    tick1.attach(q1.event(app_run), 5);//run app_run function every 5 seconds
    t.start();//start the timer

    //Protect critical section by making sure the user input is synchronized
    t1.start(callback(&q2, &EventQueue::dispatch_forever));
    inter1.fall(q2.event(isr1));//Act upon falling edge
    inter1.enable_irq(); //enable the interrupt

    t2.start(callback(&q3, &EventQueue::dispatch_forever));
    inter2.fall(q3.event(isr2));//Act upon falling edge
    inter2.enable_irq(); //enable the interrupt

    inter3.rise(&isr3);//Act upon rising edge
    inter3.enable_irq(); //enable the interrupt

    inter4.rise(&isr4);//Act upon rising edge
    inter4.enable_irq(); //enable the interrupt
    inter5.rise(&isr5);//Act upon rising edge
    inter5.enable_irq(); //enable the interrupt

    //PC10 to output, used to control the buzzer to make noise
    GPIOC->MODER &=~(0x200000);
    GPIOC->MODER |=(0x100000);

    //PC11 to output, just there for Buzzer I/O
    GPIOC->MODER &=~(0x800000);
    GPIOC->MODER |=(0x400000);

    GPIOC->ODR = 0x800;//toggle on in PC11, this is Buzzer I/O, it doesn't really matter if it's power on or not

    while (true) {
        if(num == 1){
            //printf("1 is pressed\n"); uncomment this for test
            if(code.find(user_input) != code.end()){//if code matches, btw cite the find function
                decode = code[user_input];
                msg+=decode;
                //printf("%s\n", msg.c_str());//print in serial monitor, uncomment this for testing
            }
            else{//if user gives incorrect code
                lcd.clear();//Clear display current cursor
                lcd.setCursor(0, 0);//put cursor at 0,0
                lcd.print("Invalid Code!");
                wait_us(3000000);//Give some time for user to read

            }
            //Below here is to make LCD look nice
            lcd.clear();//Clear display current cursor
            lcd.setCursor(0, 0);//put cursor at 0,0
            lcd.print("MSG:");
            lcd.print(msg.c_str());
            lcd.setCursor(0, 1);//put cursor at 0,1
            lcd.print("Code:");
            user_input = "";//reset user input
        }
        if(num == 2){//If user made a mistake they can clear the code by pressing 2
            //printf("2 is pressed\n"); uncomment this for testing
            lcd.clear();//Clear display current cursor
            lcd.setCursor(0, 0);//put cursor at 0,0
            lcd.print("MSG:");
            lcd.print(msg.c_str());
            lcd.setCursor(0, 1);//put cursor at 0,1
            lcd.print("Code:");
            user_input = "";//reset user input
        }
        if(num == 3){//By pressing 3 user can clear all the messages and code 
            //printf("3 is pressed\n"); uncomment this for testing
            lcd.begin();//Initalize LCD
            lcd.clear();//Clear display current cursor
            lcd.setCursor(0, 0);//put cursor at 0,0
            lcd.print("MSG:");
            lcd.setCursor(0, 1);//put cursor at 0,1
            lcd.print("Code:");
            msg = "";//reset
            user_input = ""; //reset
        }
        if(user_input.length() == 6){//Max code length is 5, if it's 6 then it's incorrect
            lcd.clear();//Clear display current cursor
            lcd.setCursor(0, 0);//put cursor at 0,0
            lcd.print("Invalid Code!");
            wait_us(3000000);//Give some time for user to read
            //Below here is to make LCD look nice
            lcd.clear();//Clear display current cursor
            lcd.setCursor(0, 0);//put cursor at 0,0
            lcd.print("MSG:");
            lcd.print(msg.c_str());
            lcd.setCursor(0, 1);//put cursor at 0,1
            lcd.print("Code:");
            user_input = "";//reset user input
        }
        if(signal == 0){//When left IR senses something
            user_input += '.';//append to user input
            lcd.print(".");//print to LCD
        }
        if(signal == 1){//When right IR senses something
            user_input += '-';//append to user input
            lcd.print("-");//print to LCD
        }

        //Reset
        num = 0;
        signal = -1;
    }

}

void isr1(void){// if PC8 IR set low it gives '.'
    stdio_mutex.lock();//Use mutex to protect the signal(aka user input)
    watchMe.kick();//reset the watchdog
    GPIOC->ODR = 0x400;//toggle on in PC10, this is Buzzer, vcc
    wait_us(800000); //used to address bounce, also if I press too long I dont want to append extra char
    GPIOC->ODR &= ~(0x400);//toggle buzzer off
    signal = 0;
    stdio_mutex.unlock();//Use mutex to protect the signal(aka user input)
}
void isr2(void){// if PC9 IR set low it gives '-'
    stdio_mutex.lock();//Use mutex to protect the signal(aka user input)
    watchMe.kick();//reset the watchdog
    GPIOC->ODR = 0x400;//toggle on in PC10, this is Buzzer, vcc
    wait_us(800000); //used to address bounce, also if I press too long I dont want to append extra char
    GPIOC->ODR &= ~(0x400);//toggle buzzer off
    signal = 1;
    stdio_mutex.unlock();
}

void isr3(void){// if PD6 gets input, translate user input, add 0.5 second delay
    watchMe.kick();//reset the watchdog
    wait_us(800000); //used to address bounce, also if I press too long I dont want to append extra char
    num = 1;
}
void isr4(void){// if PD5 gets input, clear user input
    watchMe.kick();//reset the watchdog
    num = 2;
}
void isr5(void){// if PD4 gets input, clear lcd to default
    watchMe.kick();//reset the watchdog
    num = 3;
}
void app_run(){//print in serial monitor how long the program is running
    printf("You have been using this for: %llu seconds\n", t.elapsed_time()/1000000);//Print in seconds
}