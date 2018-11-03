  /*
Kočica Filip, ME 3
16.5.2015

Program pro řízení zavlažovacího systému.

Na display se lze pohybovat kurzorem ve tvaru šipky pomocí tlačítek na membránové klávesnici.

-------------------------------------------------------------------------------------

Na prvním řádku LCD displaye(20x4) jsou dny Po-Ne, kde můžete nastavit
který den, v jakou hodinu a na jak dlouho budou sepnuty výstupy(OUT 1,2,3).

Na druhém řádku nastavujete vlhkost,hladinu,obnovení vstupů a limit displaye.
Nastavení vlhkosti a hladiny bude porovnáváno s hodnotami ze vstupů,pokud
NV<MV(nastavená vlhkost bude menší než měřená vlhkost) a zároveň NH<MH (nastavená
hladina bude menší než měřená hladina)Bude možno výstupy sepnout,v opačném případě
(NV>MV nebo NH>MH) výstupy nebude možno sepnout.
Obnovení vstupů znamená, po jaké době se znovu načtou hodnoty ze vstupů.
Po načtení zůstane hodnota stejná do té doby,než se znovu načtou vstupní hodnoty.
Limit displaye znamená, po jaké době se vypne podsvícení displaye,opětovné
zapnutí podsvícení displaye je možno provést pomocí libovolného tlačítka.

Na třetím řádku je zobrazena MV(měřená vlhkost) MH(měřená hladina) a P(porucha).
Pokud se na binárním vstupu (38) objeví Log.0, Program bude hlásit poruchu
pískáním peizo bzučáku a blikáním na displayi.Při poruše nelze sepnout žádný z výstupů.
Poruchu lze vypnout pouze tím,že na binární vstup (38) opět přivedeme Log.1.

Na čtvrtém řádku jsou zobrazeny výstupy a aktuální den+čas.Obě lze nastavit.
Pokud je u kteréhokoli výstupu plný obdelník,znamené že je sepnutý,pokud je u něj
prázdný obdelník,znamená že je vypnutý.Výstupy lze nastavit výběrem v menu a
skokem do fukce.
Taktéž nastavení času lze nastavit výběrem v menu a skokem do funkce.
Po nastavení času je nutno potvrdit nastavení tlačítkem SELECT.
*/

#include <Wire.h>
#include <Keypad.h> 
#include <LiquidCrystal_I2C.h>
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


// Funkce s prevody pro obvod realneho casu
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}


byte ctverec1[8] = {

  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
};

byte ctverec2[8] = {

  B00000,
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000,
};


byte sipka [8] = {

  B00000,
  B00100,
  B11110,
  B11111,
  B11110,
  B00100,
  B00000,
  B00000
};

