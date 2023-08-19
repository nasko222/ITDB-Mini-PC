#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

// Digital pins
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8
// LEDA goes to 3.3V
// VCC goes to 5V
// GND goes to GND

// Global defines
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
int appID = -1; // -1 main menu, 1 calculator, 2 calendar, 3 notepad

// Defines Calculator

// grid values
const int buttonWidth = 50;
const int buttonHeight = 40;
const int spacing = 5;

// Activates after running the equal function
bool shouldReset = false;

int numberMode = 1; //The number that gets set up, it gets changed to 2 after running a function key (plus, minus, etc...)
int highlightID = 12; // In reading order, starting from 0
//123+
//456-
//789*
//C0=/

//12 sets it to C (Clear)

String displayedText = ""; // Buffer variable for the displayed text
String currentNumber = ""; // The current number from number mode
int functionID = -1; // 0 - add, 1 - subtract, 2 - multiply, 3 - divide
long number1 = 0; // number 1 memory value
long number2 = 0; // number 2 memory value
double result = 0;

// End Defines Calculator

// Defines Calendar

const char* monthNames[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

const int monthDays[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  //  ^ , changes to 29 on a leap year, not controlled here
};

const char* dayNames[] = {
  "M", "T", "W", "T", "F", "S", "S"
};

int currentMonth = 8 - 1; // August (Month 8), don't touch the -1
int currentYear = 2023;
const int numMonths = sizeof(monthNames) / sizeof(monthNames[0]);

// End Defines Calendar

// Defines Notepad

const char* qwerty[] = {
  "QWERTYUIOP",
  "ASDFGHJKL",
  "ZXCVBNM"
};

// number of keyboard rows
const int numRows = sizeof(qwerty) / sizeof(qwerty[0]);

// grid values
const int keyWidth = 22;
const int keyHeight = 22;
const int keySpacing = 2;

int currentKey = 27; // 0-25 letters from the qwerty array, 26 is delete, 27 is spacebar, 28 is enter
String TheTextbox = ""; // Textbox value, without the "_" symbol which gets added additionally.

// End Defines Notepad

void setup() {
  // Button 1 - Pin2, Button 2 - Pin3
  // Connect the other side to GND
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  tft.begin();
  tft.fillScreen(ILI9340_BLACK);
}

void setup_calc(){
  tft.fillScreen(ILI9340_BLACK);
  drawCalculatorInterface();
  appID = 1;
}

void setup_calendar(){
  tft.fillScreen(ILI9340_BLACK);
  drawCalendar(currentYear, currentMonth);
  appID = 2;
}

void setup_notepad(){
  drawKeyboard(-2, -2, true);
  clearTextBox();
  appID = 3;
}

void loop(){
  if (appID == 1){
    loop_calc();
  }else if (appID == 2){
    loop_calendar();
  }else if (appID == 3){
    loop_notepad();
  }else{
    if (analogRead(A0) == 0){
      setup_calc();
    }else if (analogRead(A1) == 0){
      setup_calendar();
    }else if (analogRead(A2) == 0){
      setup_notepad();
    }
  }
}

void loop_calc() {
  if (digitalRead(2) == LOW) {
    button1pressAction();
    delay(200);
  }
  else if (digitalRead(3) == LOW) {
    button2pressAction();
    delay(200);
  }
}

void loop_calendar() {
  if (digitalRead(2) == LOW) {
    currentMonth = (currentMonth + 1) % numMonths;
    if (currentMonth == 0) {
      currentYear++;
    }
    drawCalendar(currentYear, currentMonth);
    delay(100);
  }
  
  if (digitalRead(3) == LOW) {
    if (currentMonth == 0) {
      currentMonth = numMonths - 1;
    } else {
      currentMonth--;
    }
    if (currentMonth == 11) {
      currentYear--;
    }
    drawCalendar(currentYear, currentMonth);
    delay(100);
  }
}

void loop_notepad() {
  if (digitalRead(2) == LOW) {
    KeyPress1();
    delay(200);
  }
  else if (digitalRead(3) == LOW) {
    KeyPress2();
    delay(200);
  }
}

void resetCalc(){
  shouldReset = false;
  result = 0;
  clearMyDisplay();
}

void clearMyDisplay(){
  // Clears the upper part of the display where numbers and functions appear
  tft.fillRect(0, 0, tft.width(), 100, ILI9340_BLACK);
  
  // Reset few stuff
  numberMode = 1;
  displayedText = "";
  currentNumber = "";
  functionID = -1;
  number1 = 0;
  number2 = 0;
}

void updateDisplayedText(int mode, const String &text) {
  displayedText = text; // Buffer
  
  int num = mode;
  tft.setCursor(20, 10 + (num - 1) * 30); // Depends on the number mode, it puts it in a particular row
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.print(displayedText);
}

void addFunction() {
  String text = "."; // Shouldn't appear as a . unless something goes wrong
  
  if (highlightID == 3){
    functionID = 0;
    text = "+";
  }
  else if (highlightID == 7){
    functionID = 1;
    text = "-";
  }
  else if (highlightID == 11){
    functionID = 2;
    text = "*";
  }
  else if (highlightID == 15){
    functionID = 3;
    text = "/";
  }
  
  displayedText = text; // Buffer
  
  tft.setCursor(200, 25);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(4);
  tft.print(displayedText);
}

void equalFunc() {
  number2 = currentNumber.toInt();
  if (functionID == 0) {
    result = number1 + number2;
  } else if (functionID == 1) {
    result = number1 - number2;
  } else if (functionID == 2) {
    result = number1 * number2;
  } else if (functionID == 3) {
    result = (float) number1 / number2; // Convert to float for division
  }

  clearMyDisplay();

  char resultString[20]; // 20 should be enough for the highest numbers
  dtostrf(result, 0, 6, resultString); // The third argument (6) is the decimal places

  // Remove unnecessary trailing zeros and decimal point
  int length = strlen(resultString);
  int decimalIndex = -1;

  for (int i = 0; i < length; i++) {
    if (resultString[i] == '.') {
      decimalIndex = i;
      break;
    }
  }

  if (decimalIndex != -1) {
    while (length > 0 && resultString[length - 1] == '0') {
      resultString[length - 1] = '\0';
      length--;
    }

    if (resultString[length - 1] == '.') {
      resultString[length - 1] = '\0';
      length--;
    }
  }
  // End of trim code

  tft.setCursor(20, 10);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.print("= "); // Equals to:
  tft.print(resultString);
  
  // After any input, it will clear the box
  shouldReset = true;
}

void drawCalculatorInterface() {
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_WHITE);
  updateCalculatorButton(-2, -2, true);
}

