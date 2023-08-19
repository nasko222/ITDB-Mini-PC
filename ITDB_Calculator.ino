#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

const int buttonWidth = 50;
const int buttonHeight = 40;
const int spacing = 5;

bool shouldReset = false;

int highlightID = 12;
int numberMode = 1;

String displayedText = "";
String currentNumber = "";
int functionID = -1;
long number1 = 0;
long number2 = 0;
double result = 0;

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  tft.begin();
  tft.fillScreen(ILI9340_BLACK);
  drawCalculatorInterface();
}

void resetCalc(){
  shouldReset = false;
  result = 0;
  clearMyDisplay();
}

void clearMyDisplay(){
  tft.fillRect(0, 0, tft.width(), 100, ILI9340_BLACK);
  numberMode = 1;
  displayedText = "";
  currentNumber = "";
  functionID = -1;
  number1 = 0;
  number2 = 0;
}

void updateDisplayedText(int mode, const String &text) {
  displayedText = text;
  int num = mode;
  tft.setCursor(20, 10 + (num - 1) * 30);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.print(displayedText);
}

void addFunction() {
  String text = ".";
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
  
  displayedText = text;
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

  char resultString[20]; // Adjust the size based on your needs
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

  tft.setCursor(20, 10);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.print("= ");
  tft.print(resultString);

  shouldReset = true;
}

void loop() {
  if (digitalRead(2) == LOW) {
    button1pressAction();
    delay(200);
  }
  else if (digitalRead(3) == LOW) {
    button2pressAction();
    delay(200);
  }
}

void drawCalculatorInterface() {
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_WHITE);
  updateCalculatorButton(-2, -2, true);
}

void updateCalculatorButton(int current, int previous, bool all){
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
}

void drawButton(const char *label, int x, int y, bool highlight) {
  if (highlight){
    tft.fillRect(x, y, buttonWidth, buttonHeight, 0b1010110011011000);
    tft.setTextColor(ILI9340_BLACK);
  }
  else{
    tft.fillRect(x, y, buttonWidth, buttonHeight, ILI9340_BLUE);
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setCursor(x + buttonWidth / 4, y + buttonHeight / 4 + 5); // Adjusted y position
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
    currentNumber = "";
    clearMyDisplay();
  }
  if (highlightID == 7 && currentNumber.length() == 0){
    currentNumber = "-";
    updateDisplayedText(numberMode, currentNumber);
  }
  if (currentNumber.length() > 0 && currentNumber != "-" && numberMode == 1){
    if (highlightID == 3 || highlightID == 7 || highlightID == 11 || highlightID == 15){
      numberMode = 2;
      addFunction();
      number1 = currentNumber.toInt();
      currentNumber = "";
    }
  }
  if (currentNumber.length() > 0 && currentNumber != "-" && numberMode == 2 && highlightID == 14){
    equalFunc();
  }
  if (currentNumber.length() < 8) { // Check if input length is less than 5
    if (highlightID == 13 && currentNumber.length() == 0){
    }
    else{
      int numberID = highlightID;
      if (highlightID == 0 || highlightID == 1 || highlightID == 2){
        numberID++;
      }
      else if (highlightID == 4 || highlightID == 5 || highlightID == 6){
      }
      else if (highlightID == 8 || highlightID == 9 || highlightID == 10){
        numberID--;
      }else if (highlightID == 13){
        numberID = 0;
      }else{
        return;
      }
      currentNumber += String(numberID);
      updateDisplayedText(numberMode, currentNumber);
    }
  }
}