byte objekt [8] = {    // ve funkci nastavcas problikava s nastavovanym cislem

  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

const byte ROWS = 5; // 5 řádků
const byte COLS = 4; // 4 sloupce

// zde si napíšete jak Vaše
// membránová klávesnice vypadá (některé znaky nahrazeny)
char hexaKeys[ROWS][COLS] = {
  {'A','B','#','*'},
  {'1','2','3','^'},
  {'4','5','6','v'},
  {'7','8','9','C'},
  {'<','0','>','E'}
};
    
byte rowPins[ROWS] = {46, A0, A1, A2, A3}; //čísla pinů s řadkem 1 2 3 4 5
byte colPins[COLS] = {A7, A6, A5, A4}; //čísla pinu se sloupcem 1 2 3 4
    
//inicializuje objekt klávesnice s názvem customKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
    
//globalni promenne
int limitdisplaye = 30;
int x;
int y;
int a;
int poc;
int poc2;
int AN1;
int AN2;
int out1;
int out2;
int out3;
int out4;
bool nastavvystupy1=false;
bool nastavvystupy2=false;
bool nacitaniVstupu=true;
long cas01;
long cas02;
long cas03;
long cas04;
long cas05;
long cas06;
long cas07;
long cas08;
long zac;
long cekej;
long cekejPointer;
int Binarnivstup1;
int analogvstup1 = A14;
int analogvstup2 = A12;
//int plus1 = A8;
int plus2 = A11;
int minus1 = A15;
int minus2 = A13;
int nastvlhkosti = 50;
int nasthladiny = 50;

int hodiny1 = 0;
int hodiny11 = 0;
int hodiny1celk;
int minuty1 = 0;
int minuty11 = 0;
int minuty1celk;

int hodiny2 = 0;
int hodiny22 = 0;
int hodiny2celk;
int minuty2 = 0;
int minuty22 = 0;
int minuty2celk;

int hodiny3 = 0;
int hodiny33 = 0;
int hodiny3celk;
int minuty3 = 0;
int minuty33 = 0;
int minuty3celk;

int hodiny4 = 0;
int hodiny44 = 0;
int hodiny4celk;
int minuty4 = 0;
int minuty44 = 0;
int minuty4celk;

int hodiny5 = 0;
int hodiny55 = 0;
int hodiny5celk;
int minuty5 = 0;
int minuty55 = 0;
int minuty5celk;

int hodiny6 = 0;
int hodiny66 = 0;
int hodiny6celk;
int minuty6 = 0;
int minuty66 = 0;
int minuty6celk;

int hodiny7 = 0;
int hodiny77 = 0;
int hodiny7celk;
int minuty7 = 0;
int minuty77 = 0;
int minuty7celk;

int min1 = 1;
int min2 = 1;
int min3 = 1;
int min4 = 1;
int min5 = 1;
int min6 = 1;
int min7 = 1;
int min11 = 2;
int min22 = 2;
int min33 = 2;
int min44 = 2;
int min55 = 2;
int min66 = 2;
int min77 = 2;
int prom1 = 0;
int prom2 = 0;
int prom3 = 0;
int prom4 = 0;
int prom5 = 0;
int prom6 = 0;
int prom7 = 0;
int rele1 = 47;
int rele2 = 49;
int rele3 = 51;
int rele4 = 53;
int nastavcasden;
int nastavcashod;
int nastavcasmin;
int nastavcasden2;
int nastavcashod2;
int nastavcasmin2;
int nastavcashodcelk;
int nastavcasmincelk;
//Napajeni Binarnich vstupu
int NapajeniBV1 = A9;
//binarni vstupy na desce (BV1/2)
int BV1 = A8;
// Piezo bzučák
int plus3 = 53;
int signal = 2;

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void setup() {
  lcd.begin(20, 4);
  lcd.backlight();
  Wire.begin();

  //setDS3231time(20,32,12,6,20,4,2015);
  //nastavení O.R.Času... při deklaraci smazat poznámku (//)

  //Binarni vstupy
  pinMode(NapajeniBV1, OUTPUT);
  digitalWrite(NapajeniBV1, HIGH);
  pinMode(BV1, INPUT);

  pinMode(analogvstup1, INPUT);
  pinMode(analogvstup2, INPUT);

  /*pinMode(plus1, OUTPUT);
  digitalWrite(plus1, HIGH);*/
  pinMode(minus1, OUTPUT);

  /*pinMode(plus2, OUTPUT);
  digitalWrite(plus2, HIGH);*/
  pinMode(minus2, OUTPUT);

  //Vytvoření znaků
  lcd.createChar(1, ctverec1);
  lcd.createChar(2, ctverec2);
  lcd.createChar(3, sipka);
  lcd.createChar(4, objekt);

  //PiezoBzučák
  pinMode(plus3, OUTPUT);
  digitalWrite(plus2, HIGH);
  pinMode(signal, OUTPUT);

  //Relátka
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(rele3, OUTPUT);
  pinMode(rele4, OUTPUT);
  digitalWrite(rele4, LOW);
  
  digitalWrite(minus2, HIGH);
  digitalWrite(minus1, HIGH);
  //načtení vstupů po startu
  AN1 = analogRead(analogvstup1);
  AN1 = map(AN1, 0, 1023, 0, 100);

  AN2 = analogRead(analogvstup2);
  AN2 = map(AN2, 0, 1023, 0, 100);
  digitalWrite(minus2, LOW);
  digitalWrite(minus1, LOW);

}

//------------------------------------------------------------------------------------------------------------------------------

void loop() {
  
  if ((millis() > cekejPointer + 1) && (millis() < cekejPointer + 1500)) {
  lcd.setCursor(x, y);
  lcd.write(byte(3));
  delay(20);    
  }

  if ((millis() > cekejPointer + 1500) && (millis() < cekejPointer + 1600)) {
  lcd.setCursor(x, y);
  lcd.write(" ");
  delay(20); 
  }
 
  if (millis() > cekejPointer + 1600) cekejPointer = millis();
 
  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  Binarnivstup1 = digitalRead(BV1);

  lcd.setCursor(1, 2);
  lcd.print("MV=");
  lcd.print(AN2);
  lcd.print("%");
  if (AN2 < 10) {
    lcd.setCursor(6, 2);
    lcd.print(" ");
  }
  lcd.setCursor(7, 2);
  if (AN2 < nastvlhkosti) {
    lcd.write(byte(1));
  } else {
    lcd.write(byte(2));
  }
  lcd.setCursor(9, 2);
  lcd.print("MH=");
  lcd.print(AN1);
  lcd.print("%");
  if (AN1 < 10) {
    lcd.setCursor(14, 2);
    lcd.print(" ");
  }
  lcd.setCursor(15, 2);
  if (AN1 < nasthladiny) {
    lcd.write(byte(2));
  } else {
    lcd.write(byte(1));
  }
  lcd.setCursor(1, 1);
  lcd.print("NV=");
  lcd.print(nastvlhkosti);
  lcd.print("%");
  lcd.setCursor(9, 1);
  lcd.print("NH=");
  lcd.print(nasthladiny);
  lcd.print("%");

  lcd.setCursor(1, 0);
  lcd.write("Po");
  lcd.setCursor(4, 0);
  lcd.write("Ut");
  lcd.setCursor(7, 0);
  lcd.write("St");
  lcd.setCursor(10, 0);
  lcd.write("Ct");
  lcd.setCursor(13, 0);
  lcd.write("Pa");
  lcd.setCursor(16, 0);
  lcd.write("So");
  lcd.setCursor(19, 0);
  lcd.write("N");

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  lcd.setCursor(12, 3);
  switch (dayOfWeek) {
    case 1:
      lcd.print("Po,");
      break;
    case 2:
      lcd.print("Ut,");
      break;
    case 3:
      lcd.print("St,");
      break;
    case 4:
      lcd.print("Ct,");
      break;
    case 5:
      lcd.print("Pa,");
      break;
    case 6:
      lcd.print("So,");
      break;
    case 7:
      lcd.print("Ne,");
      break;
  }

  if (Binarnivstup1 == LOW) {                    
    prom1 = 0;
    prom2 = 0;
    prom3 = 0;
    prom4 = 0;
    prom5 = 0;
    prom6 = 0;
    prom7 = 0;
    nastavvystupy1=false;
    nastavvystupy2=false;
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, LOW);
    digitalWrite(rele3, LOW);
    digitalWrite(rele4, HIGH);

    for (poc = 0; poc < 8; poc++) {
      digitalWrite(signal, HIGH);
      delay(5);
      digitalWrite(signal, LOW);
      delay(5);
    }
  }

  if (Binarnivstup1 == HIGH) {
    digitalWrite(rele4, LOW);
    poc2 = 0;
    lcd.setCursor(17, 2);
    lcd.write("P=");
    lcd.setCursor(19, 2);
    lcd.write(byte(2));
  }
  if (Binarnivstup1 == LOW) {
    poc2++;
    if ((poc2 > 3) && (poc2 <= 6)) {
      lcd.setCursor(17, 2);
      lcd.write("P=");
      lcd.setCursor(19, 2);
      lcd.write(byte(1));
      if (poc2 == 6) poc2 = 0;
    }
    if ((poc2 >= 0) && (poc2 <= 3)) {
      lcd.setCursor(17, 2);
      lcd.write("   ");
    }
  }

  lcd.setCursor(15, 3);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu
  lcd.setCursor(18, 3);
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);
  
  if(((prom1 == 1) || (prom2 == 1) || (prom3 == 1) || (prom4 == 1) || (prom5 == 1) || (prom6 == 1) || (prom7 == 1)) || ((prom1 == 2) || (prom2 == 2) || (prom3 == 2) || (prom4 == 2) || (prom5 == 2) || (prom6 == 2) || (prom7 == 2))){
  nastavvystupy1=false;
  nastavvystupy2=false;
  }
 
  lcd.setCursor(1, 3);
  lcd.print("OUT:");
  lcd.print("1");
  if (((nastavvystupy1 == true) || (nastavvystupy2 == true)) || ((prom1 == 1) || (prom2 == 1) || (prom3 == 1) || (prom4 == 1) || (prom5 == 1) || (prom6 == 1) || (prom7 == 1)) || ((prom1 == 2) || (prom2 == 2) || (prom3 == 2) || (prom4 == 2) || (prom5 == 2) || (prom6 == 2) || (prom7 == 2))) {
    lcd.write(byte(1));
    digitalWrite(rele1, HIGH);
    
    digitalWrite(minus2, HIGH);
    digitalWrite(minus1, HIGH);
    
    //načtení vstupů po startu
    AN1 = analogRead(analogvstup1);
    AN1 = map(AN1, 0, 1023, 0, 100);

    AN2 = analogRead(analogvstup2);
    AN2 = map(AN2, 0, 1023, 0, 100);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele1, LOW);
  }
  lcd.print("2");
  if ((prom1 == 1) || (prom2 == 1) || (prom3 == 1) || (prom4 == 1) || (prom5 == 1) || (prom6 == 1) || (prom7 == 1) || (nastavvystupy1 == true)) {
    lcd.write(byte(1));
    digitalWrite(rele2, HIGH);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele2, LOW);
  }
  lcd.print("3");
  if ((prom1 == 2) || (prom2 == 2) || (prom3 == 2) || (prom4 == 2) || (prom5 == 2) || (prom6 == 2) || (prom7 == 2) || (nastavvystupy2 == true)) {
    lcd.write(byte(1));
    digitalWrite(rele3, HIGH);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele3, LOW);
  }

  if ((millis() > cekej + 1) && (millis() < cekej + 1000)) {
    lcd.setCursor(17, 3);
    lcd.print(":");                   //blikani dvojtecky v casu ve vterinovem intervalu
  }


  if ((millis() > cekej + 1000) && (millis() < cekej + 2000)) {
    lcd.setCursor(17, 3);
    lcd.print(" ");                   //blikani dvojtecky v casu ve vterinovem intervalu

  }
  if (millis() > cekej + 2000) cekej = millis();

  if ((AN2 > nastvlhkosti) || (AN1 < nasthladiny)) {
    prom1 = 0;
    prom2 = 0;
    prom3 = 0;
    prom4 = 0;
    prom5 = 0;
    prom6 = 0;
    prom7 = 0;
    nastavvystupy1=false;
    nastavvystupy2=false;
  }
  
  if (((hour == hodiny1celk) && (minute == (minuty1celk-1)))||((hour == hodiny2celk) && (minute == (minuty2celk-1)))||((hour == hodiny3celk) && (minute == (minuty3celk-1)))||((hour == hodiny4celk) && (minute == (minuty4celk-1)))||((hour == hodiny5celk) && (minute == (minuty5celk-1)))||((hour == hodiny6celk) && (minute == (minuty6celk-1)))||((hour == hodiny7celk) && (minute == (minuty7celk-1)))) {
   
    //nacteni vstupu minutu pred spuštěním výstupů
    if(nacitaniVstupu==true){
    digitalWrite(minus2, HIGH);
    digitalWrite(minus1, HIGH);
    AN1 = analogRead(analogvstup1);
    AN1 = map(AN1, 0, 1023, 0, 100);

    AN2 = analogRead(analogvstup2);
    AN2 = map(AN2, 0, 1023, 0, 100);
    nacitaniVstupu=false;
    digitalWrite(minus2, LOW);
    digitalWrite(minus1, LOW);
    }  
  }
  
  if ((hour == hodiny1celk) && (minute == minuty1celk) && (prom1 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 1) && ((min1 > 0) || (min11 > 0))) {
    prom1 = 1;
    nacitaniVstupu=true;
    if ((min11 > 0) && (min1 == 0)) prom1 = 2;
    cas01 = millis() / 1000;                                                            
  }

  if ((hour == hodiny2celk) && (minute == minuty2celk) && (prom2 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 2) && ((min2 > 0) || (min22 > 0))) {
    prom2 = 1;
    nacitaniVstupu=true;
    if ((min22 > 0) && (min2 == 0)) prom2 = 2;
    cas02 = millis() / 1000;
  }

  if ((hour == hodiny3celk) && (minute == minuty3celk) && (prom3 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 3) && ((min3 > 0) || (min33 > 0))) {
    prom3 = 1;
    nacitaniVstupu=true;
    if ((min33 > 0) && (min3 == 0)) prom3 = 2;
    cas03 = millis() / 1000;
  }

  if ((hour == hodiny4celk) && (minute == minuty4celk) && (prom4 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 4) && ((min4 > 0) || (min44 > 0))) {
    prom4 = 1;
    nacitaniVstupu=true;
    if ((min44 > 0) && (min4 == 0)) prom4 = 2;
    cas04 = millis() / 1000;                                                           
  }

  if ((hour == hodiny5celk) && (minute == minuty5celk) && (prom5 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 5) && ((min5 > 0) || (min55 > 0))) {
    prom5 = 1;
    nacitaniVstupu=true;
    if ((min55 > 0) && (min5 == 0)) prom5 = 2;
    cas05 = millis() / 1000;
  }

  if ((hour == hodiny6celk) && (minute == minuty6celk) && (prom6 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 6) && ((min6 > 0) || (min66 > 0))) {
    prom6 = 1;
    nacitaniVstupu=true;
    if ((min66 > 0) && (min6 == 0)) prom6 = 2;
    cas06 = millis() / 1000;
  }

  if ((hour == hodiny7celk) && (minute == minuty7celk) && (prom7 == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (dayOfWeek == 7) && ((min7 > 0) || (min77 > 0))) {
    prom7 = 1;
    nacitaniVstupu=true;
    if ((min77 > 0) && (min7 == 0)) prom7 = 2;
    cas07 = millis() / 1000;
  }

  if (prom1 == 1) {
    if ((millis() / 1000) > (cas01 + (min1 * 60))) {
      cas01 = millis() / 1000;
      prom1 = 2;
    }
  }
  if (prom2 == 1) {
    if ((millis() / 1000) > (cas02 + (min2 * 60))) {
      cas01 = millis() / 1000;
      prom2 = 2;
    }
  }
  if (prom3 == 1) {
    if ((millis() / 1000) > (cas03 + (min3 * 60))) {
      cas03 = millis() / 1000;
      prom3 = 2;
    }
  }
  if (prom4 == 1) {
    if ((millis() / 1000) > (cas04 + (min4 * 60))) {
      cas04 = millis() / 1000;
      prom4 = 2;
    }
  }
  if (prom5 == 1) {
    if ((millis() / 1000) > (cas05 + (min5 * 60))) {
      cas05 = millis() / 1000;
      prom5 = 2;
    }
  }
  if (prom6 == 1) {
    if ((millis() / 1000) > (cas06 + (min6 * 60))) {
      cas06 = millis() / 1000;
      prom6 = 2;
    }
  }
  if (prom7 == 1) {
    if ((millis() / 1000) > (cas07 + (min7 * 60))) {
      cas07 = millis() / 1000;
      prom7 = 2;
    }
  }

  if (prom1 == 2) {
    if (min11 > 0) {
      if ((millis() / 1000) > (cas01 + (min11 * 60))) prom1 = 0;
    }
    else prom1 = 0;
  }
  if (prom2 == 2) {
    if (min22 > 0) {
      if ((millis() / 1000) > (cas02 + (min22 * 60))) prom2 = 0;
    }
    else prom2 = 0;
  }
  if (prom3 == 2) {
    if (min33 > 0) {
      if ((millis() / 1000) > (cas03 + (min33 * 60))) prom3 = 0;
    }
    else prom3 = 0;
  }
  if (prom4 == 2) {
    if (min44 > 0) {
      if ((millis() / 1000) > (cas04 + (min44 * 60))) prom4 = 0;
    }
    else prom4 = 0;
  }
  if (prom5 == 2) {
    if (min55 > 0) {
      if ((millis() / 1000) > (cas05 + (min55 * 60))) prom5 = 0;
    }
    else prom5 = 0;
  }
  if (prom6 == 2) {
    if (min66 > 0) {
      if ((millis() / 1000) > (cas06 + (min66 * 60))) prom6 = 0;
    }
    else prom6 = 0;
  }
  if (prom7 == 2) {
    if (min77 > 0) {
      if ((millis() / 1000) > (cas07 + (min77 * 60))) prom7 = 0;
    }
    else prom7 = 0;
  }
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
      
    cas08 = millis() / 1000;
    lcd.backlight();
    
    digitalWrite(minus2, HIGH);
    digitalWrite(minus1, HIGH);
    
    //nacteni vstupu po znáčknutí jakéhokoli tlačítka
    AN1 = analogRead(analogvstup1);
    AN1 = map(AN1, 0, 1023, 0, 100);

    AN2 = analogRead(analogvstup2);
    AN2 = map(AN2, 0, 1023, 0, 100);
    
    digitalWrite(minus2, LOW);
    digitalWrite(minus1, LOW);                                                                                                             
    
    if (customKey == '^')
    { 
        cas08 = millis() / 1000;
        lcd.backlight();
        if (y == 1) {
          y = 0;
          x = 0;
          delay(20);
          lcd.setCursor(0, 1);
          lcd.write(" ");
        }
        if ((y == 3) && (x == 11)) {
          lcd.setCursor(11, 3);
          lcd.write(" ");
          y = 1;
          x = 0;
          delay(20);
        }
        if ((y == 3) && (x == 0)) {
          lcd.setCursor(0, 3);
          lcd.write(" ");
          y = 1;
          x = 0;
          delay(20);
        } 
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
        lcd.backlight();
        if ((y == 1) && (x == 0)) {
          lcd.setCursor(0, 1);
          lcd.write(" ");
          x = 0;
          y = 3;
          delay(20);
        }
        /* if((y==1)&&(x==0)){
           lcd.setCursor(0, 1);
           lcd.write(" ");
           x=11;
           y=3;
           delay(20);
         }*/
        if (y == 0) {
          y = 1;
          x = 0;
          delay(20);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          lcd.setCursor(3, 0);
          lcd.write(" ");
          lcd.setCursor(6, 0);
          lcd.write(" ");
          lcd.setCursor(9, 0);
          lcd.write(" ");
          lcd.setCursor(12, 0);
          lcd.write(" ");
          lcd.setCursor(15, 0);
          lcd.write(" ");
          lcd.setCursor(18, 0);
          lcd.write(" ");
        }
    }
    if (customKey == '>')
    {
      cas08 = millis() / 1000;
        lcd.backlight();
        if ((x == 15) && (y == 0)) {
          lcd.setCursor(15, 0);
          lcd.print(" ");
          x = 18;
          delay(50);
        }
        if ((x == 12) && (y == 0)) {
          lcd.setCursor(12, 0);
          lcd.print(" ");
          x = 15;
          delay(50);
        }
        if ((x == 9) && (y == 0)) {
          lcd.setCursor(9, 0);
          lcd.print(" ");
          x = 12;
          delay(50);
        }
        if ((x == 6) && (y == 0)) {
          lcd.setCursor(6, 0);
          lcd.print(" ");
          x = 9;
          delay(50);
        }
        if ((x == 3) && (y == 0)) {
          lcd.setCursor(3, 0);
          lcd.print(" ");
          x = 6;
          delay(50);
        }
        if ((x == 0) && (y == 0)) {
          lcd.setCursor(0, 0);
          lcd.print(" ");
          x = 3;
          delay(50);
        }
        if ((x == 0) && (y == 3)) {
          lcd.setCursor(0, 3);
          lcd.print(" ");
          x = 11;
          delay(50);
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();
        if (x == 3) {
          lcd.setCursor(3, 0);
          lcd.print(" ");
          x = 0;
          delay(50);
        }
        if (x == 6) {
          lcd.setCursor(6, 0);
          lcd.print(" ");
          x = 3;
          delay(50);
        }
        if (x == 9) {
          lcd.setCursor(9, 0);
          lcd.print(" ");
          x = 6;
          delay(50);
        }
        if (x == 12) {
          lcd.setCursor(12, 0);
          lcd.print(" ");
          x = 9;
          delay(50);
        }
        if (x == 15) {
          lcd.setCursor(15, 0);
          lcd.print(" ");
          x = 12;
          delay(50);
        }
        if (x == 18) {
          lcd.setCursor(18, 0);
          lcd.print(" ");
          x = 15;
          delay(50);
        }
        if ((x == 11) && (y == 3)) {
          lcd.setCursor(11, 3);
          lcd.print(" ");
          x = 0;
          delay(50);
        }
    }
    if (customKey == 'E')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if ((y == 3) && (x == 11)) {
          lcd.clear();
          nastavcas();
        }
        if ((y == 0) && (x == 0)) {
          lcd.clear();
          cas1();
        }
        if ((y == 0) && (x == 3)) {                            // vyber v menu a skoky do fci
          lcd.clear();
          cas2();
        }
        if ((y == 0) && (x == 6)) {
          lcd.clear();
          cas3();
        }
        if ((y == 0) && (x == 9)) {
          lcd.clear();
          cas4();
        }
        if ((y == 0) && (x == 12)) {
          lcd.clear();
          cas5();
        }
        if ((y == 0) && (x == 15)) {
          lcd.clear();
          cas6();
        }
        if ((y == 0) && (x == 18)) {
          lcd.clear();
          cas7();
        }
        if ((y == 1) && (x == 0)) {
          lcd.clear();
          nastvlhkosti1();
        }
        if ((y == 3) && (x == 0)) {
          lcd.clear();
          nastavvystupy();
        }
  }
 }
}