void updateCalculatorButton(int current, int previous, bool all){
  // If it doesn't update all, It updates only the current and the previous selected buttons
  if (current == 0 || previous == 0 || all) drawButton("1", 10+spacing, 120, highlightID == 0);
  if (current == 1 || previous == 1 || all) drawButton("2", 10+spacing + buttonWidth + spacing, 120, highlightID == 1);
  if (current == 2 || previous == 2 || all) drawButton("3", 10+spacing + 2 * (buttonWidth + spacing), 120, highlightID == 2);
  if (current == 3 || previous == 3 || all) drawButton("+", 10+spacing + 3 * (buttonWidth + spacing), 120, highlightID == 3);

  if (current == 4 || previous == 4 || all) drawButton("4", 10+spacing, 120 + buttonHeight + spacing, highlightID == 4);
  if (current == 5 || previous == 5 || all) drawButton("5", 10+spacing + buttonWidth + spacing, 120 + buttonHeight + spacing, highlightID == 5);
  if (current == 6 || previous == 6 || all) drawButton("6", 10+spacing + 2 * (buttonWidth + spacing), 120 + buttonHeight + spacing, highlightID == 6);
  if (current == 7 || previous == 7 || all) drawButton("-", 10+spacing + 3 * (buttonWidth + spacing), 120 + buttonHeight + spacing, highlightID == 7);

  if (current == 8 || previous == 8 || all) drawButton("7", 10+spacing, 120 + 2 * (buttonHeight + spacing), highlightID == 8);
  if (current == 9 || previous == 9 || all) drawButton("8", 10+spacing + buttonWidth + spacing, 120 + 2 * (buttonHeight + spacing), highlightID == 9);
  if (current == 10 || previous == 10 || all) drawButton("9", 10+spacing + 2 * (buttonWidth + spacing), 120 + 2 * (buttonHeight + spacing), highlightID == 10);
  if (current == 11 || previous == 11 || all) drawButton("*", 10+spacing + 3 * (buttonWidth + spacing), 120 + 2 * (buttonHeight + spacing), highlightID == 11);

  if (current == 12 || previous == 12 || all) drawButton("C", 10+spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 12);
  if (current == 13 || previous == 13 || all) drawButton("0", 10+spacing + buttonWidth + spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 13);
  if (current == 14 || previous == 14 || all) drawButton("=", 10+spacing + 2 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 14);
  if (current == 15 || previous == -1 || all) drawButton("/", 10+spacing + 3 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 15);
  //                        ^, special case -1, because it goes off bounds otherwise
}

