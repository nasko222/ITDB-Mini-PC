#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

// Digital pins define

// ITDB Display
#define _sclk 13
//#define _miso 12 <-- not used if no SD card
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8
//#define _sd 4 <-- not used if no SD card
// LEDA goes to 5V
// VCC goes to 5V
// GND goes to GND

// Buttons (LEGACY)
//#define LEFT_BTN 5 // Left Button, Connect to D5 and GND
//#define RIGHT_BTN 3 // Right Button, Connect to D3 and GND
//#define ENTER_BTN 4 // Enter Button, Connect to D4 and GND
#define HOME_BTN 2 // Home Button, Connect to D2 and GND

// Joystick defines
#define JoyAxis0 A0 // X Axis
#define JoyAxis1 A1 // Y Axis
#define JoyBut0 4 // Button
const int minResistance0 = 100;
const int minResistance1 = 100;
const int maxResistance0 = 950;
const int maxResistance1 = 950;

//uint8_t horizontalDir = 1; //0 - left, 1 - idle, 2 - right
//uint8_t verticalDir = 1; //0 - up, 1 - idle, 2 - down

bool goButton(){ return digitalRead(JoyBut0) == LOW;}
bool goLeft(){ return analogRead(JoyAxis0) > maxResistance0;}
bool goRight(){ return analogRead(JoyAxis0) < minResistance0;}
bool goUp(){ return analogRead(JoyAxis1) < minResistance1;}
bool goDown(){ return analogRead(JoyAxis1) > maxResistance1;}
/*
bool goTopLeft(){ return goLeft() || goUp();}
bool goTopRight(){ return goRight() || goUp();}
bool goBottomLeft(){ return goLeft() || goDown();}
bool goBottomRight(){ return goRight() || goDown();}
*/

// Global defines
Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);
uint8_t appID; // 0 main menu, 1 calculator, 2 calendar, 3 notepad, 4 stopwatch, 5 tictactoe, 6 paint

// Main Menu Defines
const int appsAmount = 6;
const int screenWidth = 240;
uint8_t currentChosenApp = 101; // Doesn't reset
// Icons, unused
//const int iconSize = 60;
//const int iconSpacingX = 50;
//const int iconSpacingY = 20;
//const int iconsPerRow = 2;

String mainMenuTime;

// Defines Calculator

// grid values
const int buttonWidth = 50;
const int buttonHeight = 40;
const int spacing = 5;

// Activates after running the equal function
bool shouldReset;

uint8_t numberMode; //The number that gets set up, it gets changed to 2 after running a function key (plus, minus, etc...)
int8_t highlightID; // In reading order, starting from 0
//123+
//456-
//789*
//C0=/

String displayedText; // Buffer variable for the displayed text
String currentNumber; // The current number from number mode
uint8_t functionID; // 0 - add, 1 - subtract, 2 - multiply, 3 - divide
long number1; // number 1 memory value
long number2; // number 2 memory value
double result;

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

const int numMonths = sizeof(monthNames) / sizeof(monthNames[0]);

uint8_t currentDay;
uint8_t calendarMonth;
uint8_t currentMonth; //RTC controlled
int calendarYear;
int currentYear; //RTC controlled

bool yearMode; //If set to true, the arrow keys will control the year, instead of the months

// End Defines Calendar

// Defines Notepad

const char* qwerty[] = {
  "QWERTYUIOP",
  "ASDFGHJKL",
  "ZXCVBNM"
};

const char* qwerty2[] = {
  "qwertyuiop",
  "asdfghjkl",
  "zxcvbnm"
};

const char* symbol[] = {
  "1234567890",
  "+-*/=.,?!",
  "()#$%:\""
};

// number of keyboard rows
const int numRows = sizeof(qwerty) / sizeof(qwerty[0]);

// grid values
const int keyWidth = 22;
const int keyHeight = 22;
const int keySpacing = 2;

// Notepad technical consts
const int notepadSize = 144;
const int maxYGridRender = 170;

// Notepad rectangle margin
const int notepadGridX1 = 5;
const int notepadGridY1 = 7;
const int notepadGridX2 = 230;
const int notepadGridY2 = 170;

// Valid colors, available for change for our font color
const uint16_t validColors[] = {ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, 0x471A, ILI9341_WHITE}; // 0x471A is better blue

int8_t currentKey; // 0-25 letters from the qwerty array, 26 is delete, 27 is spacebar, 28 is enter
int8_t keyboardMode; //0 - caps, 1 - small, 2 - numbers and symbols

String TheTextbox = ""; // Textbox value, without the "_" symbol which gets added additionally.
uint16_t currentColor = ILI9341_WHITE; // Text font color

// End Defines Notepad

// Defines Stopwatch

bool running = false;
bool paused = false;
uint8_t stopwatchButton = 0;
unsigned long stopwatchTimeStamp = 0;
unsigned long freezeTimeStamp = 0;
unsigned long clockElapsedSeconds = 0;

unsigned long pausedSecs = 0;
int updatePossibleSecs;

// End Defines Stopwatch

// Defines Tic Tac Toe

const int CELL_SIZE = 75;
const int GRID_OFFSET_X = 8;
const int GRID_OFFSET_Y = 12;

uint8_t currentPlayer;
int8_t currentCursor;

uint8_t occupiedLoc[9]; //0 - Empty, 1 - X, 2 - O

bool gameTie;
bool showWinner;
char winnerChar;

// End Defines Tic Tac Toe

// Start Defines Paint

bool showGrid = true;

const int negativeYOffset = 5;
const int colorSlideDelay = 10;

uint8_t imageRes = 16;

uint16_t imageData[1024];

// Slider values
int8_t slider = 0; // 0 - red, 1 - green, 2 - blue
uint8_t rSlider = 127;
uint8_t gSlider = 127;
uint8_t bSlider = 127;

// Grid Cursor
int8_t cursorX;// = 15;
int8_t cursorY;// = 31;

uint8_t sliderMode = 0;
bool paintBTNRide = false;

// Current Brush
uint16_t getPaintBrush() {return tft.color565(rSlider, gSlider, bSlider);}

// End Defines Paint

// RTC Clock

ThreeWire myWire(6,7,5); // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);
bool RTCFailure;

// End RTC

void setup(){
  // Buttons (LEGACY)
  //pinMode(LEFT_BTN, INPUT_PULLUP);
  //pinMode(RIGHT_BTN, INPUT_PULLUP);
  //pinMode(ENTER_BTN, INPUT_PULLUP);
  pinMode(HOME_BTN, INPUT_PULLUP);
  
  // JoyAxis0 and 1 are defined by default as analog in
  //pinMode(JoyAxis0, INPUT);
  //pinMode(JoyAxis1, INPUT);
  pinMode(JoyBut0, INPUT_PULLUP);

  Rtc.Begin();
  tft.begin();
  appID = 0; // Get main menu
  defines();
  //generateRandomColors();
}

/*

Debug stuff random colors for testing cursor


void generateRandomColors() {
  for (int i = 0; i < imageWidth * imageHeight; i++) {
    for (int j = 0; j < 3; j++) {
      imageData[i][j] = random(256);
    }
  }
}

uint16_t getRandomColor() {
  return tft.color565(random(0, 256), random(0, 256), random(0, 256));
}*/

void RTCKeepAliveCheck(){
  RtcDateTime now = Rtc.GetDateTime();
  if (!now.IsValid()) RTCFailure = true;
}

void defines(){
  //Reset screen
  tft.fillScreen(ILI9341_BLACK);
  
  //Reset RTC Clock
  mainMenuTime = "";
  
  //Calculator
  shouldReset = false;
  numberMode = 1; // Default state, waiting for first number
  highlightID = 12; //12 sets it to C (Clear)
  displayedText = "";
  currentNumber = "";
  functionID = -1; // No functions (0 is add)
  number1 = 0;
  number2 = 0;
  result = 0;

  //Calendar
  //currentDay = 1; - All assigned by RTC
  //calendarMonth = 0; // 0 - January, 1 - February, 2 - March, etc
  //calendarYear = 2023; //Failsafe for RTC crash
  
  yearMode = false;

  //Notepad
  currentKey = 27; // Spacebar
  keyboardMode = 0; //CAPS

  //TheTextbox = "";
  //currentColor = ILI9341_WHITE;
  
  //Stopwatch
  //running = false;
  //paused = false;
  //stopwatchButton = 0;
  
  //stopwatchTimeStamp = 0;
  //freezeTimeStamp = 0;
  //clockElapsedSeconds = 0;
  //pausedSecs = 0;
  updatePossibleSecs = 0;
  
  //Tic Tac Toe
  currentPlayer = 0;
  currentCursor = 4;
  
  for (int i = 0; i < 9; i++) occupiedLoc[i] = 0;
  
  gameTie = false;
  showWinner = false;
  winnerChar = ' ';
  
  //Paint
  //(none)
  
  //RTC Keep Alive
  RTCKeepAliveCheck();
  
  //Start appropriate app
  if (appID == 1){
    setup_calc();
  }else if (appID == 2){
    setup_calendar();
  }else if (appID == 3){
    setup_notepad();
  }else if (appID == 4){
    setup_stopwatch();
  }else if (appID == 5){
    setup_tictactoe();
  }else if (appID == 6){
    setup_paint();
  }else{
    start_main(); // No other apps? Start main menu!
  }
}