//------------------------------------------------------------------------------------------------------------------------------

// FUNKCE
int nastvlhkosti1() {

  lcd.setCursor(1, 0);
  lcd.write("NAST. VLHKOSTI=");
  if (nastvlhkosti >= 10) {
    lcd.setCursor(17, 0);
    lcd.print(nastvlhkosti);
  }
  else {
    lcd.setCursor(17, 0);
    lcd.write(" ");
    lcd.setCursor(18, 0);
    lcd.print(nastvlhkosti);
  }
  lcd.setCursor(19, 0);
  lcd.print("%");
  lcd.setCursor(1, 1);
  lcd.write("NAST. HLADINY =");
  if (nasthladiny < 10) {
    lcd.setCursor(18, 1);
    lcd.print(nasthladiny);
    lcd.print("%");
  }
  else {
    lcd.setCursor(17, 1);
    lcd.print(nasthladiny);
    lcd.print("%");
  }
  lcd.setCursor(1, 2);
  lcd.write("LIMIT DISPLAYE=");
  if (limitdisplaye < 10) {
    lcd.setCursor(17, 2);
    lcd.print(" ");
    lcd.setCursor(18, 2);
    lcd.print(limitdisplaye);
    lcd.print("s");
  }
  else {
    lcd.setCursor(17, 2);
    lcd.print(limitdisplaye);
    lcd.print("s");
  }

  if (a == 0) {
    lcd.setCursor(0, 0);
    lcd.write(byte(3));
    delay(10);
  }
  if (a == 1) {
    lcd.setCursor(0, 1);
    lcd.write(byte(3));
    delay(10);
  }
  if (a == 2) {
    lcd.setCursor(0, 2);
    lcd.write(byte(3));
    delay(10);
  }
  delay(400);
zacatek:

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  if (a == 0) {
    lcd.setCursor(0, 0);
    lcd.write(byte(3));
    delay(100);
  }
  if (a == 1) {
    lcd.setCursor(0, 1);
    lcd.write(byte(3));
    delay(100);
  }
  if (a == 2) {
    lcd.setCursor(0, 2);
    lcd.write(byte(3));
    delay(10);
  }
  lcd.setCursor(1, 0);
  lcd.write("NAST. VLHKOSTI=");
  if (nastvlhkosti >= 10) {
    lcd.setCursor(17, 0);
    lcd.print(nastvlhkosti);
  }
  else {
    lcd.setCursor(17, 0);
    lcd.write(" ");
    lcd.setCursor(18, 0);
    lcd.print(nastvlhkosti);
  }
  lcd.setCursor(19, 0);
  lcd.print("%");
  lcd.setCursor(1, 1);
  lcd.write("NAST. HLADINY =");
  if (nasthladiny < 10) {
    lcd.setCursor(18, 1);
    lcd.print(nasthladiny);
    lcd.print("%");
  }
  else {
    lcd.setCursor(17, 1);
    lcd.print(nasthladiny);
    lcd.print("%");
  }
  
  lcd.setCursor(1, 2);
  lcd.write("LIMIT DISPLAYE=");
  if (limitdisplaye < 10) {
    lcd.setCursor(17, 2);
    lcd.print(" ");
    lcd.setCursor(18, 2);
    lcd.print(limitdisplaye);
    lcd.print("s");
  }
  else {
    lcd.setCursor(17, 2);
    lcd.print(limitdisplaye);
    lcd.print("s");
  }


  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == 'C')    //konec (ESC)
    {
      lcd.backlight();
      cas08 = millis() / 1000;
      goto konec;
    }
    if (customKey == '^')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti <= 98) {
            nastvlhkosti = nastvlhkosti + 1;
            delay(50);
          }
        }
        if (a == 1) {
          if (nasthladiny <= 98) {
            nasthladiny = nasthladiny + 1;
            delay(50);
          }
        }
        if (a == 2) {
          if (limitdisplaye <= 98) {
            limitdisplaye = limitdisplaye + 1;
            delay(50);
          }
        }
    }
    if (customKey == 'v')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti >= 2) {
            nastvlhkosti = nastvlhkosti - 1;
            delay(50);
          }
        }
        if (a == 1) {
          if (nasthladiny >= 2) {
            nasthladiny = nasthladiny - 1;
            delay(50);
          }
        }
        if (a == 2) {
          if (limitdisplaye >= 2) {
            limitdisplaye = limitdisplaye - 1;
            delay(50);
          }
        }
    }
    if (customKey == '<')
    {
      lcd.backlight();
      cas08 = millis() / 1000;
    }
    if (customKey == '>')
    {
      lcd.backlight();
      cas08 = millis() / 1000;
    }
    if (customKey == 'E')
    {
      lcd.backlight();
        cas08 = millis() / 1000;
        if (a == 0) {
          a = 1;
          delay(250);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          goto zacatek;
        }
        if (a == 1) {
          a = 2;
          delay(250);
          lcd.setCursor(0, 1);
          lcd.write(" ");
          goto zacatek;
        }
        if (a == 2) {
          a = 0;
          goto konec;
        }
      }
    }
  goto zacatek;
