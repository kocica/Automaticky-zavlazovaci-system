//Zkusebni programek pro I2C LCD 20x4
//JDC (c)2015
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int x,y = 0;
int sx,sy = 1;
String Text = "Ahoj";

void setup() {
     Serial.begin(9600);
     lcd.begin(20,4);
     lcd.backlight();     
}

void loop()
  {
     lcd.clear();
     Uprav(x,sx,20-Text.length());
     Uprav(y,sy,3);
     lcd.setCursor(x,y);    
     lcd.print(Text);
     delay(250);
  }
  
int Uprav(int &x, int &sx, int maxx) {
   if (sx) {
      x++;
      if (x>=maxx) sx=0;
    } else {
      x--;
      if (x<=0) sx=1; 
    }
}  