void start_main(){
  // Draw PC label
  tft.setTextSize(3);
  tft.setCursor((screenWidth - 160) / 2, 20);
  tft.setTextColor(ILI9341_RED); tft.print("A");
  tft.setTextColor(0xFD20); tft.print("r"); //Orange
  tft.setTextColor(ILI9341_YELLOW); tft.print("d");
  tft.setTextColor(0x0420); tft.print("u"); // Lime
  tft.setTextColor(ILI9341_GREEN); tft.print("i");
  tft.setTextColor(0x95D4); tft.print("n"); //Light Blue
  tft.setTextColor(ILI9341_BLUE); tft.print("o");
  tft.setTextColor(0x780F); tft.print("P"); //Purple
  tft.setTextColor(0xFC9F); tft.print("C"); //Pink
  
  updateMainClock();
  
  drawApps(99, 99, true);
}

void updateMainClock(){
  //Draw a rectangle to clear overlap issues
  tft.fillRect((screenWidth - 100) / 2 - 5, 45, 120, 25, ILI9341_BLACK);
  //Draw clock below ArduinoPC Logo
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor((screenWidth - 100) / 2, 50);
  
  mainMenuTime = getTime(); tft.print(mainMenuTime);
}

String getTime() {
  RTCKeepAliveCheck();
  
  if (RTCFailure){
	  return "RTC Error";
  }
  RtcDateTime now = Rtc.GetDateTime();

  int hours = now.Hour();
  int minutes = now.Minute();
  //int seconds = now.Second();

  // Convert to 12-hour format
  bool isPM = false;
  if (hours >= 12) {
    isPM = true;
    if (hours > 12) {
      hours -= 12;
    }
  }

  // Ensure 12:01 AM instead of 00:01 AM
  if (hours == 0) {
    hours = 12;
  }
  
  //Fix for situations like 11:60 PM
  if (minutes >= 60){
	  hours++;
	  minutes = 0;
	  if (hours == 0) isPM = false; //Ensure 12:00 AM doesn't appear as PM and vice versa
  }

  // Format the time string
  String timeString = String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);

  // Add AM/PM
  if (isPM) {
    timeString += " PM";
  } else {
    timeString += " AM";
  }

  return timeString;
}


void drawApps(uint8_t current, uint8_t previous, bool all){
  if (current == 101 || previous == 101 || all) drawAppBox("Calculator", 0, ILI9341_WHITE, 0x95D4, 2);
  if (current == 102 || previous == 102 || all) drawAppBox("Calendar", 1, ILI9341_WHITE, 0xFD20, 2);
  if (current == 103 || previous == 103 || all) drawAppBox("Notepad", 2, ILI9341_WHITE, 0x780F, 2);
  if (current == 104 || previous == 104 || all) drawAppBox("Stopwatch", 3, ILI9341_WHITE, 0xF201, 2);
  if (current == 105 || previous == 105 || all) drawAppBox("TicTacToe", 4, ILI9341_WHITE, 0x537F, 2);
  if (current == 106 || previous == 106 || all) drawAppBox("Paint", 5, ILI9341_WHITE, 0x0420, 2);
}

// Main menu app drawing
void drawAppBox(const char* appName, uint8_t position, uint16_t appBoxColor, uint16_t textColor, uint8_t textSize) {
  int internalAppID = position + 101;
  //position += 1; // Do not interfere with the header <-- never mind its fine now
  
  int yOffset = 75;
  
  // Calculate new y coordinates for two boxes per row
  int y = (position / 2) * 80 + yOffset; // Adjusted to start from the top-left
  int y2 = 80; // Full height of the app box
  
  // Calculate x coordinates for two boxes per row
  int x = (position % 2) * screenWidth / 2;
  int x2 = screenWidth / 2;
  
  // Revert previous state
  tft.fillRect(x, y, x2, y2, ILI9341_BLACK);
  
  if (internalAppID == currentChosenApp) tft.fillRect(x, y, x2, y2, textColor);
  else tft.drawRect(x, y, x2, y2, appBoxColor);
  
  // App text label
  int textLength = strlen(appName);
  //int textSize = 2;
  int textWidth = textSize * 6 * textLength;
  int textX = x + (x2 - textWidth) / 2;
  int textY = y + y2 / 2 - 5;
  
  if (internalAppID == currentChosenApp) tft.setTextColor(ILI9341_BLACK);
  else tft.setTextColor(textColor);
  tft.setTextSize(textSize);
  tft.setCursor(textX, textY);
  
  tft.print(appName);
}


void setup_calc(){
  tft.fillScreen(ILI9341_BLACK);
  drawCalculatorInterface();
  
  //appID = 1;
}

void setup_calendar() {
  tft.fillScreen(ILI9341_BLACK);

  getRTCCalendarData();

  drawCalendar(currentYear, currentMonth);

  //appID = 2;
}

void setup_notepad(){
  clearTextBox();
  drawKeyboard(-2, -2, true);
  
  //appID = 3;
}

void setup_stopwatch(){
  if (running){
    if (!paused) updateStopwatchDisplay((millis() / 1000) - stopwatchTimeStamp); // Timer in memory, load back and continue
    else{
      updateStopwatchDisplay(pausedSecs);
    }
  }
  else updateStopwatchDisplay(0);
  
  displayStatusMessage();
  
  //appID = 4;
}

void setup_tictactoe(){
  drawBoard();
}

void setup_paint(){
  // Start the paint app
  startPaintApp();
}

void loop(){
  loop_general(); // Background loop, Always loops, in any app
  
  if (appID == 1){
    loop_calc();
  }else if (appID == 2){
    loop_calendar();
  }else if (appID == 3){
    loop_notepad();
  }else if (appID == 4){
    loop_stopwatch();
  }else if (appID == 5){
    loop_tictactoe();
  }else if (appID == 6){
    loop_paint();
  }else{
    loop_main(); // No other apps? Go to Main menu loop
  }
}

void loop_general(){  
  if (digitalRead(HOME_BTN) == LOW && appID > 0) {
    appID = 0;
    defines();
  }
}

void loop_main() {
  if (getTime() != mainMenuTime) updateMainClock();
  
  if (goRight()) {
    int prevApp = currentChosenApp;
    currentChosenApp++;
    if (currentChosenApp > appsAmount + 100) currentChosenApp = 101;
    drawApps(currentChosenApp, prevApp, false);
    delay(200);
  }
  else if (goLeft()) {
    int prevApp = currentChosenApp;
    currentChosenApp--;
    if (currentChosenApp < 101) currentChosenApp = appsAmount + 100;
    drawApps(currentChosenApp, prevApp, false);
    delay(200);
  }
  else if (goDown()) {
    int prevApp = currentChosenApp;
    currentChosenApp+=2;
    if (currentChosenApp > appsAmount + 101) currentChosenApp = 102;
    if (currentChosenApp > appsAmount + 100) currentChosenApp = 101;
    drawApps(currentChosenApp, prevApp, false);
    delay(200);
  }
  else if (goUp()) {
    int prevApp = currentChosenApp;
    currentChosenApp-=2;
    if (currentChosenApp < 100) currentChosenApp = appsAmount + 99;
    if (currentChosenApp < 101) currentChosenApp = appsAmount + 100;
    drawApps(currentChosenApp, prevApp, false);
    delay(200);
  }
  else if (goButton()) {
    if (currentChosenApp < appsAmount + 101){
      appID = currentChosenApp - 100;
      defines();
    }
  }
}

void loop_calc() {
  if (goLeft()) {
    button0pressAction();
    delay(200);
  }
  else if (goRight()) {
    button1pressAction();
    delay(200);
  }
  else if (goUp()) {
    button0pressActionB();
    delay(200);
  }
  else if (goDown()) {
    button1pressActionB();
    delay(200);
  }
  else if (goButton()) {
    button2pressAction();
    delay(200);
  }
}