konec:
  lcd.clear();
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas1() {                //funkce cas 1
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min1 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min1);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min1);
  }
  if (min11 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min11);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min11);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny11);
  lcd.setCursor(3,0);
  lcd.print(minuty1);
  lcd.setCursor(4,0);
  lcd.print(minuty11);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny1);
  lcd.setCursor(3,0);
  lcd.print(minuty1);
  lcd.setCursor(4,0);
  lcd.print(minuty11);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny1);
  lcd.setCursor(1,0);
  lcd.print(hodiny11);
  lcd.setCursor(4,0);
  lcd.print(minuty11);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny1);
  lcd.setCursor(1,0);
  lcd.print(hodiny11);
  lcd.setCursor(3,0);
  lcd.print(minuty1);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny1);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny11);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty1);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty11);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min1 <= 98) {
            min1 = min1  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min11 <= 98) {
            min11 = min11 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min1 >= 1) {
            min1 = min1 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min11 >= 1) {
            min11 = min11 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny1=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny11=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty1=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny1=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny11=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty1=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny11<=3){
        if (promenna == 1) {
          hodiny1=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny11=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty1=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny11=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty1=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty1=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty1=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty11=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty11=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty11=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty11=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny1 <=1){
          hodiny11=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty11=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty11);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny1celk=(hodiny1*10)+ hodiny11;
  
        minuty1celk=(minuty1*10)+ minuty11;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas2() {                //funkce cas 2
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min2 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min2);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min2);
  }
  if (min22 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min22);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min22);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny22);
  lcd.setCursor(3,0);
  lcd.print(minuty2);
  lcd.setCursor(4,0);
  lcd.print(minuty22);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny2);
  lcd.setCursor(3,0);
  lcd.print(minuty2);
  lcd.setCursor(4,0);
  lcd.print(minuty22);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny2);
  lcd.setCursor(1,0);
  lcd.print(hodiny22);
  lcd.setCursor(4,0);
  lcd.print(minuty22);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny2);
  lcd.setCursor(1,0);
  lcd.print(hodiny22);
  lcd.setCursor(3,0);
  lcd.print(minuty2 );
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny2);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny22);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty2);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty22);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min2 <= 98) {
            min2 = min2  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min22 <= 98) {
            min22 = min22 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min2 >= 1) {
            min2 = min2 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min22 >= 1) {
            min22 = min22 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny2=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny22=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty2=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny2=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny22=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty2=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny22<=3){
        if (promenna == 1) {
          hodiny2=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny22=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty2=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny22=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty2=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty2=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty2=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty22=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty22=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty22=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty22=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny2 <=1){
          hodiny22=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty22=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty22);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny2celk=(hodiny2*10)+ hodiny22;
  
        minuty2celk=(minuty2*10)+ minuty22;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas3() {                //funkce cas 3
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min3 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min3);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min3);
  }
  if (min33 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min33);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min33);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny33);
  lcd.setCursor(3,0);
  lcd.print(minuty3);
  lcd.setCursor(4,0);
  lcd.print(minuty33);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny3);
  lcd.setCursor(3,0);
  lcd.print(minuty3);
  lcd.setCursor(4,0);
  lcd.print(minuty33);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny3);
  lcd.setCursor(1,0);
  lcd.print(hodiny33);
  lcd.setCursor(4,0);
  lcd.print(minuty33);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny3);
  lcd.setCursor(1,0);
  lcd.print(hodiny33);
  lcd.setCursor(3,0);
  lcd.print(minuty3);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny3);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny33);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty3);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty33);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min3 <= 98) {
            min3 = min3  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min33 <= 98) {
            min33 = min33 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min3 >= 1) {
            min3 = min3 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min33 >= 1) {
            min33 = min33 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny3=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny33=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty3=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny3=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny33=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty3=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny33<=3){
        if (promenna == 1) {
          hodiny3=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny33=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty3=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny33=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty3=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty3=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty3=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty33=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty33=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty33=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty33=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny3 <=1){
          hodiny33=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty33=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty33);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny3celk=(hodiny3*10)+ hodiny33;
  
        minuty3celk=(minuty3*10)+ minuty33;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas4() {                //funkce cas 4
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min4 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min4);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min4);
  }
  if (min44 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min44);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min44);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny44);
  lcd.setCursor(3,0);
  lcd.print(minuty4);
  lcd.setCursor(4,0);
  lcd.print(minuty44);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny4);
  lcd.setCursor(3,0);
  lcd.print(minuty4);
  lcd.setCursor(4,0);
  lcd.print(minuty44);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny4);
  lcd.setCursor(1,0);
  lcd.print(hodiny44);
  lcd.setCursor(4,0);
  lcd.print(minuty44);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny4);
  lcd.setCursor(1,0);
  lcd.print(hodiny44);
  lcd.setCursor(3,0);
  lcd.print(minuty4);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny4);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny44);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty4);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty44);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min4 <= 98) {
            min4 = min4  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min44 <= 98) {
            min44 = min44 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min4 >= 1) {
            min4 = min4 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min44 >= 1) {
            min44 = min44 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny4=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny44=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty4=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny4=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny44=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty4=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny44<=3){
        if (promenna == 1) {
          hodiny4=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny44=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty4=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny44=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty4=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty4=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty4=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty44=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty44=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty44=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty44=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny4 <=1){
          hodiny44=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty44=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty44);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny4celk=(hodiny4*10)+ hodiny44;
  
        minuty4celk=(minuty4*10)+ minuty44;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas5() {                //funkce cas 5
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min5 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min5);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min5);
  }
  if (min55 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min55);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min55);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny55);
  lcd.setCursor(3,0);
  lcd.print(minuty5);
  lcd.setCursor(4,0);
  lcd.print(minuty55);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny5);
  lcd.setCursor(3,0);
  lcd.print(minuty5);
  lcd.setCursor(4,0);
  lcd.print(minuty55);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny5);
  lcd.setCursor(1,0);
  lcd.print(hodiny55);
  lcd.setCursor(4,0);
  lcd.print(minuty55);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny5);
  lcd.setCursor(1,0);
  lcd.print(hodiny55);
  lcd.setCursor(3,0);
  lcd.print(minuty5);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny5);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny55);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty5);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty55);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min5 <= 98) {
            min5 = min5  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min55 <= 98) {
            min55 = min55 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min5 >= 1) {
            min5 = min5 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min55 >= 1) {
            min55 = min55 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny5=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny55=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty5=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny5=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny55=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty5=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny55<=3){
        if (promenna == 1) {
          hodiny5=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny55=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty5=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny55=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty5=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty5=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty5=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty55=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty55=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty55=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty55=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny5 <=1){
          hodiny55=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty55=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty55);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny5celk=(hodiny5*10)+ hodiny55;
  
        minuty5celk=(minuty5*10)+ minuty55;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas6() {                //funkce cas 6
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min6 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min6);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min6);
  }
  if (min66 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min66);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min66);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny66);
  lcd.setCursor(3,0);
  lcd.print(minuty6);
  lcd.setCursor(4,0);
  lcd.print(minuty66);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny6);
  lcd.setCursor(3,0);
  lcd.print(minuty6);
  lcd.setCursor(4,0);
  lcd.print(minuty66);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny6);
  lcd.setCursor(1,0);
  lcd.print(hodiny66);
  lcd.setCursor(4,0);
  lcd.print(minuty66);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny6);
  lcd.setCursor(1,0);
  lcd.print(hodiny66);
  lcd.setCursor(3,0);
  lcd.print(minuty6);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny6);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny66);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty6);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty66);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min6 <= 98) {
            min6 = min6  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min66 <= 98) {
            min66 = min66 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min6 >= 1) {
            min6 = min6 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min66 >= 1) {
            min66 = min66 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny6=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny66=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty6=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny6=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny66=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty6=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny66<=3){
        if (promenna == 1) {
          hodiny6=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny66=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty6=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny66=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty6=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty6=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty6=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty66=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty66=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty66=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty66=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny6 <=1){
          hodiny66=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty66=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty66);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny6celk=(hodiny6*10)+ hodiny66;
  
        minuty6celk=(minuty6*10)+ minuty66;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int cas7() {                //funkce cas 7
  int promenna = 1;
navesti:

  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(8, 2);
  lcd.print("NAST.MINUT");
  if (min7 >= 10) {
    lcd.setCursor(6, 3);
    lcd.print(min7);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(" ");
    lcd.setCursor(7, 3);
    lcd.print(min7);
  }
  if (min77 >= 10) {
    lcd.setCursor(12, 3);
    lcd.print(min77);
  }
  else {
    lcd.setCursor(12, 3);
    lcd.print(" ");
    lcd.setCursor(13, 3);
    lcd.print(min77);
  }
  
  lcd.setCursor(2,0);
  lcd.print(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(hodiny77);
  lcd.setCursor(3,0);
  lcd.print(minuty7);
  lcd.setCursor(4,0);
  lcd.print(minuty77);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(hodiny7);
  lcd.setCursor(3,0);
  lcd.print(minuty7);
  lcd.setCursor(4,0);
  lcd.print(minuty77);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(hodiny7);
  lcd.setCursor(1,0);
  lcd.print(hodiny77);
  lcd.setCursor(4,0);
  lcd.print(minuty77);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(hodiny7);
  lcd.setCursor(1,0);
  lcd.print(hodiny77);
  lcd.setCursor(3,0);
  lcd.print(minuty7);
  } 
  
  if ((promenna == 5)||(promenna == 1)){
  lcd.setCursor(11, 3);
    lcd.write(" ");
  }
  if ((promenna == 6)||(promenna == 4)){
  lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }
  if (promenna == 6) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(11, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("< KONEC,UP +,DOWN - ");
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(hodiny7);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(hodiny77);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(minuty7);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(minuty77);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (min7 <= 98) {
            min7 = min7  + 1;
          }
          delay(200);
        } 
        if (promenna == 6) {
          if (min77 <= 98) {
            min77 = min77 + 1;
          }
          delay(200);
        }  
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (min7 >= 1) {
            min7 = min7 - 1;
          }
          delay(200);
        }
        if (promenna == 6) {
          if (min77 >= 1) {
            min77 = min77 - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny7=0;
          delay(200);
        }
        if (promenna == 2) {
          hodiny77=0;
          delay(200);
        } 
        if (promenna == 3) {
          minuty7=0;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          hodiny7=1;
          delay(200);
        }
        if (promenna == 2) {
          hodiny77=1;
          delay(200);
        } 
        if (promenna == 3) {
          minuty7=1;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(hodiny77<=3){
        if (promenna == 1) {
          hodiny7=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          hodiny77=2;
          delay(200);
        } 
        if (promenna == 3) {
          minuty7=2;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          hodiny77=3;
          delay(200);
        } 
        if (promenna == 3) {
          minuty7=3;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          minuty7=4;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          minuty7=5;
          delay(200);
        }
        if (promenna == 4) {
          minuty77=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty77=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          minuty77=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty77=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(hodiny7 <=1){
          hodiny77=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          minuty77=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 7) {
          promenna++;
          delay(200);
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(minuty77);
          }
          if (promenna == 7) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 6;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        
        hodiny7celk=(hodiny7*10)+ hodiny77;
  
        minuty7celk=(minuty7*10)+ minuty77;
        
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);        
        lcd.clear();
        promenna=1;
        goto navesti;
    }
   }

goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int nastavcas() {
  int promenna = 1;
  nastavcasden = 1;
  nastavcashod = 0;
  nastavcasmin = 0;
  cekej = millis();
navesti:
  // Nacteni hodin a minut
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 2);
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour, DEC);          //vypsani casu

  lcd.print(":");
  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  lcd.setCursor(9, 2);
  lcd.print("NAST.DNE");

  lcd.setCursor(6, 3);
  switch (nastavcasden) {
    case 1:
      lcd.print("Pondeli  ");
      break;
    case 2:
      lcd.print("Utery   ");
      break;
    case 3:
      lcd.print("Streda  ");
      break;
    case 4:
      lcd.print("Ctvrtek  ");
      break;
    case 5:
      lcd.print("Patek   ");
      break;
    case 6:
      lcd.print("Sobota  ");
      break;
    case 7:
      lcd.print("Nedele  ");
      break;
  }

  lcd.setCursor(2, 0);
  lcd.write(":");
  
  if (promenna == 1){
  lcd.setCursor(1,0);
  lcd.print(nastavcashod2);
  lcd.setCursor(3,0);
  lcd.print(nastavcasmin);
  lcd.setCursor(4,0);
  lcd.print(nastavcasmin2);
  } 
  if (promenna == 2){
  lcd.setCursor(0,0);
  lcd.print(nastavcashod);
  lcd.setCursor(3,0);
  lcd.print(nastavcasmin);
  lcd.setCursor(4,0);
  lcd.print(nastavcasmin2);
  } 
  if (promenna == 3){
  lcd.setCursor(0,0);
  lcd.print(nastavcashod);
  lcd.setCursor(1,0);
  lcd.print(nastavcashod2);
  lcd.setCursor(4,0);
  lcd.print(nastavcasmin2);
  } 
  if (promenna == 4){
  lcd.setCursor(0,0);
  lcd.print(nastavcashod);
  lcd.setCursor(1,0);
  lcd.print(nastavcashod2);
  lcd.setCursor(3,0);
  lcd.print(nastavcasmin);
  }   
  
  if ((promenna == 1)||(promenna == 2)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Hodiny");
    lcd.setCursor(5, 3);
    lcd.write(" ");
  }
  if ((promenna == 3)||(promenna == 4)) {
    lcd.setCursor(6, 0);
    lcd.write("Nast. Minuty");
  }
  if (promenna == 5) {
    lcd.setCursor(6, 0);
    lcd.write("             ");
    lcd.setCursor(5, 3);
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write("Konec= Esc,Uloz= Ent");
  
  nastavcashodcelk=(nastavcashod*10)+nastavcashod2;
  
  nastavcasmincelk=(nastavcasmin*10)+nastavcasmin2;
  
  if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.write(byte(4));
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.write(byte(4));
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.write(byte(4));
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.write(byte(4));
      break;
  }            
 }


  if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
    switch (promenna) {
    case 1:
          lcd.setCursor(0,0);
          lcd.print(nastavcashod);
      break;
    case 2:
          lcd.setCursor(1,0);
          lcd.print(nastavcashod2);
      break;
    case 3:
          lcd.setCursor(3,0);
          lcd.print(nastavcasmin);
      break;
    case 4:
          lcd.setCursor(4,0);
          lcd.print(nastavcasmin2);
      break;
  }   
 }
 
 if (millis() > cekej + 1000) cekej = millis();
  
  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (nastavcasden <= 6) {
            nastavcasden = nastavcasden + 1;
          }
          delay(200);
        }   
    }
    if (customKey == 'v')
    {
       cas08 = millis() / 1000;
       lcd.backlight();

        if (promenna == 5) {
          if (nastavcasden >= 2) {
            nastavcasden = nastavcasden - 1;
          }
          delay(200);
        }
    }
    
    if (customKey == '0')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          nastavcashod=0;
          delay(200);
        }
        if (promenna == 2) {
          nastavcashod2=0;
          delay(200);
        } 
        if (promenna == 3) {
          nastavcasmin=0;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=0;
          delay(200);
        }
    }
    if (customKey == '1')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 1) {
          nastavcashod=1;
          delay(200);
        }
        if (promenna == 2) {
          nastavcashod2=1;
          delay(200);
        } 
        if (promenna == 3) {
          nastavcasmin=1;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=1;
          delay(200);
        }
    }
    if (customKey == '2')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      if(nastavcashod2<=3){
        if (promenna == 1) {
          nastavcashod=2;
          delay(200);
        }
      }
        if (promenna == 2) {
          nastavcashod2=2;
          delay(200);
        } 
        if (promenna == 3) {
          nastavcasmin=2;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=2;
          delay(200);
        }
    }
    if (customKey == '3')
    {
      cas08 = millis() / 1000;
      lcd.backlight();

        if (promenna == 2) {
          nastavcashod2=3;
          delay(200);
        } 
        if (promenna == 3) {
          nastavcasmin=3;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=3;
          delay(200);
        }
    }
    if (customKey == '4')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=4;
          delay(200);
          }
        } 
        if (promenna == 3) {
          nastavcasmin=4;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=4;
          delay(200);
        }
    }
    if (customKey == '5')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=5;
          delay(200);
          }
        } 
        
        if (promenna == 3) {
          nastavcasmin=5;
          delay(200);
        }
        if (promenna == 4) {
          nastavcasmin2=5;
          delay(200);
        }
    }
    if (customKey == '6')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=6;
          delay(200);
          }
        } 

        if (promenna == 4) {
          nastavcasmin2=6;
          delay(200);
        }
    }
    if (customKey == '7')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
        
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=7;
          delay(200);
          }
        } 
        
        if (promenna == 4) {
          nastavcasmin2=7;
          delay(200);
        }
    }
    if (customKey == '8')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=8;
          delay(200);
          }
        } 

        if (promenna == 4) {
          nastavcasmin2=8;
          delay(200);
        }
    }
    if (customKey == '9')
    {
      cas08 = millis() / 1000;
      lcd.backlight();
      
        if (promenna == 2) {
          if(nastavcashod <=1){
          nastavcashod2=9;
          delay(200);
          }
        } 

        if (promenna == 4) {
          nastavcasmin2=9;
          delay(200);
        }
    }
    
    if (customKey == '>')
    {
        cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna < 6) {
          promenna++;
          if(promenna==5){
            lcd.setCursor(4,0);
            lcd.print(nastavcasmin2);
          }
          delay(200);
          if (promenna == 6) {
            promenna = 1;
          }
        }
    }
    if (customKey == '<')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 4) {
            lcd.setCursor(5,3);
            lcd.print(" ");
          }
          if (promenna == 0) {
            promenna = 5;
          }
        }
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;

        goto ukonceni;
    }
    if (customKey == 'E')
    {
      cas08 = millis() / 1000;
        lcd.backlight();

        if ((nastavcashod > 0) || (nastavcasmin > 0)) {
          setDS3231time(00, nastavcasmincelk, nastavcashodcelk, nastavcasden, 12, 5, 2015);
          lcd.clear();
          lcd.setCursor(6, 1);
          lcd.print("*ULOZENO*");
          delay(1000);
          lcd.setCursor(6, 1);
          lcd.print("         ");
          promenna=1;
          goto navesti;
        }
    }
   }
   

  goto navesti;
