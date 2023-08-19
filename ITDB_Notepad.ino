#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

// Define display pins
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

const char* qwerty[] = {
  "QWERTYUIOP",
  "ASDFGHJKL",
  "ZXCVBNM"
};

const int numRows = sizeof(qwerty) / sizeof(qwerty[0]);
const int keyWidth = 22;
const int keyHeight = 22;
const int keySpacing = 2;

int currentKey = 27;
String TheTextbox = "";

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9340_BLACK);
  drawKeyboard(-2, -2, true);
  clearTextBox();
}

void clearTextBox(){
  tft.drawRect(5, 5, 230, 200, ILI9340_YELLOW);
  tft.drawRect(6, 6, 229, 199, ILI9340_YELLOW);
  tft.drawRect(6, 6, 228, 198, ILI9340_YELLOW);
  tft.drawRect(6, 6, 227, 197, ILI9340_YELLOW);
  tft.drawRect(6, 6, 226, 196, ILI9340_YELLOW);
  tft.fillRect(7, 7, 225, 195, ILI9340_BLACK);

  int displayHeight = tft.height();  // Height of the display
  int keyboardHeight = numRows * (keyHeight + keySpacing) + 25;  // Height of the keyboard
  int margin = (displayHeight - keyboardHeight);  // Adjust this value as needed
  tft.drawRect(0, margin, tft.width(), keyboardHeight, ILI9340_WHITE);
  tft.drawRect(1, margin + 1, tft.width() - 2, keyboardHeight - 2, ILI9340_WHITE);

  TheTextbox = "";
  drawText(TheTextbox + "_");
}

void loop() {
  if (digitalRead(2) == LOW) {
    KeyPress1();
    delay(200);
  }
  else if (digitalRead(3) == LOW) {
    KeyPress2();
    delay(200);
  }
}

void drawText(String text){
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);

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
    drawText(TheTextbox + "_");
  }else if (currentKey == 26){
    if (TheTextbox.length() > 0) {
        TheTextbox = TheTextbox.substring(0, TheTextbox.length() - 1);
        drawText(TheTextbox + "_");
    }
  }else if (currentKey == 27){
    TheTextbox += " ";
    drawText(TheTextbox + "_");
  }else if (currentKey == 28){
    int seperate = 18 - (TheTextbox.length() % 18);
    for (int i = 0; i < seperate; i++) TheTextbox += " ";
    drawText(TheTextbox + "_");
  }
  
}





void drawKeyboard(int a, int b, bool all) {
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
  if (a == 28 || b == -1 || all)
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
    tft.fillRect(x, y, width, height, ILI9340_BLACK);
    tft.drawRect(x, y, width, height, ILI9340_WHITE);
  }
  if (keysize == 1){
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