void loop_calendar() {
  // Left arrow, go backwards
  if (goUp()) {
    if (yearMode){
      if (calendarYear > 1970){
        calendarYear--;
      }else{
        calendarYear = 2037;
      }
    }
    else{
      if (calendarMonth == 0) {
        calendarMonth = numMonths - 1;
      } else {
        calendarMonth--;
      }
      if (calendarMonth == 11) {
        calendarYear--;
      }
    }
    getRTCCalendarData();
    drawCalendar(calendarYear, calendarMonth);
    delay(50);
  }

  // Right arrow, go forwards
  if (goDown()) {
    if (yearMode){
      if (calendarYear < 2037){
        calendarYear++;
      }else{
        calendarYear = 1970;
      }
    }
    else{
      calendarMonth = (calendarMonth + 1) % numMonths;
      if (calendarMonth == 0) {
        calendarYear++;
      }
    }
	getRTCCalendarData();
    drawCalendar(calendarYear, calendarMonth);
    delay(50);
  }
  
  //Switch controller to year/month
  if (goLeft() && yearMode) {
    yearMode = false;
    printCalendarLabel(100); //yOffset
    delay(200);
  }
  
  if (goRight() && !yearMode) {
    yearMode = true;
    printCalendarLabel(100); //yOffset
    delay(200);
  }
  
  //Enter button - Go back to RTC-date
  if (goButton()){
	getRTCCalendarData();
	drawCalendar(currentYear, currentMonth);
	delay(50);
  }
}

void loop_notepad() {
  if (goLeft()) {
    KeyPress0();
    delay(300);
  }
  else if (goRight()) {
    KeyPress1();
    delay(300);
  }
  else if (goUp()) {
    KeyPress0b();
    delay(300);
  }
  else if (goDown()) {
    KeyPress1b();
    delay(300);
  }
  else if (goButton()) {
    KeyPress2();
    delay(200);
  }
}

void loop_stopwatch() {
  unsigned long currentTimeMillis = millis();
  unsigned long elapsedSeconds = (currentTimeMillis / 1000) - stopwatchTimeStamp;

  if ((goLeft() || goRight() || goUp() || goDown()) && running) {
    stopwatchButton = (stopwatchButton + 1) % 2; // Both buttons switch to the opposite function (Pause/Resume, Start/Stop)
    displayStatusMessage();
    delay(300);
  } else if (goButton()) {
    if (!running || stopwatchButton == 0) {
      // Start or Stop
      running = !running;
      paused = false;
      if (running) {
        clockElapsedSeconds = 0;
        pausedSecs = 0;
        updateStopwatchDisplay(0); //loadStopwatch();
        stopwatchTimeStamp = currentTimeMillis / 1000;
        if (paused) {
          unsigned long pauseDuration = (currentTimeMillis - freezeTimeStamp) / 1000;
          stopwatchTimeStamp += pauseDuration; // Compensate for paused time
        }
      } else {
        //updateStopwatchDisplay(elapsedSeconds); <-- don't do it, it breaks pause, just stop it
      }
      displayStatusMessage();
      delay(200);
    } else {
      // Pause or Resume
      if (paused) {
        // Resume
        unsigned long pauseDuration = (currentTimeMillis - freezeTimeStamp) / 1000;
        stopwatchTimeStamp += pauseDuration; // Compensate for paused time
        pausedSecs = 0;
      } else {
        // Pause
        clockElapsedSeconds = elapsedSeconds;
        pausedSecs = clockElapsedSeconds;
        freezeTimeStamp = currentTimeMillis;
      }
      paused = !paused;
      displayStatusMessage();
      delay(200);
    }
  }

  if (running && !paused) {
    updatePossibleSecs++; if (updatePossibleSecs > 1000) updatePossibleSecs = 1000;
    if (clockElapsedSeconds != elapsedSeconds && elapsedSeconds != 0 && updatePossibleSecs == 1000) updateStopwatchDisplay(elapsedSeconds);
  }else{
    updatePossibleSecs--; if (updatePossibleSecs < 0) updatePossibleSecs = 0;
  }
}

void loop_tictactoe(){
  if (goLeft()) {
    TicTacLeft();
    delay(300);
  }
  if (goRight()) {
    TicTacRight();
    delay(300);
  }
  if (goUp()) {
    TicTacUp();
    delay(300);
  }
  if (goDown()) {
    TicTacDown();
    delay(300);
  }
  else if (goButton()) {
    TicTacEnter();
    delay(200);
  }
}

void loop_paint(){
  if (sliderMode == 1){
      if (goLeft()){
        moveCursor(-1, 0);
        delay(150);
      }
      if (goRight()){
        moveCursor(1, 0);
        delay(150);
      }
      if (goUp()) {
        moveCursor(0, -1);
        delay(150);
      }
      if (goDown()) {
        moveCursor(0, 1);
        delay(150);
      }

      if (goButton()) addPaintColor();

      return;
  }
  if (goLeft()){
    backwardSlider();
  }
  else if (goRight()){
    forwardSlider();
  }
  else if (goUp()) {
    if (slider > -2){
      slider--;
	  updateSliderIndicator();
    if (slider == -2){
      sliderMode = 1;
      
      cursorX = imageRes == 32 ?15:7;
      cursorY = imageRes == 32 ?31:15;

      drawCursor();
    }
    delay(300);
	}
  }
  else if (goDown()) {
    if (slider < 2){
      slider++;
    updateSliderIndicator();
    
    delay(300);
	}
  }
  
  else if (goButton()){
    paintBTNManager();
	  delay(200);
  }
}

// Start Calculator Code

void resetCalc(){
  shouldReset = false;
  result = 0;
  clearMyDisplay();
}

void clearMyDisplay(){
  // Clears the upper part of the display where numbers and functions appear
  tft.fillRect(0, 0, tft.width(), 100, ILI9341_BLACK);
  
  // Reset few stuff
  numberMode = 1;
  displayedText = "";
  currentNumber = "";
  functionID = -1;
  number1 = 0;
  number2 = 0;
}

void updateDisplayedText(uint8_t mode, const String &text) {
  displayedText = text; // Buffer
  
  int num = mode;
  tft.setCursor(20, 10 + (num - 1) * 30); // Depends on the number mode, it puts it in a particular row
  tft.setTextColor(ILI9341_WHITE);
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
  tft.setTextColor(ILI9341_WHITE);
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
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("= "); // Equals to:
  tft.print(resultString);
  
  // After any input, it will clear the box
  shouldReset = true;
}

void drawCalculatorInterface() {
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  updateCalculatorButton(-2, -2, true, false);
}

void updateCalculatorButton(int8_t current, int8_t previous, bool all, bool alt){
  // If it doesn't update all, It updates only the current and the previous selected buttons
  if (alt){ //Up-down alternative reloading
    if (current == 0 || previous == 0 || previous == 16) drawButton("1", 10+spacing, 120, highlightID == 0);
    if (current == 1 || previous == 1 || previous == 17) drawButton("2", 10+spacing + buttonWidth + spacing, 120, highlightID == 1);
    if (current == 2 || previous == 2 || previous == 18) drawButton("3", 10+spacing + 2 * (buttonWidth + spacing), 120, highlightID == 2);
    if (current == 3 || previous == 3 || previous == 19) drawButton("+", 10+spacing + 3 * (buttonWidth + spacing), 120, highlightID == 3);
  }else{
    if (current == 0 || previous == 0 || previous == 16 || all) drawButton("1", 10+spacing, 120, highlightID == 0);
    if (current == 1 || previous == 1 || all) drawButton("2", 10+spacing + buttonWidth + spacing, 120, highlightID == 1);
    if (current == 2 || previous == 2 || all) drawButton("3", 10+spacing + 2 * (buttonWidth + spacing), 120, highlightID == 2);
    if (current == 3 || previous == 3 || all) drawButton("+", 10+spacing + 3 * (buttonWidth + spacing), 120, highlightID == 3);
  }

  if (current == 4 || previous == 4 || all) drawButton("4", 10+spacing, 120 + buttonHeight + spacing, highlightID == 4);
  if (current == 5 || previous == 5 || all) drawButton("5", 10+spacing + buttonWidth + spacing, 120 + buttonHeight + spacing, highlightID == 5);
  if (current == 6 || previous == 6 || all) drawButton("6", 10+spacing + 2 * (buttonWidth + spacing), 120 + buttonHeight + spacing, highlightID == 6);
  if (current == 7 || previous == 7 || all) drawButton("-", 10+spacing + 3 * (buttonWidth + spacing), 120 + buttonHeight + spacing, highlightID == 7);

  if (current == 8 || previous == 8 || all) drawButton("7", 10+spacing, 120 + 2 * (buttonHeight + spacing), highlightID == 8);
  if (current == 9 || previous == 9 || all) drawButton("8", 10+spacing + buttonWidth + spacing, 120 + 2 * (buttonHeight + spacing), highlightID == 9);
  if (current == 10 || previous == 10 || all) drawButton("9", 10+spacing + 2 * (buttonWidth + spacing), 120 + 2 * (buttonHeight + spacing), highlightID == 10);
  if (current == 11 || previous == 11 || all) drawButton("*", 10+spacing + 3 * (buttonWidth + spacing), 120 + 2 * (buttonHeight + spacing), highlightID == 11);

  if (alt){ //Up-down alternative reloading
    if (current == 12 || previous == 12 || previous == -4) drawButton("C", 10+spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 12);
    if (current == 13 || previous == 13 || previous == -3) drawButton("0", 10+spacing + buttonWidth + spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 13);
    if (current == 14 || previous == 14 || previous == -2) drawButton("=", 10+spacing + 2 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 14);
    if (current == 15 || previous == 15 || previous == -1) drawButton("/", 10+spacing + 3 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 15);
  }
  else{
    if (current == 12 || previous == 12 || all) drawButton("C", 10+spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 12);
    if (current == 13 || previous == 13 || all) drawButton("0", 10+spacing + buttonWidth + spacing, 120 + 3 * (buttonHeight + spacing), highlightID == 13);
    if (current == 14 || previous == 14 || all) drawButton("=", 10+spacing + 2 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 14);
    if (current == 15 || previous == 15 || previous == -1 || all) drawButton("/", 10+spacing + 3 * (buttonWidth + spacing), 120 + 3 * (buttonHeight + spacing), highlightID == 15);
  }
  // special cases -1 and 16, because it goes off bounds otherwise
}

