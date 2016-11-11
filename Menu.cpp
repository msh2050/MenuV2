/*
 * Menu.cpp
 * Author : Jacques Bellavance
 * Date : August 27, 2016
 * Revision : Added support for LCDs and switches
 * Version : 2.00
 */

 /*
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General
 Public License along with this library; if not, write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA  02111-1307  USA
 */

/*
 * Menu.cpp
 * Allows the use of menus in sketches.
 * The menu is declared in a String. 
 * (See menuParse() below for instructions on how a menu is built) 
 * The menu is parsed, allowing easy navigation thru the menus
 * The library can handle the LCD and the switches that allows navigating the menus.
 * Supported LCDs:
 * LiquidCrystal.h      See: https://www.arduino.cc/en/Tutorial/HelloWorld
 * LiquidTWI.h          See: https://github.com/johnmccombs/arduino-libraries/tree/master/LiquidTWI
 * Supported Keypads:
 * Four keys, one digital pin per key     See the tutorial
 * One analog pin for the four switches   See the tutorial
 */
 
#include <Arduino.h>
#include <Menu.h>

//Constructor=================================================================
Menu::Menu(String items) {
  MYitems = items;  //Full menu
	//Find the number of items of the menu (count the colons)
	//and allocate memory for the nodes
	int count = 0;
	for (int i = 0; i < MYitems.length(); i++)
		if (MYitems.charAt(i) == ':') count++;
	nodes = (node*) calloc(count + 1, sizeof(node));
	nodes[0].eldest = 1;  //The first item in the menu is the eldest of nodes[0]

	menuParse();          //Parse "MYitems" in "nodes[]"
  currentNode = 1;      //Set first node as curent
}//Constructor-----------------------------------------------------------------

 //menuParse====================================================================================================================
//The full menu is built in the sketch in the following mannner :
//  String menuItems = 
//  "-READ:000"
//  "--SENSORS:000"
//  "---SENSOR A1:101"
//  "---SENSOR A2:102"
//  "--SWITCHES:000"
//  "---SWITCH PIN 4:103"
//  "---SWITCH PIN 5:104"
//  "-SET:000"
//  "--SERVO ARM:105"
//  "--SERVO BASE:106"
//  "-MOVE:107";
//The menu items contains 4 parts:
//  a series of dashes that indicate the item's level
//  the label of the item
//  a colon (:) (token)
//  a 3 digits number between "000" and "999" ("000" means: I have a submenu) to tag an action to be performed in the sketch
//In order to navigate the menu, each item is associated to a node : 
//  struct node {       //For each item :
//    int starts = 0;     //The index of the start of the label in MYitems
//    int ends = 0;       //The index of the end of the label in MYitems
//    int parent = 0;     //The node number of the parent of this item
//    int eldest = 1;     //The node number of the eldest child of this item
//    int action = 0;     //The action associated to this item
//  };
//-----------------------------------------------------------------------------------------------------------------------------
void Menu::menuParse() {
  int stack[8] = { 0,0,0,0,0,0,0,0 }; //Stack (Last-in/First-out) (parent management)
  int stackPtr = 7;                   //Stack pointer
  int pos = 1;                        //The position of the pointer in the string "MYitems"
  int item = 1;                       //The pointer to the current item
  int curLevel = 1;                   //The level of the current item
  int nextLevel = 1;                  //The level of the next item
  int len = MYitems.length();         //The length of "MYitems"
  
  while(pos < len) {                                               //Parse the whole menu
    nodes[item].eldest = 1;                                          //Default value. "I have no child"
    nodes[item].starts = pos;                                        //The start of the label
    while(MYitems.charAt(pos) != ':') pos++;                         //Forward to the ":" token
    nodes[item].ends = pos;                                          //The end of the label
    nodes[item].action = MYitems.substring(pos+1, pos+4).toInt();    //The integer associated to the action
	  nodes[item].parent = stack[stackPtr];                            //The parent of the item (current on stack)
	  pos += 4;                                                        //Forward to the next item
    nextLevel = 0 ;
    while(MYitems.charAt(pos) == '-') { pos++; nextLevel++; }        //Find the level of the next item (count dashes)
    if (nextLevel > curLevel) {                                      //If the next item has a higher level (item is a parent)
		  stackPtr--; stack[stackPtr] = item;                              //Push the item as a parent on the stack
      nodes[item].eldest = item + 1;                                   //The next item is the eldest child of the current item
    }
    if (nextLevel < curLevel) {                                      //If the next item has a lower level
		  for (int i = nextLevel; i < curLevel; i++) {                     //For the number of generations 
		    stack[stackPtr] = 0;  stackPtr++;                                //Pull as many parents as necessary off the stack 
      }                              
    }
    item++; curLevel = nextLevel;                                   //Go to next item
  }
  lastNode = item-1;                                                //Set "lastNode" to the number of items in the menu
}//menuParse-------------------------------------------------------------------------------------------------------------------