void drawButton(const char *label, int x, int y, bool highlight) {
  if (highlight){
    tft.fillRect(x, y, buttonWidth, buttonHeight, 0b1010110011011000); // Very light blue
    tft.setTextColor(ILI9340_BLACK);
  }
  else{
    tft.fillRect(x, y, buttonWidth, buttonHeight, ILI9340_BLUE);
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setCursor(x + buttonWidth / 4, y + buttonHeight / 4 + 5);
  tft.setTextSize(2);
  tft.print(label);
}

void button1pressAction(){
  if (shouldReset){
    resetCalc();
  }
  
  highlightID++;
  if (highlightID > 15){
    highlightID = 0;
  }
  updateCalculatorButton(highlightID, highlightID - 1, false);
}

void button2pressAction(){
  if (shouldReset){
    resetCalc();
  }
  
  if (highlightID == 12){
  // Clear
    currentNumber = "";
    clearMyDisplay();
  }
  if (highlightID == 7 && currentNumber.length() == 0){
  // Negative numbers?
  // put "-" in front of the number instead of running it as a function key.
    currentNumber = "-";
    updateDisplayedText(numberMode, currentNumber);
  }
  // Function keys, only number 1 mode, checks whether number is valid
  if (currentNumber.length() > 0 && currentNumber != "-" && numberMode == 1){
    if (highlightID == 3 || highlightID == 7 || highlightID == 11 || highlightID == 15){
      numberMode = 2; // Waiting for the next number now
      addFunction();
      number1 = currentNumber.toInt(); // Save to memory
      currentNumber = ""; // Clear current number, prepare for number 2
    }
  }
  if (currentNumber.length() > 0 && currentNumber != "-" && numberMode == 2 && highlightID == 14){
  // Equals, only after we get two valid numbers
    equalFunc();
  }
  
  // Number keys
  if (currentNumber.length() < 8) { // <-- don't allow to go offscreen
    if (highlightID == 13 && currentNumber.length() == 0){
    // No zeros before other numbers.
    }
    else{
      int numberID = highlightID;
      if (highlightID == 0 || highlightID == 1 || highlightID == 2){
        numberID++; //1, 2 and 3
      }
      else if (highlightID == 4 || highlightID == 5 || highlightID == 6){
          //4, 5 and 6
      }
      else if (highlightID == 8 || highlightID == 9 || highlightID == 10){
        numberID--; //7, 8 and 9
      }
    else if (highlightID == 13){
        numberID = 0;//0
      }
    else{
    // Not a number key, break.
        return;
      }
      currentNumber += String(numberID); // Convert to string, add to current number
      updateDisplayedText(numberMode, currentNumber);
    }
  }
}

// End Calculator Code

// Start Calendar Code

void drawCalendar(int year, int monthIndex) {
  
  tft.fillScreen(ILI9340_BLACK);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);

  int xOffset = 20;
  int yOffset = 100; // Adjust this value to move the calendar up-down
  int cellWidth = 30;
  int cellHeight = 30;
  
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_RED);
  
  // Load the days
  for (int day = 0; day < 7; ++day) {
    int x = 8 + xOffset + day * cellWidth;
    int y = yOffset;
    
    tft.setCursor(x, y);
    tft.print(dayNames[day]);
  }
  
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_YELLOW);
  tft.setCursor(60, yOffset - 80);
  
  //Show month and year on top
  tft.print(monthNames[monthIndex]);
  tft.print(" ");
  tft.print(year);
  
  yOffset += cellHeight;
  
  int daysInMonth = monthDays[monthIndex];
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) { // Proper leap year check
    daysInMonth++; // February has 29 days in a leap year
  }
  
  int startingDay = dayOfWeek(year, monthIndex + 1, 1);
  
    for (int day = 1; day <= daysInMonth; ++day) {
    int x = xOffset + ((day - 1 + startingDay) % 7) * cellWidth;
    int y = yOffset + ((day - 1 + startingDay) / 7) * cellHeight;
  
    // Draw a square around the day's number
    tft.drawRect(x, y, cellWidth, cellHeight, ILI9340_WHITE);
  
    // Calculate the center of the square for the text positioning
    int textX = x + cellWidth / 2 - (day >= 10 ? 10 : 5);
    int textY = y + cellHeight / 2 - 8;
  
    tft.setCursor(textX, textY);
    tft.setTextColor(ILI9340_WHITE);
    tft.print(day);
  }

}