void drawButton(const char *label, int x, int y, bool highlight) {
  if (highlight){
    tft.fillRect(x, y, buttonWidth, buttonHeight, 0b1010110011011000); // Very light blue
    tft.setTextColor(ILI9341_BLACK);
  }
  else{
    tft.fillRect(x, y, buttonWidth, buttonHeight, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE);
  }
  tft.setCursor(x + buttonWidth / 4, y + buttonHeight / 4 + 5);
  tft.setTextSize(2);
  tft.print(label);
}

void button0pressAction(){
  if (shouldReset){
    resetCalc();
  }
  
  highlightID--;
  if (highlightID < 0){
    highlightID = 15;
  }
  updateCalculatorButton(highlightID, highlightID + 1, false, false);
}

void button1pressAction(){
  if (shouldReset){
    resetCalc();
  }
  
  highlightID++;
  if (highlightID > 15){
    highlightID = 0;
  }
  updateCalculatorButton(highlightID, highlightID - 1, false, false);
}

void button0pressActionB(){
  if (shouldReset){
    resetCalc();
  }
  
  highlightID-=4;
  if (highlightID < -3) highlightID = 12;
  if (highlightID < -2) highlightID = 13;
  if (highlightID < -1) highlightID = 14;
  if (highlightID < 0) highlightID = 15;
  updateCalculatorButton(highlightID, highlightID + 4, false, true);
}

void button1pressActionB(){
  if (shouldReset){
    resetCalc();
  }
  
  highlightID+=4;
  if (highlightID > 18) highlightID = 3;
  if (highlightID > 17) highlightID = 2;
  if (highlightID > 16) highlightID = 1;
  if (highlightID > 15) highlightID = 0;
  updateCalculatorButton(highlightID, highlightID - 4, false, true);
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
        numberID++;   // 1, 2 and 3
      }
      else if (highlightID == 4 || highlightID == 5 || highlightID == 6){
                      // 4, 5 and 6
      }
      else if (highlightID == 8 || highlightID == 9 || highlightID == 10){
        numberID--;   // 7, 8 and 9
      }
      else if (highlightID == 13){
        numberID = 0; // 0
      }else{
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

void getRTCCalendarData(){
  RTCKeepAliveCheck();
  // Get current date and time from the RTC
  if (RTCFailure){
	  currentDay = 0;
	  currentMonth = 0;
	  currentYear = 2023;
  }else{
    RtcDateTime now = Rtc.GetDateTime();
    currentDay = now.Day();
    currentYear = now.Year();
    currentMonth = now.Month();
	
	if (now.Hour() == 23 && now.Minute() == 59) currentDay++; //Fix for day updating at 12:01 AM
	
    currentMonth--;
    // Done? Now continue loading...
  }
}

void drawCalendar(int year, uint8_t monthIndex) {
  calendarYear = year;
  calendarMonth = monthIndex;
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  int xOffset = 20;
  int yOffset = 100; // Adjust this value to move the calendar up-down
  int cellWidth = 30;
  int cellHeight = 30;
  
  tft.setTextSize(2);
  tft.setTextColor(0xC618);
  
  // Load the days
  for (int day = 0; day < 7; ++day) {
    int x = 8 + xOffset + day * cellWidth;
    int y = yOffset;
  
    if (day == 5) tft.setTextColor(ILI9341_RED); // Saturday and Sunday are RED
    
    tft.setCursor(x, y);
    tft.print(dayNames[day]);
  }
  
  printCalendarLabel(yOffset);

  yOffset += cellHeight;
  
  int daysInMonth = monthDays[monthIndex];
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) { // Proper leap year check
    if (monthIndex == 1) daysInMonth++; // February has 29 days in a leap year
  }
  
  int startingDay = dayOfWeek(year, monthIndex + 1, 1);
  
  for (int day = 1; day <= daysInMonth; ++day) {
    int x = xOffset + ((day - 1 + startingDay) % 7) * cellWidth;
    int y = yOffset + ((day - 1 + startingDay) / 7) * cellHeight;
  
    // Draw a square around the day's number
    tft.drawRect(x, y, cellWidth, cellHeight, ILI9341_WHITE);
  
    // Calculate the center of the square for the text positioning
    int textX = x + cellWidth / 2 - (day >= 10 ? 10 : 5);
    int textY = y + cellHeight / 2 - 8;
  
    tft.setCursor(textX, textY);
    tft.setTextColor(ILI9341_WHITE);
    if (((day - 1 + startingDay) % 7) >= 5) tft.setTextColor(0xFACA); // Saturday and Sunday are RED
	if (day == currentDay && monthIndex == currentMonth && year == currentYear) tft.setTextColor(ILI9341_GREEN);
    tft.print(day);
  }
}

void printCalendarLabel(uint8_t yOffset){
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(60, yOffset - 80);
  
  int posX = 55; // The same as the month label
  int monthGridSpaces = String(monthNames[calendarMonth]).length();
  int textWidth = monthGridSpaces * 11; // Each letter in this context takes 11 units
  if (yearMode){
    textWidth = 48; // Only cover the year
    posX += ((monthGridSpaces + 2) * 11) - 5; // Move the cursor to only cover the year
  }
  tft.fillRect(50, yOffset - 85, 200, 30, ILI9341_BLACK);
  tft.fillRect(posX, yOffset - 85, textWidth + 10, 30, ILI9341_YELLOW); // Adjust the size and color as needed
  
  //Show month and year on top
  if (!yearMode) tft.setTextColor(ILI9341_BLACK);
  tft.print(monthNames[calendarMonth]);
  tft.print(" ");
  if (yearMode) tft.setTextColor(ILI9341_BLACK);
  else tft.setTextColor(ILI9341_YELLOW);
  tft.print(calendarYear);
}

int dayOfWeek(int year, uint8_t month, uint8_t day) {
  
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
  tft.drawRect(notepadGridX1, notepadGridX1, notepadGridX2, notepadGridY2, 0b1010110011011000); // Very light blue
  tft.drawRect(notepadGridX1 + 1, notepadGridX1 + 1, notepadGridX2 - 1, notepadGridY2 - 1, 0b1010110011011000); // Very light blue
  tft.drawRect(notepadGridX1 + 1, notepadGridX1 + 1, notepadGridX2 - 2, notepadGridY2 - 2, 0b1010110011011000); // Very light blue
  tft.drawRect(notepadGridY1 - 1, notepadGridY1 - 1, notepadGridX2 - 3, notepadGridY2 - 3, 0b1010110011011000); // Very light blue
  tft.drawRect(notepadGridY1 - 1, notepadGridY1 - 1, notepadGridX2 - 4, notepadGridY2 - 4, 0b1010110011011000); // Very light blue
  tft.fillRect(notepadGridY1, notepadGridY1, notepadGridX2 - 5, notepadGridY2 - 5, ILI9341_BLACK);

  // Additional keyboard squares
  int displayHeight = tft.height();  // Height of the display
  int keyboardHeight = numRows * (keyHeight + keySpacing) + 25;  // Height of the keyboard
  int margin = (displayHeight - keyboardHeight);
  
  tft.drawRect(0, margin, tft.width(), keyboardHeight, ILI9341_WHITE);
  tft.drawRect(1, margin + 1, tft.width() - 2, keyboardHeight - 2, ILI9341_WHITE);
  
  //TheTextbox = "";
  drawText(TheTextbox);
}