//label=====================================================================
//Return the label of the item "node"
//--------------------------------------------------------------------------
String Menu::label(int node) {
  return MYitems.substring(nodes[node].starts, nodes[node].ends);
}//label--------------------------------------------------------------------

//parent===================================================
//Return the number of the parent of "node"
//---------------------------------------------------------
int Menu::parent(int node) {
    return nodes[node].parent;
}//parent--------------------------------------------------

//eldest============================================
//Return the eldest child of "node"
//--------------------------------------------------
int Menu::eldest(int node) {
  return nodes[node].eldest;
}//eldest-------------------------------------------

//previousSibling===================================================
//If "node" is the eldest, return "node".
//Otherwise, return it's older sibling.
//------------------------------------------------------------------
int Menu::previousSibling(int node) {
  if (eldest(parent(node)) == node) return node;
  for (int i = node-1 ; i > 0 ; i--) {
    if (parent(i) == parent(node)) return i;
  }
}//previousSibling--------------------------------------------------

//nextSibling=====================================================
//Return the next sibling of "node",
//or "node" if it is the youngest
//----------------------------------------------------------------
int Menu::nextSibling(int node) {
  for (int i = node+1 ; i <= lastNode ; i++) {
    if (parent(i) == parent(node)) return i;
  }//Passed this point, no older brother has been found.
  return node;
}//nextSibling----------------------------------------------------

//rank===========================================================
//Returs the rank of "node" amongst it's siblings.
//---------------------------------------------------------------
int Menu::rank(int node) {
	int child = eldest(parent(node)); //From the eldest child...
	int rankOfNode = 1;
  while (child != node) {
    child = nextSibling(child);
    rankOfNode++;
  }
  return rankOfNode;
}//rank----------------------------------------------------------

//siblingsCount========================================================================
//returns the number of siblings of "node"
//-------------------------------------------------------------------------------------
int Menu::siblingsCount(int node) {
  int parentOfNode = parent(node);
	int child = eldest(parentOfNode); //From the eldest child...
  int count = 0;
  for (int i = child ; i <= lastNode; i++) if (parent(i) == parentOfNode) count++;
  return count;
}//siblingsCount------------------------------------------------------------------------

//getCurrentItem==========================================
//Returns the current node
//--------------------------------------------------------
int Menu::getCurrentItem() {
	return currentNode;
}//getCurrentItem-----------------------------------------
 
//getCurrentItem==========================================
 //Sets the menu item with theLabel as the current item
 //-------------------------------------------------------
void Menu::setCurrentItem(String theLabel) {
  int foundAt = 0;
	for (int i = 1; i <= lastNode; i++) {
    if (label(i) = theLabel) foundAt = i;
	}
  if (foundAt > 0) currentNode = foundAt;
}//setCurrentItem------------------------------------------