int dayOfWeek(int year, int month, int day) {
  
// Calculates where each day of each year belongs in the weeks respectively

  if (month < 3) {
    month += 12;
    year--;
  }
  return (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
}

// End Calendar Code

// Start Notepad Code

void clearTextBox(){
  // The notepad screen
  tft.drawRect(5, 5, 230, 200, ILI9340_YELLOW);
  tft.drawRect(6, 6, 229, 199, ILI9340_YELLOW);
  tft.drawRect(6, 6, 228, 198, ILI9340_YELLOW);
  tft.drawRect(6, 6, 227, 197, ILI9340_YELLOW);
  tft.drawRect(6, 6, 226, 196, ILI9340_YELLOW);
  tft.fillRect(7, 7, 225, 195, ILI9340_BLACK);

  // Additional keyboard squares
  int displayHeight = tft.height();  // Height of the display
  int keyboardHeight = numRows * (keyHeight + keySpacing) + 25;  // Height of the keyboard
  int margin = (displayHeight - keyboardHeight);
  
  tft.drawRect(0, margin, tft.width(), keyboardHeight, ILI9340_WHITE);
  tft.drawRect(1, margin + 1, tft.width() - 2, keyboardHeight - 2, ILI9340_WHITE);
  
  TheTextbox = "";
  drawText(TheTextbox + "_"); // Don't forget the "_" cursor
}

void drawText(String text){
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);
  
  // Proper text wrap, only inside the yellow rectangle
  int x = 10;
  int y = 10;
  for (int i = 0; i < text.length() + 1; ++i) {
    tft.fillRect(x, y, 12, 20, ILI9340_BLACK);
    tft.setCursor(x, y);
    tft.print(text.charAt(i));
    
    x += 12; // Adjust the x position based on character width
    if ((i + 1) % 18 == 0) {
      y += 20; // Move to the next row
      x = 10;  // Reset x position for the new row
    }
  }
}

void KeyPress1(){
  currentKey++;
  if (currentKey > 28){
    currentKey = 0;
  }
  drawKeyboard(currentKey, currentKey - 1, false);
}