void drawText(String text){
  if (text.length() < notepadSize){ // Don't put if it's last character
    text = text + "_"; // Cursor
  }
  
  tft.setTextColor(currentColor);
  tft.setTextSize(2);
  
  // Proper text wrap, only inside the yellow rectangle
  int x = 10;
  int y = 10;

  for (int i = 0; i < text.length() + 1; ++i) {
    if (y < maxYGridRender){ // Check this, so it doesn't go off screen and delete part of yellow border
      tft.fillRect(x, y, 12, 20, ILI9341_BLACK);
    }
    tft.setCursor(x, y);
    tft.print(text.charAt(i));
    
    x += 12; // Adjust the x position based on character width
    if ((i + 1) % 18 == 0) {
      y += 20; // Move to the next row
      x = 10;  // Reset x position for the new row
    }
  }
}

void ChangeKeyboard(){
  if (keyboardMode == 0) keyboardMode = 1;
  else if (keyboardMode == 1) keyboardMode = 2;
  else if (keyboardMode == 2) keyboardMode = 0;
  
  drawKeyboard(-3, -3, true);
}

void KeyPress0(){
  currentKey--;
  if (currentKey < 0){
    currentKey = 33;
  }
  drawKeyboard(currentKey, currentKey + 1, false);
}

void KeyPress1(){
  currentKey++;
  if (currentKey > 33){
    currentKey = 0;
  }
  drawKeyboard(currentKey, currentKey - 1, false);
}

void KeyPress0b(){
  int8_t prevKey = currentKey;
  
  if (currentKey <= 1) currentKey = 29;
  else if (currentKey <= 3) currentKey = 30;
  else if (currentKey <= 5) currentKey = 31;
  else if (currentKey <= 7) currentKey = 32;
  else if (currentKey <= 9) currentKey = 33;
  else if (currentKey <= 18) currentKey -= 10;
  else if (currentKey <= 25) currentKey -= 8;
  else if (currentKey == 26) currentKey = 19;
  else if (currentKey == 27) currentKey = 22;
  else if (currentKey == 28) currentKey = 25;
  else if (currentKey == 29) currentKey = 26;
  else if (currentKey <= 32) currentKey = 27;
  else if (currentKey == 33) currentKey = 28;
  
  drawKeyboard(currentKey, prevKey, false);
}

void KeyPress1b(){
  int8_t prevKey = currentKey;
  
  if (currentKey == 33) currentKey = 8;
  else if (currentKey == 32) currentKey = 6;
  else if (currentKey == 31) currentKey = 4;
  else if (currentKey == 30) currentKey = 2;
  else if (currentKey == 29) currentKey = 0;
  else if (currentKey == 28) currentKey = 33;
  else if (currentKey == 27) currentKey = 31;
  else if (currentKey == 26) currentKey = 29;
  else if (currentKey == 25) currentKey = 28;
  else if (currentKey >= 20) currentKey = 27;
  else if (currentKey == 19) currentKey = 26;
  else if (currentKey == 18) currentKey = 25;
  else if (currentKey >= 11) currentKey += 8;
  else if (currentKey == 10) currentKey = 19;
  else if (currentKey == 9) currentKey = 18;
  else if (currentKey >= 0) currentKey += 10;
  
  drawKeyboard(currentKey, prevKey, false);
}

// r y g b w //29-33
//qwertyuiop //0-9
//asdfghjkl //10-18
// zxcvbnm //19-25
//del --- mode //26, 27, 28

void KeyPress2() {
  if (currentKey < 26 && TheTextbox.length() < notepadSize){
    String keyboard = "";
    if (keyboardMode == 0) keyboard =        "QWERTYUIOPASDFGHJKLZXCVBNM";
    else if (keyboardMode == 1) keyboard =   "qwertyuiopasdfghjklzxcvbnm";
    else if (keyboardMode == 2) keyboard =   "1234567890+-*/=.,?!()#$%:\"";
    TheTextbox += keyboard[currentKey];
    drawText(TheTextbox);
  }else if (currentKey == 26){
    // Delete, removes last char on the textbox
    if (TheTextbox.length() > 0) {
      // Unused code, deletes all space characters, but has performance issues
      //if (TheTextbox.charAt(TheTextbox.length() - 1) == ' '){
      //  while (TheTextbox.charAt(TheTextbox.length() - 1) == ' '){ // Loop repeat to remove all empty chars
      //    TheTextbox = TheTextbox.substring(0, TheTextbox.length() - 1);
      //    drawText(TheTextbox);
      //  }
      //}else{
      // Just remove one character
      TheTextbox = TheTextbox.substring(0, TheTextbox.length() - 1);
      drawText(TheTextbox);
      ////}
    }
  }else if (currentKey == 27 && TheTextbox.length() < notepadSize){
    // Spacebar
    TheTextbox += " ";
    drawText(TheTextbox);
  }else if (currentKey == 28 && TheTextbox.length() < notepadSize){
    //// Enter, adds enough space characters until it fills the row
    //int seperate = 18 - (TheTextbox.length() % 18); // Calculate how many characters can fit in the row?
    //for (int i = 0; i < seperate; i++) TheTextbox += " "; // Fill the places
    //drawText(TheTextbox);
  
    //NEW! Change keyboard mode
    ChangeKeyboard();
  }else if (currentKey > 28){
    // Color keys, update the color of the text
    currentColor = validColors[currentKey - 29];
    drawText(TheTextbox);
  }
}

void drawKeyboard(int8_t a, int8_t b, bool all) {
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
      if (nextKey == a || nextKey == b || all || (b == 34 && nextKey == 0)){ // Special case out of bounds, just like the calculator
        if (keyboardMode == 0) drawKey(x, y, keyWidth, keyHeight, String(qwerty[row][col]), 2, nextKey);
        else if (keyboardMode == 1) drawKey(x, y, keyWidth, keyHeight, String(qwerty2[row][col]), 2, nextKey);
        else if (keyboardMode == 2) drawKey(x, y, keyWidth, keyHeight, String(symbol[row][col]), 2, nextKey);
      }
    }
  }
  
  if (a == -3 || b == -3) return; // Update only the typing keys

  // Draw spacebar
  int spacebarWidth = keyWidth * 6 + keySpacing * 5;
  int spacebarX = (tft.width() - spacebarWidth) / 2;
  int spacebarY = tft.height() - keyHeight - keySpacing;
  
  if (a == 27 || b == 27 || all)
  drawKey(spacebarX, spacebarY, spacebarWidth, keyHeight, " ", 2, 27);

  // Draw enter key
  // Update, its no longer enter, I changed it to mode
  int enterWidth = keyWidth * 2 + keySpacing;
  int enterX = tft.width() - enterWidth - keySpacing;
  int enterY = spacebarY;
  
  if (a == 28 || b == 28 || all)
  //drawKey(enterX, enterY, enterWidth, keyHeight, "ENTER", 1, 28);
  drawKey(enterX, enterY, enterWidth, keyHeight, " MODE", 1, 28);

  // Draw backspace key
  int backspaceWidth = keyWidth * 2 + keySpacing;
  int backspaceX = keySpacing;
  int backspaceY = spacebarY;
  
  if (a == 26 || b == 26 || all)
  drawKey(backspaceX, backspaceY, backspaceWidth, keyHeight, "DELETE", 1, 26);

  // Draw colors
  int colorX = keySpacing;
  int colorY = keyHeight + 165;
  int colorWidth = keySpacing + 40;
  int colorHeight = keyHeight;
  int colorBTNspacing = 48;
  if (a == 29 || b == 29 || all) drawColorKey(colorX + colorBTNspacing * 0, colorY, colorWidth, colorHeight, ILI9341_RED, " RED ", 29);
  if (a == 30 || b == 30 || all) drawColorKey(colorX + colorBTNspacing * 1, colorY, colorWidth, colorHeight, ILI9341_YELLOW, "YELLOW", 30);
  if (a == 31 || b == 31 || all) drawColorKey(colorX + colorBTNspacing * 2, colorY, colorWidth, colorHeight, ILI9341_GREEN, "GREEN", 31);
  if (a == 32 || b == 32 || all) drawColorKey(colorX + colorBTNspacing * 3, colorY, colorWidth, colorHeight, 0x471A, " BLUE", 32); // 0x471A is better blue
  if (a == 33 || b == 33 || b == -1 || all) drawColorKey(colorX + colorBTNspacing * 4, colorY, colorWidth, colorHeight, ILI9341_WHITE, "WHITE", 33);
  //                              ^, Special case, -1, just like the calculator
}