//getAction==========================================
//Returns the action associated to the current node
//---------------------------------------------------
int Menu::getAction() {
  return nodes[currentNode].action;
}//getAction-----------------------------------------

//getCurrentLabel========================
//Returns the label of the current item
//---------------------------------------
String Menu::getCurrentLabel() {
  return label(currentNode);
}//getCurrentLabel-----------------------

//restart==================
//Reinitialise the menu
//-------------------------
void Menu::restart() {
  currentNode = 1;
}//restart-----------------

//updated===============================
//Signals that the LCD has been updated
//--------------------------------------
void Menu::updated() {
  lcdNeedsUpdate = false;
}//updated------------------------------

//needsUpdate===============================
//Signals that the LCD needs to be updated
//------------------------------------------
bool Menu::needsUpdate() {
  return lcdNeedsUpdate;
}//needsUpdate------------------------------

//updateLcd===========================================
//Signal the Library that the LCD needs to be updated
//----------------------------------------------------
void Menu::updateLcd() {
	lcdNeedsUpdate = true;
}//updateLcd------------------------------------------

 //handleLcd==============================================================
 //Allows the sketch to pass a pointer to it's lcd object
 //From then on, the library will handle the menu portion on the LCD
 //This is the <LiquidCrystal.h> option
 //-----------------------------------------------------------------------
void Menu::handleLcd(LiquidCrystal *Lcd, int columns, int rows) {
	MYLcd = Lcd;      //Get hold of the pointer
	LCDcol = columns; //Number of columns of the sketche's LCD
	LCDrows = rows;   //Number of rows of the sketche's LCD
	LcdId = LcdIsFourPins;          //Remember which Library we are using
	handelingLcd = true;            //The lcd is ours to display
	showMenu();                     //So display it.
}//handleLcd--------------------------------------------------------------

//handleLcd===============================================================
//Allows the sketch to pass a pointer to it's lcd object
//From then on, the library will handle the menu portion on the LCD
//This is the <LiquidTWI.h> option
//------------------------------------------------------------------------
void Menu::handleLcd(LiquidTWI *Lcd, int columns, int rows) {
  MYLcdTWI = Lcd;   //Get hold of the pointer
  LCDcol = columns; //Number of columns of the sketche's LCD
  LCDrows = rows;   //Number of rows of the sketche's LCD
	LcdId = LcdIsTWI;                  //Remember which Library we are using
	handelingLcd = true;               //The lcd is ours to display
  showMenu();                        //So display it.
}//handleLcd---------------------------------------------------------------

//defineLcd====================================================
//The number of columns and lines of the sketche's LCD
//This is used if the sketch is handeling the LCD
//-------------------------------------------------------------
void Menu::defineLcd(int columns, int rows) {
	LCDcol = columns; //Number of columns of the sketche's LCD
	LCDrows = rows;   //Number of rows of the sketche's LCD
  handelingLcd = false;
}//defineLcd---------------------------------------------------

//lcdLine==================================================================================================================================
//Return a string containing the label of the item to be displayed
//on the "requested Line" on the lcd for the current menu or submenu.
//Preceded by a caret ">" if the item is the current item.
//-----------------------------------------------------------------------------------------------------------------------------------------
String Menu::lcdLine(int requestedLine) {
	int currentRank = rank(currentNode);                              //Where the current node is amongst it's siblings
  if((requestedLine + 1) > siblingsCount(currentNode)) return "";   //There is no item at this rank in the menu
	int child = eldest(parent(currentNode));                          //From the eldest
	int targetRank = requestedLine + 1;                               //Case where the LCD line 0 displays the eldest
	if (currentRank >= LCDrows) targetRank += currentRank - LCDrows;  //If not, add the difference between the the item and LCD's line count
	for (int i = 1; i < targetRank; i++)  child = nextSibling(child); //Find the item at "targetRank"
	if (currentRank == targetRank) return '>' + label(child);         //If it is the curent item, add a ">" before the label
	else                           return ' ' + label(child);         //If not, add a " " before the label
}//cdLine-----------------------------------------------------------------------------------------------------------------------------------

