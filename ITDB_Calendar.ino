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

const char* monthNames[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

const int monthDays[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char* dayNames[] = {
  "M", "T", "W", "T", "F", "S", "S"
};

int currentMonth = 8 - 1;
int currentYear = 2023;
const int numMonths = sizeof(monthNames) / sizeof(monthNames[0]);

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  
  tft.begin();
  tft.fillScreen(ILI9340_BLACK);
  drawCalendar(currentYear, currentMonth);
}

void loop() {
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


void drawCalendar(int year, int monthIndex) {
  
  tft.fillScreen(ILI9340_BLACK);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);

  int xOffset = 20;
  int yOffset = 100; // Adjust this value to move the calendar down
  int cellWidth = 30;
  int cellHeight = 30;
  
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_RED);
  
  for (int day = 0; day < 7; ++day) {
    int x = 8 + xOffset + day * cellWidth;
    int y = yOffset;
    
    tft.setCursor(x, y);
    tft.print(dayNames[day]);
  }
  
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_YELLOW);
  tft.setCursor(60, yOffset - 80);
  tft.print(monthNames[monthIndex]);
  tft.print(" ");
  tft.print(year);
  
  yOffset += cellHeight;
  
  int daysInMonth = monthDays[monthIndex];
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
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
  if (month < 3) {
    month += 12;
    year--;
  }
  return (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
}