void drawKey(int x, int y, uint8_t width, uint8_t height, String label, uint8_t keysize, uint8_t keyID) {
  if (keyID == currentKey){
    tft.fillRect(x, y, width, height, ILI9341_WHITE);
  }else{
    // Highlighted key appear in white with black font, the rest appear black with white font
    tft.fillRect(x, y, width, height, ILI9341_BLACK);
    tft.drawRect(x, y, width, height, ILI9341_WHITE);
  }
  if (keysize == 1){ // Alternative calculation for the bigger keys
    tft.setCursor(x + width / 8, y + 5 + height / 6);
  }else{
    tft.setCursor(x + width / 4, y + height / 4);
  }
  if (keyID == currentKey){
    tft.setTextColor(ILI9341_BLACK);
  }else{
    tft.setTextColor(ILI9341_WHITE);
  }
  tft.setTextSize(keysize);
  tft.print(label);
}

void drawColorKey(int colorX, int colorY, uint8_t colorWidth, uint8_t colorHeight, uint16_t color, String label, uint8_t keyID){
  if (keyID == currentKey){
    tft.fillRect(colorX, colorY, colorWidth, colorHeight, color);
  }else{
    // Highlighted key appear in the appropriate color with black font
    tft.fillRect(colorX, colorY, colorWidth, colorHeight, ILI9341_BLACK);
    tft.drawRect(colorX, colorY, colorWidth, colorHeight, color);
  }
  tft.setCursor(colorX + colorWidth / 8, colorY + 5 + colorHeight / 6);
  if (keyID == currentKey){
    tft.setTextColor(ILI9341_BLACK);
  }else{
    tft.setTextColor(0xC618); // Light Gray
  }
  tft.setTextSize(1);
  tft.print(label);
}

// End Notepad Code

// Start Stopwatch Code

// Deprecated, Use updateStopwatchDisplay(0) instead.
//void loadStopwatch() {
//  tft.fillRect(20, 70, 200, 50, ILI9341_BLACK);
//  tft.setCursor(25, 80);
//  tft.setTextSize(4);
//  tft.setTextColor(0b1010110011011000); // Very light blue
//  tft.print("00:00:00");
//}

void updateStopwatchDisplay(unsigned long elapsedSeconds) {
  clockElapsedSeconds = elapsedSeconds; // Remember in memory for later
  
  unsigned long seconds = elapsedSeconds % 60;
  unsigned long minutes = (elapsedSeconds / 60) % 60;
  unsigned long hours = elapsedSeconds / 3600;

  tft.fillRect(20, 70, 200, 50, ILI9341_BLACK);

  tft.setCursor(25, 80);
  tft.setTextSize(4);
  tft.setTextColor(0b1010110011011000); // Very light blue
  tft.print(hours < 10 ? "0" : "");
  tft.print(hours);
  tft.print(":");
  tft.print(minutes < 10 ? "0" : "");
  tft.print(minutes);
  tft.print(":");
  tft.print(seconds < 10 ? "0" : "");
  tft.print(seconds);
}

void displayStatusMessage() {
  
  tft.setTextSize(2);
  uint16_t pauseColor = 0xFD20;
  String pauseLabel = "Pause";
  int pauseX = 30;
  if (paused){
    pauseColor = 0x03E0;
    pauseLabel = "Resume";
    pauseX = 25;
  }
  
  if (running) {
    tft.fillRect(10, 220, 220, 90, ILI9341_BLACK);
  
    if (stopwatchButton == 0){
      tft.drawRect(10, 220, 100, 90, pauseColor);
      tft.setTextColor(pauseColor);
    }else{
      tft.fillRect(10, 220, 100, 90, pauseColor);
      tft.setTextColor(ILI9341_BLACK);
    }
    tft.setCursor(pauseX, 255);
    tft.print(pauseLabel);
    
    if (stopwatchButton == 0){
      tft.fillRect(130, 220, 100, 90, ILI9341_RED);
      tft.setTextColor(ILI9341_BLACK);
    }else{
      tft.drawRect(130, 220, 100, 90, ILI9341_RED);
      tft.setTextColor(ILI9341_RED);
    }
    tft.setCursor(155, 255);
    tft.print("Stop");
      
  } else {
    tft.fillRect(0, 220, 230, 90, ILI9341_BLACK);
  
    tft.fillRect(20, 220, 200, 90, ILI9341_GREEN);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(90, 255);
    tft.print("Start");
  }
}

// End Stopwatch Code

// Start Tic Tac Toe Code

void drawBoard(){
  for (int i = 1; i < 3; i++) {
    tft.drawFastVLine(GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y, CELL_SIZE * 3, ILI9341_WHITE);
    tft.drawFastHLine(GRID_OFFSET_X, GRID_OFFSET_Y + i * CELL_SIZE, CELL_SIZE * 3, ILI9341_WHITE);
  
    int x = GRID_OFFSET_X + i * CELL_SIZE;
    int y = GRID_OFFSET_Y + i * CELL_SIZE;
  
    //Additional thickness
    tft.drawFastVLine(x - 1, GRID_OFFSET_Y, CELL_SIZE * 3, ILI9341_WHITE);
    tft.drawFastVLine(x + 1, GRID_OFFSET_Y, CELL_SIZE * 3, ILI9341_WHITE);
  
    tft.drawFastHLine(GRID_OFFSET_X, y - 1, CELL_SIZE * 3, ILI9341_WHITE);
    tft.drawFastHLine(GRID_OFFSET_X, y + 1, CELL_SIZE * 3, ILI9341_WHITE);
  
    printTurn(currentPlayer);
  }
}

void printTurn(int8_t id){
  if (showWinner){
    tft.fillRect(GRID_OFFSET_X + 10, GRID_OFFSET_Y + 230, 200, 30, ILI9341_BLACK); // Clear player turn
    tft.setTextColor(ILI9341_RED); // Darker than usual red
    tft.setTextSize(2);
    tft.setCursor(GRID_OFFSET_X + 60, GRID_OFFSET_Y + 240);
    tft.print("Game Over!");
  }else{
    //Clear previous
    tft.fillRect(GRID_OFFSET_X + 10, GRID_OFFSET_Y + 230, 200, 30, id == 0 ? 0x07FF : 0xF800); // The opposite color as text background
  
    //Now put the current player
    if (id == 0){
      tft.setTextColor(0xF800); // Red
      tft.setTextSize(2);
      tft.setCursor(GRID_OFFSET_X + 20, GRID_OFFSET_Y + 240);
      tft.print("Player X's Turn");
    }else{
      tft.setTextColor(0x07FF); // Blue
      tft.setTextSize(2);
      tft.setCursor(GRID_OFFSET_X + 20, GRID_OFFSET_Y + 240);
      tft.print("Player O's Turn");
    }
  }
  
  // Refresh the matrix
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (occupiedLoc[i * 3 + j] == 1) {
        drawX(i, j);
      } else if (occupiedLoc[i * 3 + j] == 2) {
        drawO(i, j);
      }
    }
  }
  
  if (showWinner){
    tft.setTextSize(3);
  if (gameTie){
      tft.setCursor(GRID_OFFSET_X + 20, GRID_OFFSET_Y + 270);
      tft.setTextColor(ILI9341_YELLOW);
      tft.print("It's a tie!");
  }
  else{
      tft.setCursor(GRID_OFFSET_X + 30, GRID_OFFSET_Y + 270);
    tft.setTextColor(ILI9341_GREEN);
      tft.print("Winner: " + String(winnerChar));
  }
  }
  drawCursor(currentCursor, -1);
}

void drawX(int8_t row, int8_t col) {
  int x = GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
  int y = GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
  
  tft.drawLine(x - CELL_SIZE / 3, y - CELL_SIZE / 3, x + CELL_SIZE / 3, y + CELL_SIZE / 3, 0xF800); // Red
  tft.drawLine(x + CELL_SIZE / 3, y - CELL_SIZE / 3, x - CELL_SIZE / 3, y + CELL_SIZE / 3, 0xF800);
  
  //Additional thickness
  tft.drawLine(x - CELL_SIZE / 3 - 1, y - CELL_SIZE / 3, x + CELL_SIZE / 3 - 1, y + CELL_SIZE / 3, 0xF800);
  tft.drawLine(x + CELL_SIZE / 3 + 1, y - CELL_SIZE / 3, x - CELL_SIZE / 3 + 1, y + CELL_SIZE / 3, 0xF800);
  
  tft.drawLine(x - CELL_SIZE / 3, y - CELL_SIZE / 3 - 1, x + CELL_SIZE / 3, y + CELL_SIZE / 3 - 1, 0xF800);
  tft.drawLine(x + CELL_SIZE / 3, y - CELL_SIZE / 3 + 1, x - CELL_SIZE / 3, y + CELL_SIZE / 3 + 1, 0xF800);
}