//toLCD==========================================================================
//Sends the string "msg" to the current LCD at column "col" and row "row"
//-------------------------------------------------------------------------------
int Menu::toLCD(String msg, int col, int row) {
	switch(LcdId) {
		case 1: { MYLcd->setCursor(col, row); MYLcd->print(msg); break; }
    case 2: { MYLcdTWI->setCursor(col, row); MYLcdTWI->print(msg); break; }
	}
}//toLCD-------------------------------------------------------------------------

//clearLCD==========================
//Clears the current LCD
//----------------------------------
void Menu::clearLCD() {
	switch (LcdId) {
	case 1: { MYLcd->clear(); break; }
  case 2: { MYLcdTWI->clear(); break; }
	}
}//clearLCD--------------------------

//showMenu==============================================================
//Sends the current menu or submenu to the LCD
//Is executed only when it needs an update.
//----------------------------------------------------------------------
void Menu::showMenu() {
  if (lcdNeedsUpdate) {
    clearLCD();
	  for (int i = 0 ; i < LCDrows ; i++)  toLCD(lcdLine(i), 0, i);
    delay(100);
  lcdNeedsUpdate = false;
  }
}//showMenu-------------------------------------------------------------

//handleSwitches===================================================================
//Allows the sketch to pass the pin layout of the arrow switches
//From then on, the library will handle reading the switches to navigate the menu
//This is the Digital (one pin per switch) option
//---------------------------------------------------------------------------------
void Menu::handleSwitches(int keyUP, int keyDOWN, int keyLEFT, int keyRIGHT) {
  MYUP = keyUP;                   //Set the library's variables
  MYDOWN = keyDOWN;
  MYLEFT = keyLEFT;
  MYRIGHT = keyRIGHT;
  pinMode(MYUP, INPUT_PULLUP);    //Initialise the pins
  pinMode(MYDOWN, INPUT_PULLUP);
  pinMode(MYLEFT, INPUT_PULLUP);
  pinMode(MYRIGHT, INPUT_PULLUP);
  handelingSwitches = true;
  switchesAreAnalog = false;
}//handleSwitches-------------------------------------------------------------------

//handleSwitches===================================================================
//Allows the sketch to pass the pin layout of the arrow switches
//From then on, the library will handle reading the switches to navigate the menu
//This is the Analog (one pin for all the four switches) option
//---------------------------------------------------------------------------------
void Menu::handleSwitches(int analogPin) {
  MYANALOG = analogPin;
  pinMode(MYANALOG, INPUT);
  handelingSwitches = true;
  switchesAreAnalog = true;
}//handleSwitches-------------------------------------------------------------------

//mapKeyChar===========================================================
//The sketch handles the LCD
//This is used if the keypad returns characters
//Most matrix keypads use this scheme.
//---------------------------------------------------------------------
void Menu::mapKeys(char UP, char DOWN, char LEFT, char RIGHT) {
	MYCHARUP = UP;
	MYCHARDOWN = DOWN;
	MYCHARLEFT = LEFT;
	MYCHARRIGHT = RIGHT;
  handelingSwitches = false;
  keysAreIntegers = false;
}//mapKeyChars---------------------------------------------------------

//mapKeyInt===========================================================
//The sketch handles the LCD
//This is used if the keypad returns integers
//--------------------------------------------------------------------
void Menu::mapKeys(int UP, int DOWN, int LEFT, int RIGHT) {
	MYINTUP = UP;
	MYINTDOWN = DOWN;
	MYINTLEFT = LEFT;
	MYINTRIGHT = RIGHT;
	handelingSwitches = false;
  keysAreIntegers = true;
}//mapKeyInt---------------------------------------------------------

