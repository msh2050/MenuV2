/*
* Menu.h
* Author : Jacques Bellavance
* Date : August 27, 2016
* Version : 1.00
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
 * Menu.h
 * Allows the use of menus in sketches.
 * The menu is declared in a String. 
 * (See the example sketch "MenuEN.ino" for instructions on how a menu is built) 
 * The menu is parsed, allowing easy navigation thru the menu
 * The library can handle the LCD and the switches (arrow keys) to navigate the menus.
 */
 
#ifndef Menu_h
#define Menu_h

//Those are the only Liquid Crystal Libraries the are presently supported right now.
#include <LiquidCrystal.h>
#include <LiquidTWI.h>

class Menu {
  public: //===================================================================================================
  //Constructor 
    //items : the String containing the menu
    Menu(String items);

  //Methods
    //The Library can handle the LCD or let the sketch do it
		void handleLcd(LiquidCrystal *lcd, int columns, int rows);                //The 4 data pins version
		void handleLcd(LiquidTWI *lcd, int columns, int rows);                    //An I2C Library object
		void defineLcd(int columns, int rows);                                    //The sketch manages it's LCD

		String lcdLine(int line);          																				//Returns the label to be displayed on the LCD's "line"
		bool needsUpdate();                                                       //Returns "true" if the LCD needs to be updated
		void updated();                                                           //Says that the current menu was udated on the LCD
		void updateLcd();                                                         //Says that the LCD will need to be updated
		void showMenu();                                                          //Displays the menu on the LCD

    //The Library can handle the keypad or let the sketch do it
		void handleSwitches(int keyUP, int keyDOWN, int keyLEFT, int keyRIGHT);   //The digital (1 pin per switch) version
    void handleSwitches(int analogPin);                                       //The analog (1 pin for the four switches version
		void mapKeys(char UP, char DOWN, char LEFT, char RIGHT);                  //The sketch handles it's own keypad (Characters)
		void mapKeys(int UP, int DOWN, int LEFT, int RIGHT);                      //The sketch handles it's own keypad (Integers)

	  int readKey();		                                                        //Returns the key that was pressed
    bool isPressed(int key);                                                  //Request a specific key
		bool wasPressed(int key);                                                 //Request a specific key and wait for release
		bool keyPressed();                                                        //Is true if any switch is still pressed  
		int readKeyWithRepeat(int delayForRepeat, int sensitivity);               //Returns repeatedly the key pressed

    //A key was pressed, update the menu
    int update();                                                             //Read the switches and update the menu accordingly
		int update(int key);	                                                    //Update the menu (Library controls the keypad) 
		int updateWith(char key);                                                 //Update the menu according to mapKey() (Characters)
		int updateWith(int key);                                                  //Update the menu according to mapKey() (Integers)

    //Provide some informations to the sketch 
	  String getCurrentLabel();                                                 //Returns the label of the current item
		int getCurrentItem();                                                     //Returns the number of the current menu item (currentNode)
    int getAction();            	                                            //Returns the action associated to the current item
		void setCurrentItem(String label);

 
    //Let the Sketch advise us that
		void done();                                                              //The action is handled, return to the menu
    void restart();                                                           //Sets currentNode to 1
    
    //Extras  
		void dump(); //debug only

  private: //====================================================================================================
    //Default values for the size of the LCD
    int LCDcol = 16;
    int LCDrows = 2;

    //The full menu is contained in "MYitems"
    //A node is associated to each item in the menu.
    //The nodes are placed in the table "nodes[]"
    //The menu is the parsed to setup nodes
    String MYitems;     //The full menu
    struct node {       //For each item :
      int starts = 0;     //the index of the start of the label
      int ends = 0;       //the index of the end of the label
      byte parent = 0;    //the node number of the parent of this item
      byte eldest = 1;    //the node number of the eldest of this item
      int action = 0;     //the action associated to this item
    };
	  node *nodes;        //The table that holds the nodes (using calloc() to use only the needed memory)
    void menuParse();   //Actual parsing of "MYitems" and setup of "nodes[]"

    //Labels of the menu or submenu to be displayed on the LCD
    String label(int node); //Returns the label of "node"

    //Moving around the menus
    int currentNode = 1;            //The index of the current node
    int lastNode = 1;               //The index of the last node
    int parent(int node);           //The parent of "node"
    int eldest(int node);           //The eldest child of "node"
    int previousSibling(int node);  //The previous sibling of "node"
    int nextSibling(int node);      //The next sibling of "node"
		int siblingsCount(int node);    //The number of siblings of "node"
    int rank(int node);             //The rank of "node" amongst it's siblings
    
  //LCD
    LiquidCrystal *MYLcd;                    //A pointer to the sketche's LCD with 4 data pins
		LiquidTWI *MYLcdTWI;
    bool handelingLcd = false;               //A reminder as to who is handling the LCD (the sketch or the library)
    int LcdId = 0;                           //On which LCD the menu is displayed (Default: the sketch handles the LCD)
    int LcdIsFourPins = 1;
    int LcdIsTWI = 2;
    bool lcdNeedsUpdate = true;              //Keeps track of the needs to update the LCD
    void clearLCD();
		int toLCD(String msg, int col, int row);

  //SWITCHES  
    int MYUP;           //The pins for the four arrow switches
    int MYDOWN;
    int MYLEFT;
    int MYRIGHT;
    int MYANALOG;       //The pin for the analog switches
		char MYCHARUP;      //The values provided by the sketch are characters
		char MYCHARDOWN;
		char MYCHARLEFT;
		char MYCHARRIGHT;
		int MYINTUP;        //The values provided by the sketch are integers
		int MYINTDOWN;
		int MYINTLEFT;
		int MYINTRIGHT;
		int UP = 1;         //The values we return to the sketch
	  int DOWN = 2;
	  int LEFT = 3;
	  int RIGHT = 4;
    bool handelingSwitches = false;  //A reminder as to who is handling the switches (the sketch or the library)
	  bool switchesAreAnalog = false;  //To let the sketch decide what kind of switches to use.
	  int readDigitalKey();            //Use four Arduino pins in INPUT_PULLUP mode
	  int readAnalogKey();             //Use one analog pin
		bool keysAreIntegers = false;    //The keys sent by the sketch are integers
};
#endif