void drawO(int8_t row, int8_t col) {
  int x = GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
  int y = GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;

  tft.drawCircle(x, y, CELL_SIZE / 3, 0x07FF); // Blue
  
  //Additional thickness
  tft.drawCircle(x, y, (CELL_SIZE / 3) - 1, 0x07FF);
}

void drawCursor(int8_t current, int8_t previous) {
  //Static vars
  int row = current / 3;
  int col = current % 3;
  
  int rowB4 = previous / 3;
  int colB4 = previous % 3;
  
  int cursorSize = CELL_SIZE - 10;
  
  //Vars that change depending on the cursor
  int x = GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
  int y = GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;

  int cursorX1 = x - cursorSize / 2;
  int cursorY1 = y - cursorSize / 2;
  int cursorX2 = x + cursorSize / 2;
  int cursorY2 = y + cursorSize / 2;
  
  if (showWinner) tft.drawRect(cursorX1, cursorY1, cursorSize, cursorSize, ILI9341_BLACK); //Game over? Draw black cursor and clear
  else tft.drawRect(cursorX1, cursorY1, cursorSize, cursorSize, currentPlayer == 0 ? 0xFB6D : 0x87FF); // The cursor is the lighter color of the current player
  
  if (previous == -1) return; // Used for start game and player turn, No need for clear
  
  //Now clear the previous cursor
  x = GRID_OFFSET_X + colB4 * CELL_SIZE + CELL_SIZE / 2;
  y = GRID_OFFSET_Y + rowB4 * CELL_SIZE + CELL_SIZE / 2;

  cursorX1 = x - cursorSize / 2;
  cursorY1 = y - cursorSize / 2;
  cursorX2 = x + cursorSize / 2;
  cursorY2 = y + cursorSize / 2;
  
  tft.drawRect(cursorX1, cursorY1, cursorSize, cursorSize, ILI9341_BLACK);
}

void TicTacLeft(){
  if (showWinner){
    resetTicTacToe();
    return;
  }
  
  int prevCursor = currentCursor;
  currentCursor--;
  if (currentCursor < 0) currentCursor = 8;
  drawCursor(currentCursor, prevCursor);
}

void TicTacRight(){
  if (showWinner){
    resetTicTacToe();
    return;
  }
  
  int prevCursor = currentCursor;
  currentCursor++;
  if (currentCursor > 8) currentCursor = 0;
  drawCursor(currentCursor, prevCursor);
}

void TicTacUp(){
  if (showWinner){
    resetTicTacToe();
    return;
  }
  
  int prevCursor = currentCursor;
  currentCursor-=3;
  if (currentCursor < -2) currentCursor = 6;
  if (currentCursor < -1) currentCursor = 7;
  if (currentCursor < 0) currentCursor = 8;
  drawCursor(currentCursor, prevCursor);
}

void TicTacDown(){
  if (showWinner){
    resetTicTacToe();
    return;
  }
  
  int prevCursor = currentCursor;
  currentCursor+=3;
  if (currentCursor > 10) currentCursor = 2;
  if (currentCursor > 9) currentCursor = 1;
  if (currentCursor > 8) currentCursor = 0;
  drawCursor(currentCursor, prevCursor);
}

void TicTacEnter(){
  if (showWinner){
    resetTicTacToe();
    return;
  }
  
  if (occupiedLoc[currentCursor] == 0){
    occupiedLoc[currentCursor] = currentPlayer + 1;
    checkForWinner();
    
    // Everything good? Switch to the next player
    currentPlayer = (currentPlayer + 1) % 2;
    printTurn(currentPlayer);
  }
}

void checkForWinner() {
  // Check for diagonal win
  if ((occupiedLoc[0] == occupiedLoc[4] && occupiedLoc[4] == occupiedLoc[8]) && occupiedLoc[0] != 0) {
    winnerChar = (occupiedLoc[0] == 1) ? 'X' : 'O';
    showWinner = true;
    return;
  }
  
  if ((occupiedLoc[2] == occupiedLoc[4] && occupiedLoc[4] == occupiedLoc[6]) && occupiedLoc[2] != 0) {
    winnerChar = (occupiedLoc[2] == 1) ? 'X' : 'O';
    showWinner = true;
    return;
  }

  // Check for horizontal and vertical wins
  for (int i = 0; i < 3; i++) {
    if ((occupiedLoc[i * 3] == occupiedLoc[i * 3 + 1] && occupiedLoc[i * 3 + 1] == occupiedLoc[i * 3 + 2]) && occupiedLoc[i * 3] != 0) {
      winnerChar = (occupiedLoc[i * 3] == 1) ? 'X' : 'O';
      showWinner = true;
      return; // Horizontal win
    }
    
    if ((occupiedLoc[i] == occupiedLoc[i + 3] && occupiedLoc[i + 3] == occupiedLoc[i + 6]) && occupiedLoc[i] != 0) {
      winnerChar = (occupiedLoc[i] == 1) ? 'X' : 'O';
      showWinner = true;
      return; // Vertical win
    }
  }
  
  // Check for tie game
  bool isTie = true;
  for (int i = 0; i < 9; i++) {
    if (occupiedLoc[i] == 0) {
      isTie = false;
      break;
    }
  }
  if (isTie) {
    showWinner = true;
    gameTie = true;
  }
  
  //showWinner = false; // No winner
  //return;
}

void resetTicTacToe(){
  tft.fillScreen(ILI9341_BLACK);
  currentPlayer = 0;
  currentCursor = 4;
  for (int i = 0; i < 9; i++) occupiedLoc[i] = 0;
  gameTie = false;
  showWinner = false;
  winnerChar = ' ';
  drawBoard();
}

// End Tic Tac Toe Code

// Start Paint Code

void startPaintApp(){
  //Draw main outline
  if (imageRes == 32) tft.drawRect(21, 1, 194, 194, ILI9341_WHITE);
  else tft.drawRect(19, 7, 194, 194, ILI9341_WHITE);

  drawSliders();
  gridBTN(true);
  
  tft.setCursor(120, 205);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Color:");
  
  updateColor();
  updateSliderIndicator();
}

void updateColor(){
  // Draw brush color
  tft.drawRect(199, 204, 17, 17, ILI9341_WHITE);
  tft.fillRect(200, 205, 15, 15, getPaintBrush());
}

void paintBTNManager(){
  if (slider == -1){
    // In range
    if (!paintBTNRide){
      // Grid toggle
      showGrid = !showGrid;
      gridBTN(true);
    }else{
      // Are you sure you wanna delete?
      imageRes = imageRes == 16 ? 32 : 16;

      clearImageMemory();
      defines(); // Calls outside of memory bounds
    }
  }
}

void gridBTN(bool updateGrid){
  tft.fillRect(10, 205, 95, 15, ILI9341_BLACK);
  
  tft.setCursor(10, 205);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(paintBTNRide ? "Res:" : "Grid:");
  
  tft.setCursor(70, 205);
  tft.setTextSize(2);
  if (paintBTNRide){
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(imageRes == 16 ? "16x" : "32x");
  }
  else if (showGrid){
    tft.setTextColor(ILI9341_GREEN);
    tft.print("ON");
  }else{
    tft.setTextColor(ILI9341_RED);
    tft.print("OFF");
  }

  if (updateGrid) drawGrid();
}

void drawGrid() {
  int xAdjust = imageRes == 32 ? 16 : 8;
  int margin = imageRes == 32 ? 6 : 12;

  drawImage();

  if (!showGrid) return;
  
  // Draw vertical lines
  for (int i = 0; i <= imageRes; i++) {
    int x = (i + 1) * margin;
   // if (!showGrid && i != 0 && i != imageRes) continue;
    tft.drawFastVLine(x + xAdjust, margin - negativeYOffset, 192, ILI9341_WHITE);
  }

  // Draw horizontal lines
  for (int i = 0; i <= imageRes; i++) {
    int y = (i + 1) * margin;
   // if (!showGrid && i != 0 && i != imageRes) continue;
    tft.drawFastHLine(margin + xAdjust, y - negativeYOffset, 192, ILI9341_WHITE);
  }
}

void drawSliders() {
  int xAdjust = 8;
  
  tft.drawRect(xAdjust + 9, 229, xAdjust + 202, 22, ILI9341_WHITE);
  int rSliderPos = map(rSlider, 0, 255, -8, 200);
  tft.fillRect(xAdjust + 10, 230, xAdjust + rSliderPos, 20, ILI9341_RED);

  // Draw G slider
  tft.drawRect(xAdjust + 9, 259, xAdjust + 202, 22, ILI9341_WHITE);
  int gSliderPos = map(gSlider, 0, 255, -8, 200);
  tft.fillRect(xAdjust + 10, 260, xAdjust + gSliderPos, 20, ILI9341_GREEN);

  // Draw B slider
  tft.drawRect(xAdjust + 9, 289, xAdjust + 202, 22, ILI9341_WHITE);
  int bSliderPos = map(bSlider, 0, 255, -8, 200);
  tft.fillRect(xAdjust + 10, 290, xAdjust + bSliderPos, 20, ILI9341_BLUE);
}