//readDigitalKey=======================================================
//Reads the switch on pin "key". (debounced)
//500 positive counts can handle most switches in a normal environment.
//If noise still occurs, increase the number of counts.
//---------------------------------------------------------------------
int Menu::readDigitalKey() {
  int key = 0;
  int pinKey = 0;
  //find if a key is pressed
  if (digitalRead(MYUP) == LOW) { pinKey = MYUP; key = UP; }
  if (digitalRead(MYDOWN) == LOW) { pinKey = MYDOWN; key = DOWN; }
  if (digitalRead(MYLEFT) == LOW) { pinKey = MYLEFT; key = LEFT; }
  if (digitalRead(MYRIGHT) == LOW) { pinKey = MYRIGHT; key = RIGHT; }
  if (key > 0) { //And debounce it
	  int count = 0;
	  int stateNew;
	  int stateOld = digitalRead(pinKey);
	  while (count < 500) {
	    stateNew = digitalRead(pinKey);
	    if (stateNew == stateOld) count++; else count--;
	  }
  }
  return key;
}//readDigitalKey------------------------------------------------------

//readAnalogKey==================================================================================
//Reads the switch on analog pin "MYANALOG". (debounced)
//Averaging 300 reads can handle most switches in a normal environment.
//If noise still occurs, increase the number of counts.
//The 0:1023 range of digitalRead() is divided in four bins of size 256.
//The center of each bin (around 134, 368, 658 and 893) is returned by analogRead(MYANALOG)
//if the proper resistances are used : Pullup = 10K, Switches : 1.5K, 5.6K, 18K, 68K).
//A value of over 1000 (much higher than 893) means that no key was pressed (idealy 1023).
//Otherwise, casting to an integer the average divided by 256 yelds 0, 1, 2 or 3
//Adding one gives us the proper key.
//-----------------------------------------------------------------------------------------------
int Menu::readAnalogKey() {
  int average = analogRead(MYANALOG);
  for (int i = 0 ; i<300 ; i++) {
    int key = analogRead(MYANALOG);
    average = int((average + key) / 2);
  }
  if (average > 1000) return 0;
  else                return int(average / 256) + 1;
}//readAnalogKey---------------------------------------------------------------------------------

//readKey===============================================
//Returns the key ID or 0 if no key was pressed
//------------------------------------------------------
int Menu::readKey() {
	if (switchesAreAnalog) return readAnalogKey();
	else                   return readDigitalKey();
}//readKey----------------------------------------------

//readKeyWithRepeat=======================================================================================
//Returns the key ID or 0 if no key was pressed
//DelayForRepeat is the number of milliseconds before the function starts sending repeatedly
//Sensitivity is the number of times a key is probed between each send after delayForRepeat has expired
//--------------------------------------------------------------------------------------------------------
int Menu::readKeyWithRepeat(int delayForRepeat, int sensitivity) {
	static bool repeating = false;  //static variables retain their values between calls
	static unsigned long chrono;
  static int key = 0;
  int k = 0;
  int tempKey = 0;

	tempKey = readKey();
  if (tempKey != key) chrono = millis(); //First time around

  while (true) {
    while ((!repeating) && (millis() - chrono < sensitivity)) {} //wait
		k = tempKey;
		tempKey = readKey();
		if (tempKey == 0) {
      key = 0;
      chrono = millis();
      repeating = false; 
      return k;
    }
    if ((!repeating) && (millis() - chrono >= delayForRepeat)) {
      repeating = true;
      key = tempKey;
      chrono = millis();
      return key;
    }
		if ((repeating) && (millis() - chrono >= sensitivity)) {
      key = tempKey;
      chrono = millis();
      return key;
    }
  }
}//readKeyWithRepeat----------------------------------------------------------------------------------------

//wasPressed=========================================================
//Asks if a specific key was pressed.
//If so, waits for the key to be released before sending the result.
//-------------------------------------------------------------------
bool Menu::wasPressed(int key) {
  if (readKey() == key) {
    while (readKey() == key) {}
    return true;
  }
  else return false;
}//wasPressed--------------------------------------------------------

 //isPressed=========================================================
 //Asks if a specific key was pressed.
 //If so, waits for the key to be released before sending the result.
 //-------------------------------------------------------------------
