
/* 
 * This example shows how to use the Menu Library when you use your own Keypad
 * NOTE : I don't own that keypad, so it was not tested. 
 *        If you do, please email me your experience at: j-bellavance@videotron.ca
 * Author : Jacques Bellavance
 * Version 1.00: September 12, 2016
 * Version 2.00 : November 11, 2016
 * Added the possibility to let the Menu Library manage both the LCD and the keypad
 */

#include <Menu.h>
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define ANALOG 1
#define DIGITAL 0
#define PINMOTOR 26

///////////////////////////////////////////////////////////////////////////////////////////////////THE LCD
int lcdNumCols = 20;
int lcdNumRows = 4;

//Depending on the LDC library that you are using, comment out the other one

//See: https://www.arduino.cc/en/Tutorial/HelloWorld
#include <LiquidCrystal.h>                     //Include for LiquidCrystal
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);           //Constructor for LiquidCrystal

//See: https://github.com/johnmccombs/arduino-libraries/tree/master/LiquidTWI
//#include <Wire.h>
//#include <LiquidTWI.h>                         //Include for LiquidTWI
//LiquidTWI lcd(0);                              //Constructor for LiquidTWI

//If you use another LCD Library, place the include and instantiation code here

////////////////////////////////////////////////////////////////////////////////////////////////THE KEYPAD
#define UP '2'
#define DOWN '8'
#define LEFT '4'
#define RIGHT '6'

#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
   {'1','2','3'},
   {'4','5','6'},
   {'7','8','9'},
   {'#','0','*'}
};
byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 11, 12}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//////////////////////////////////////////////////////////////////////////////////////////////////THE MENU
const String menuItems = 
"-READ PINS:000"
"--SENSORS:000"
"---SENSOR A1:101"
"---SENSOR A2:102"
"--SWITCHES:000"
"---SWITCH PIN 24:103"
"---SWITCH PIN 26:104"
"-MOTOR:000"
"--START:105"
"--STOP:106";
Menu menu(menuItems); //Set up menu

////////////////////////////////////////////////////////////////////////////////////////ACTIONS 101 to 104
void readPin(int pin, int pinType) {
  int key = 0;                                          //Initialise key as NoKeyPressed
  lcd.clear();                                          //Clear the LCD
  lcd.setCursor(0,0);                                   //On the first row
  lcd.print(menu.getCurrentLabel());                    //Print the label for that menu item.
  while(key != LEFT && key != RIGHT) {                  //Exit on LEFT or RIGHT.
    lcd.setCursor(0, 1);                                  //On the second row
    lcd.print("   ");                                     //Erase the old value
    lcd.setCursor(0, 1);                                  //On the second row
    if (pinType == DIGITAL)                               //If it is a digital pin,
      lcd.print(digitalRead(pin) ? "HIGH" : "LOW");         //Print HIGH or LOW
    else                                                  //If it is an analog pin, (not digital)
      lcd.print(analogRead(pin));                           //Print value (0:1023)
    key = menu.readKey();                                 //Read the keypad.
    delay(100);                                           //Reduce flickering.
    
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////ACTION SELECT
void make(int action) {
  switch (action) {
    case 101: { readPin(A1, ANALOG); break; }
    case 102: { readPin(A2, ANALOG); break; }
    case 103: { readPin(22, DIGITAL); break; }
    case 104: { readPin(24, DIGITAL); break; }
    case 105: { digitalWrite(PINMOTOR, HIGH); break; }
    case 106: { digitalWrite(PINMOTOR, LOW); break; }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////SETUP
void setup() {
  Serial.begin(9600);
  lcd.begin(lcdNumCols, lcdNumRows);                   //Initialize the LCD
  menu.handleLcd(&lcd, lcdNumCols, lcdNumRows);        //Give a pointer of your LCD to the Menu Library
  menu.mapKeys(UP, DOWN, LEFT, RIGHT);                //Give the pins number of your digital keypad

  pinMode(22,INPUT_PULLUP);      //Setup for the actions
  pinMode(24,INPUT_PULLUP);
  pinMode(PINMOTOR, OUTPUT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////LOOP
void loop() {
  int action = 0;
  menu.showMenu();                 //Update the LCD with the menu.
  char myKey = keypad.getKey();
  if (myKey == UP or myKey == DOWN or myKey == LEFT or myKey == RIGHT) 
    action = menu.update(myKey);
  if (action > 0) {                //If we need to act:
    make(action);                    //make it.
    menu.done();                     //We are done with this action... Go back to the menu.
  }
}