void backwardSlider(){
  if (slider == 0 && rSlider > 0){
    rSlider--;
    updateRedSlider(false);
    delay(colorSlideDelay);
  }
  if (slider == 1 && gSlider > 0){
    gSlider--;
    updateGreenSlider(false);
    delay(colorSlideDelay);
  }
  if (slider == 2 && bSlider > 0){
    bSlider--;
    updateBlueSlider(false);
    delay(colorSlideDelay);
  }

  if (slider == -1){
    paintBTNRide = !paintBTNRide;
    gridBTN(false);
    delay(250);
  }
}

void forwardSlider(){
  if (slider == 0 && rSlider < 255){
    rSlider++;
    updateRedSlider(true);
    delay(colorSlideDelay);
  }
  if (slider == 1 && gSlider < 255){
    gSlider++;
    updateGreenSlider(true);
    delay(colorSlideDelay);
  }
  if (slider == 2 && bSlider < 255){
    bSlider++;
    updateBlueSlider(true);
    delay(colorSlideDelay);
  }

  if (slider == -1){
    paintBTNRide = !paintBTNRide;
    gridBTN(false);
    delay(250);
  }
}

void updateRedSlider(bool forward) {
  int xAdjust = 8;
  if (!forward){
    // Calculate the previous and new positions of the slider
    int prevPos = map(rSlider + 1, 0, 255, -8, 200);
    int newPos = map(rSlider, 0, 255, -8, 200);
	
	tft.fillRect(xAdjust + 18 + newPos, 230, xAdjust + prevPos - newPos, 20, ILI9341_BLACK);
	
	//Overlap fix
	tft.drawFastVLine(xAdjust + xAdjust + 8 + 202, 229, 22, ILI9341_WHITE);
  }else{
    // Calculate the previous and new positions of the slider
    int prevPos = map(rSlider + 1, 0, 255, -8, 200);
    int newPos = map(rSlider, 0, 255, -8, 200);
	
	//Underlap fix
	if (newPos < -1) newPos = -1;
	
	tft.fillRect(xAdjust + 10 + newPos, 230, xAdjust + prevPos - newPos, 20, ILI9341_RED);
	
	//Underlap fix end
	tft.drawFastVLine(xAdjust + 9, 229, 22, ILI9341_WHITE);
  }
  updateColor();
}

void updateGreenSlider(bool forward) {
  int xAdjust = 8;
  if (!forward){
    // Calculate the previous and new positions of the slider
    int prevPos = map(gSlider + 1, 0, 255, -8, 200);
    int newPos = map(gSlider, 0, 255, -8, 200);
	
	tft.fillRect(xAdjust + 18 + newPos, 260, xAdjust + prevPos - newPos, 20, ILI9341_BLACK);
	
	//Overlap fix
	tft.drawFastVLine(xAdjust + xAdjust + 8 + 202, 259, 22, ILI9341_WHITE);
  }else{
    // Calculate the previous and new positions of the slider
    int prevPos = map(gSlider + 1, 0, 255, -8, 200);
    int newPos = map(gSlider, 0, 255, -8, 200);
	
	//Underlap fix
	if (newPos < -1) newPos = -1;
	
	tft.fillRect(xAdjust + 10 + newPos, 260, xAdjust + prevPos - newPos, 20, ILI9341_GREEN);
	
	//Underlap fix end
	tft.drawFastVLine(xAdjust + 9, 259, 22, ILI9341_WHITE);
  }
  updateColor();
}

void updateBlueSlider(bool forward) {
  int xAdjust = 8;
  if (!forward){
    // Calculate the previous and new positions of the slider
    int prevPos = map(bSlider + 1, 0, 255, -8, 200);
    int newPos = map(bSlider, 0, 255, -8, 200);
	
	tft.fillRect(xAdjust + 18 + newPos, 290, xAdjust + prevPos - newPos, 20, ILI9341_BLACK);
	
	//Overlap fix
	tft.drawFastVLine(xAdjust + xAdjust + 8 + 202, 289, 22, ILI9341_WHITE);
  }else{
    // Calculate the previous and new positions of the slider
    int prevPos = map(bSlider + 1, 0, 255, -8, 200);
    int newPos = map(bSlider, 0, 255, -8, 200);
	
	//Underlap fix
	if (newPos < -1) newPos = -1;
	
	tft.fillRect(xAdjust + 10 + newPos, 290, xAdjust + prevPos - newPos, 20, ILI9341_BLUE);
	
	//Underlap fix end
	tft.drawFastVLine(xAdjust + 9, 289, 22, ILI9341_WHITE);
  }
  updateColor();
}

void updateSliderIndicator(){
  tft.fillCircle(5, 240, 3, ILI9341_BLACK);
  tft.fillCircle(5, 270, 3, ILI9341_BLACK);
  tft.fillCircle(5, 300, 3, ILI9341_BLACK);
  if (slider == -2 || slider == 0) tft.drawRect(7, 202, 99, 21, ILI9341_BLACK);
  else if (slider == -1) tft.drawRect(7, 202, 99, 21, ILI9341_WHITE);

  if (slider == 0) tft.fillCircle(5, 240, 3, ILI9341_GREEN);
  else if (slider == 1) tft.fillCircle(5, 270, 3, ILI9341_GREEN);
  else if (slider == 2) tft.fillCircle(5, 300, 3, ILI9341_GREEN);
}

void moveCursor(int deltaX, int deltaY) {
  if (cursorY == (imageRes - 1) && deltaY == 1){
    clearCursor();

    tft.drawRect(7, 202, 99, 21, ILI9341_WHITE);

    sliderMode = 0;
    slider = -1;

    return;
  }
  int newCursorX = constrain(cursorX + deltaX, 0, imageRes - 1);
  int newCursorY = constrain(cursorY + deltaY, 0, imageRes - 1);

  clearCursor();

  cursorX = newCursorX;
  cursorY = newCursorY;

  drawCursor();
}

void drawCursor() {
  int xAdjust = imageRes == 32 ? 16 : 8;
  int margin = imageRes == 32 ? 6 : 12;
  int cursorXPos = (cursorX + 1) * margin + xAdjust;
  int cursorYPos = (cursorY + 1) * margin;

  if (!showGrid) tft.fillRect(cursorXPos, cursorYPos + 1 - negativeYOffset, margin, margin, imageData[cursorY * imageRes + cursorX]);
  else tft.fillRect(cursorXPos + 1, cursorYPos + 1 - negativeYOffset, margin - 1, margin - 1, imageData[cursorY * imageRes + cursorX]);
  
  if (showGrid) tft.fillRect(cursorXPos + 2, cursorYPos + 2 - negativeYOffset, margin - 3, margin - 3, getPaintBrush());
  else tft.fillRect(cursorXPos + 1, cursorYPos + 2 - negativeYOffset, margin - 2, margin - 2, getPaintBrush());
}

void clearCursor() {
  int xAdjust = imageRes == 32 ? 16 : 8;
  int margin = imageRes == 32 ? 6 : 12;
  int cursorXPos = (cursorX + 1) * margin + xAdjust;
  int cursorYPos = (cursorY + 1) * margin;

  tft.fillRect(cursorXPos + 1, cursorYPos + 1 - negativeYOffset, margin - 1, margin - 1, imageData[cursorY * imageRes + cursorX]);
}

void drawImage() {
  int xAdjust = imageRes == 32 ? 16 : 8;
  int margin = imageRes == 32 ? 6 : 12;
  int squareSize = imageRes == 32 ? 6 : 12;

  int startX = imageRes == 32 ? 22 : 20;
  int startY = imageRes == 32 ? 7 : 13;

  for (int y = 0; y < imageRes; y++) {
    for (int x = 0; x < imageRes; x++) {
      int index = y * imageRes + x;
      uint16_t pixelColor = imageData[index];

      int xPos = x * margin + startX;
      int yPos = y * margin + startY;

      // Draw a filled rectangle for each pixel
      tft.fillRect(xPos, yPos - negativeYOffset, squareSize, squareSize, pixelColor);
    }
  }
}

void addPaintColor(){
  imageData[cursorY * imageRes + cursorX] = getPaintBrush();
  drawCursor();
}

void clearImageMemory() {
  for (int i = 0; i < 1024; i++) {
    imageData[i] = 0;
  }

  // Reset screen all modes
  tft.fillRect(18, 0, 198, 201, ILI9341_BLACK);
}

// End Paint Code