ukonceni:
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------

int nastavvystupy() {

  int promenna = 1;
  x = 0;
  y = 2;

navesti:
  promenna = y;
  lcd.setCursor(x, y);
  lcd.write(byte(3));
  delay(20);

  if ((millis() / 1000) > (cas08 + (1 * limitdisplaye))) lcd.noBacklight();

  lcd.setCursor(0, 0);
  lcd.print("*NASTAVENI  VYSTUPU*");

  lcd.setCursor(1, 1);
  lcd.print("OUT1=");
  if ((nastavvystupy1 == true) || (nastavvystupy2 == true)) {
    lcd.write(byte(1));
    lcd.setCursor(8,1);
    lcd.print("AKTIVNI  ");
  } else {
    lcd.write(byte(2));
    lcd.setCursor(8,1);
    lcd.print("NEAKTIVNI");
  }

  lcd.setCursor(1, 2);
  lcd.print("OUT2=");
  if (nastavvystupy1 == true) {
    lcd.write(byte(1));
    lcd.setCursor(8,2);
    lcd.print("AKTIVNI  ");
  } else {
    lcd.write(byte(2));
    lcd.setCursor(8,2);
    lcd.print("NEAKTIVNI");
  }

  lcd.setCursor(1, 3);
  lcd.print("OUT3=");
  if (nastavvystupy2 == true) {
    lcd.write(byte(1));
    lcd.setCursor(8,3);
    lcd.print("AKTIVNI  ");
  } else {
    lcd.write(byte(2));
    lcd.setCursor(8,3);
    lcd.print("NEAKTIVNI");
  }

  char customKey = customKeypad.getKey();
     
    if (customKey){
    if (customKey == '^')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        lcd.setCursor(x, y);
        lcd.print(" ");
        y--;
        if (y == 1) {
          y = 3;
        }
        delay(100);
    }
    if (customKey == 'v')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        lcd.setCursor(x, y);
        lcd.print(" ");
        y++;
        if (y == 4) {
          y = 2;
        }
        delay(100);
    }
    if (customKey == 'C')
    {
        lcd.backlight();
        cas08 = millis() / 1000;
        goto ukonceni;
    }
    if (customKey == 'E')
    {
      lcd.backlight();
        cas08 = millis() / 1000;
        if ((AN2 < nastvlhkosti) && (AN1 > nasthladiny)) {
                                                              
        if (promenna == 2) {
          if (nastavvystupy1 == false) {
            nastavvystupy1 = true;
            delay(200);
            goto navesti;
          }
          if (nastavvystupy1 == true) {
            nastavvystupy1 = false;
            delay(200);
            goto navesti;            
          }
        }
        if (promenna == 3) {
          if (nastavvystupy2 == false) {
            nastavvystupy2 = true;
            delay(200);
            goto navesti;            
          }
          if (nastavvystupy2 == true) {
            nastavvystupy2 = false;
            delay(200);
            goto navesti;            
          }
        }
      }
    }
  }
   
goto navesti;
ukonceni:
  x = 0;
  y = 3;
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------