bool Menu::isPressed(int key) {
	if (readKey() == key) return true;
	else return false;
}//isPressed--------------------------------------------------------

//keyPressed=================================================
//Returns true if any of the arrow keys are pressed.
//-----------------------------------------------------------
bool Menu::keyPressed() {
  if (readKey() == 0) return false;
  else                return true;
}//keyPressed------------------------------------------

//update=================================================================================
//Use argument "key" to navigate the menu and set the currentNode.
//UP, DOWN and LEFT sets the current node to, respectively :
//the previous sibling, the next sibling and the parent of the current node.
//For the RIGHT key, there are two possibilities :
//- The action associated to the node needs to be carried out by the sketch. (>"000")
//- The action is "000" and the current node becomes it's eldest child
//This is the version where the library takes care of the keypad (either digital or analog)
//The LCD is automaticaly updated if it is our responsibility.
//If the sketch handles the LCD, we raise the "lcdNeedsUpdate" flag.
//---------------------------------------------------------------------------------------
int Menu::update(int key) {
int node = currentNode;
	if (key == UP) currentNode = previousSibling(currentNode);
	if (key == DOWN) currentNode = nextSibling(currentNode);
	if (key == LEFT) if (parent(currentNode) != 0) currentNode = parent(currentNode);
	if (key == RIGHT) {
		int action = getAction();
		if (action > 0) return action;
		else            if (eldest(currentNode) != 1) currentNode = eldest(currentNode);
	}
  if (currentNode != node) {
		lcdNeedsUpdate = true;
	  if (handelingLcd) showMenu();
  }
	return 0;
}//--------------------------------------------------------------------------------------

//update=====================================
//The Library handles the Keypad
//-------------------------------------------
int Menu::update() {
  int action = 0;
	if (wasPressed(UP)) action = update(UP);
	if (wasPressed(DOWN)) action = update(DOWN);
	if (wasPressed(LEFT)) action = update(LEFT);
	if (wasPressed(RIGHT)) action = update(RIGHT);
  return action;
}//update------------------------------------

//update=====================================
//The keypad returns characters
//-------------------------------------------
int Menu::updateWith(char key) {
  int action = 0;
	if (key == MYCHARUP) action = update(UP);
	if (key == MYCHARDOWN) action = update(DOWN);
	if (key == MYCHARLEFT) action = update(LEFT);
	if (key == MYCHARRIGHT) action = update(RIGHT);
  return action;
}//update-------------------------------------

 //update=====================================
 //The keypad returns integers
 //-------------------------------------------
int Menu::updateWith(int key) {
	int action = 0;
	if (key == MYINTUP) action = update(UP);
	if (key == MYINTDOWN) action = update(DOWN);
	if (key == MYINTLEFT) action = update(LEFT);
	if (key == MYINTRIGHT) action = update(RIGHT);
	return action;
}//update-------------------------------------

//done================================================================================================
//Allows the sketch to signal the library that the action is finished and that we return to the menu
//----------------------------------------------------------------------------------------------------
void Menu::done() {
  while(keyPressed()) {}
  lcdNeedsUpdate = true;
  if (handelingLcd) showMenu();
}//done-----------------------------------------------------------------------------------------------

//For debugging purposes...
void Menu::dump() {
	for (int i = 0; i <= lastNode; i++) {
		Serial.print(i); Serial.print(" : ");
		Serial.print(nodes[i].starts); Serial.print(" - ");
		Serial.print(nodes[i].ends); Serial.print(" - ");
		Serial.print(nodes[i].parent); Serial.print(" - ");
		Serial.print(nodes[i].eldest); Serial.print(" - ");
		Serial.println(nodes[i].action);
	}
	Serial.println(sizeof(node));

}