void KeyPress2() {
  if (currentKey < 26){
    String keyboard = "QWERTYUIOPASDFGHJKLZXCVBNM";
    TheTextbox += keyboard[currentKey];
    drawText(TheTextbox + "_"); // Don't forget the "_" cursor
  }else if (currentKey == 26){
  // Delete, removes last char on the textbox
    if (TheTextbox.length() > 0) {
        TheTextbox = TheTextbox.substring(0, TheTextbox.length() - 1);
        drawText(TheTextbox + "_"); // Don't forget the "_" cursor
    }
  }else if (currentKey == 27){
  // Spacebar
    TheTextbox += " ";
    drawText(TheTextbox + "_"); // Don't forget the "_" cursor
  }else if (currentKey == 28){
  // Enter, adds enough space characters until it fills the row
    int seperate = 18 - (TheTextbox.length() % 18); // Calculate how many characters can fit in the row?
    for (int i = 0; i < seperate; i++) TheTextbox += " "; // Fill the places
    drawText(TheTextbox + "_"); // Don't forget the "_" cursor
  }
  
}

void drawKeyboard(int a, int b, bool all) {
  // If it doesn't update all, It updates only the current and the previous selected buttons
  int yOffset = tft.height() - ((numRows + 1) * (keyHeight + keySpacing));
  
  // Draw QWERTY keyboard
  int nextKey = -1;
  for (int row = 0; row < numRows; ++row) {
    int rowLength = strlen(qwerty[row]);
    int rowWidth = rowLength * (keyWidth + keySpacing) - keySpacing;
    int rowXOffset = (tft.width() - rowWidth) / 2;
    
    for (int col = 0; col < rowLength; ++col) {
      int x = rowXOffset + col * (keyWidth + keySpacing);
      int y = yOffset + row * (keyHeight + keySpacing);
      nextKey++;
      if (nextKey == a || nextKey == b || all)
      drawKey(x, y, keyWidth, keyHeight, String(qwerty[row][col]), 2, nextKey);
    }
  }

  // Draw spacebar
  int spacebarWidth = keyWidth * 6 + keySpacing * 5;
  int spacebarX = (tft.width() - spacebarWidth) / 2;
  int spacebarY = tft.height() - keyHeight - keySpacing;
  if (a == 27 || b == 27 || all)
  drawKey(spacebarX, spacebarY, spacebarWidth, keyHeight, " ", 2, 27);

  // Draw enter key
  int enterWidth = keyWidth * 2 + keySpacing;
  int enterX = tft.width() - enterWidth - keySpacing;
  int enterY = spacebarY;
  if (a == 28 || b == -1 || all) // Special case, -1, just like the calculator
  drawKey(enterX, enterY, enterWidth, keyHeight, "ENTER", 1, 28);

  // Draw backspace key
  int backspaceWidth = keyWidth * 2 + keySpacing;
  int backspaceX = keySpacing;
  int backspaceY = spacebarY;
  if (a == 26 || b == 26 || all)
  drawKey(backspaceX, backspaceY, backspaceWidth, keyHeight, "DELETE", 1, 26);
}

void drawKey(int x, int y, int width, int height, String label, int keysize, int keyID) {
  if (keyID == currentKey){
    tft.fillRect(x, y, width, height, ILI9340_WHITE);
  }else{
  // Highlighted key appear in white with black font, the rest appear black with white font
    tft.fillRect(x, y, width, height, ILI9340_BLACK);
    tft.drawRect(x, y, width, height, ILI9340_WHITE);
  }
  if (keysize == 1){ // Alternative calculation for the bigger keys
    tft.setCursor(x + width / 8, y + 5 + height / 6);
  }else{
    tft.setCursor(x + width / 4, y + height / 4);
  }
  if (keyID == currentKey){
    tft.setTextColor(ILI9340_BLACK);
  }else{
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setTextSize(keysize);
  tft.print(label);
}

// End Notepad Code
