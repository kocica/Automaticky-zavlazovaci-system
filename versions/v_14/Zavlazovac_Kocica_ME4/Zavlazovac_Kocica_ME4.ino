/*
Kočica Filip, ME 3
 16.5.2015

 Program pro řízení zavlažovacího systému.

 Na display se lze pohybovat kurzorem ve tvaru šipky pomocí tlačítek na membránové klávesnici.

 ----------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Directiva include pro zahrnutí knihoven do programu
#include <EEPROM.h>
#include <Wire.h>                                        //knihvna pro práci s čtením/zápisem z/do ORC
#include "Wire.h"                                        //                  -||-
#include <Keypad.h>                                      //knihovna pro komunikaci s membránovou klávesnicí 5x4
#include <LiquidCrystal_I2C.h>                           //práce s displayem přes i2c                                             
#define ORC_I2C_ADRESA 0x68                              //definice adresy v paměti


// Piny použité pro připojení LCD displaye (20x4, přes i2c)
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Zadeklarování prototypů fcí a definice parametrů.
int nastaveniParametru(int limitdisplaye, int nastavhladiny , int nastavvlhkosti);                                           //fce pro nastavení hladiny,vlhkosti,limitu displaye
int cas(int limitdisplaye);                                                                                                  //fce pro nastavení Po-Ne (hodiny,minuty,min1,min2)
int nastavcas(int limitdisplaye);                                                                                            //fce pro nastavení obvodu reálného času
int nastavvystupy(int limitdisplaye);                                                                                        //fce pro nastavení výstupu (LOW/HIGH)
int hlaseniPoruchy(bool nastavvystupy1, bool nastavvystupy2, int Binarnivstup1, int poc, int poc2, int i);                   //fce pro hlášení poruchy (binární vstup == 0)

void nactiVstupy();                                                                                                          //procedura na načtení vstupů
void prectiEEPROM();
void zkontrolujEEPROM();
void problikavani();
void prace_s_vystupy();
void zobrazNaDisplayi();
void stlaceniTlacitkaLoop();

byte DECnaBCD(byte hodnota);                                                                                                 //fce pro převod decimální hodnoty do kodu BCD pro obvod reálného času -
byte BCDnaDEC(byte hodnota);                                                                                                 //- a z BCD na decimální hodnotu
void cteniORC(byte *sekundy, byte *minuty, byte *hodiny, byte *denVTydnu, byte *denVMesici);                                 //procedura s parametry pro čtení z obvodu reálného času
void zapisORC(byte sekundy, byte minuty, byte hodiny, byte denVTydnu, byte denVMesici);                                      //procedura s parametry pro uložení do obvodu reálného času

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Znaky
byte ctverec1[8] = {        //plný čtverec

  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
};

byte ctverec2[8] = {     //prázdný čtverec

  B00000,
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000,
};


byte sipka [8] = {      //kurzor

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

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const byte radky = 5; // 5 řádků
const byte sloupce = 4; // 4 sloupce

//více rozměrné pole
char klavesnice[radky][sloupce] = {
  {
    'A', 'B', '#', '*'
  }
  ,
  {
    '1', '2', '3', '^'
  }
  ,
  {
    '4', '5', '6', 'v'
  }
  ,
  {
    '7', '8', '9', 'C'
  }
  ,
  {
    '<', '0', '>', 'E'
  }
};

byte pinyRadku[radky] = {
  46, A0, A1, A2, A3
}; //čísla pinů s řadkem 1 2 3 4 5
byte pinySloupcu[sloupce] = {
  A7, A6, A5, A4
}; //čísla pinu se sloupcem 1 2 3 4


//inicializuje objekt klávesnice s názvem stisknutiTlacitka
char posledniZmackle;
bool pusteniTlacitka;
Keypad stisknutiTlacitka = Keypad(makeKeymap(klavesnice), pinyRadku, pinySloupcu, radky, sloupce);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//globalni promenne
short x, y, i, a, o;  // "pomocné proměnne"

int poc, poc2;      //počitadla

byte sekundy, minuty, hodiny, denVTydnu, denVMesici;    // globální proměnné pro obvod reálného času

int AN1, AN2, Binarnivstup1;            //analogové vstupy 1,2 a binární vstup 1, kam se ukládají přepočítané hodnoty ze vstupů

bool nastavvystupy1 = false;
bool nastavvystupy2 = false;  // booleanske promenne pro nastavovani výstupu a pro nacteni vstupu
bool nacitaniVstupu = true;
bool soucasnyBeh = false;
bool soucasnyBehVystupy = false;

unsigned long cekej, cekej01;     //časovače

int zhasnuti = 0, identifik = 0;
int nedele2 = 0;
int analogvstup1 = A14;    // Analogový vstup
int analogvstup2 = A12;    //      -||-
int cteniVstupu1 = A15;    // Tyto proměnné se nastaví na "HIGH" pouze při čtení vstupů
int cteniVstupu2 = A13;    // a pak se nastaví zpět na "LOW", aby do čidla nešlo neustále napájení
int NapajeniBV1 = A9;      // Napajeni Binarnich vstupu
int BV1 = A8;              // binarni vstup na desce (BV1)
int plus = 53;             // Piezo bzučák
int signal = 2;            //     -||-

int limitdisplaye = 0;        // po jake době se vypne podsvícení displaye
int nastvlhkosti = 0, nasthladiny = 0;    // Přiřazení hodnot pro nastavování

// static array(pole se statickou vazbou)
int nastav_vystup[] = {0 , 0, 0, 0, 0, 0, 0};
int rele[] = { 47, 49, 51, 53 };
double cas_vystupu = 0, cas2 = 0;
int nastavORC [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//struktura pole
typedef struct {
  int hodiny1;
  int hodiny2;
  int hodinycelk;
  int minuty1;
  int minuty2;
  int minutycelk;
  int min1;
  int min10;
  int min1celk;
  int min2;
  int min20;
  int min2celk;
}
nastaveniDny;

//strukturové pole
nastaveniDny dny[6];

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);                           //begin pro objekt Serial,definuje 9600 baudů
  Wire.begin();                                 //begin pro objekt Wire (zápis a čtení z ORČ)
  lcd.begin(20, 4);                             //definuje objekt o velikosti 20-4
  lcd.backlight();                              //podsvícení displaye

  /*
  for (int i = 0; i < 1000; i++)                //při mazání paměti se musí znovu nastavit obvod reálného času
  EEPROM.write(i, 0);                           //Vycisti EEPROM - 1kB
  */

  //zapisORC(0,0,0,0,0);                        // formát (vteřiny,minuty,hodiny,denVTydnu,denVMesici) - nastavení Obvodu Reálného Času, při deklaraci smazat poznámku (//) mimochodem čas lze nastavit na displayi vpravo dole

  pinMode(NapajeniBV1, OUTPUT);
  digitalWrite(NapajeniBV1, HIGH);              //Binarni vstup, definuje ho jako INPUT a napájení pro něj
  pinMode(BV1, INPUT);

  pinMode(analogvstup1, INPUT);
  pinMode(analogvstup2, INPUT);                 //Analogove vstupy

  lcd.createChar(1, ctverec1);
  lcd.createChar(2, ctverec2);
  lcd.createChar(3, sipka);                     //Vytvoření objektů znaků
  lcd.createChar(4, objekt);

  pinMode(plus, OUTPUT);
  pinMode(signal, OUTPUT);                      //PiezoBzučák

  for (i = 0; i < 4; i++) {
    pinMode(rele[i], OUTPUT);
    if (i == 3) digitalWrite(rele[i], LOW);     //nastaví relátka jako výstupy a 4. relé definuje jako LOW
  }

  pinMode(cteniVstupu1, OUTPUT);                //Tyto výstupy se přepnou do log. "1" pouze při čtení vstupů,pak se nastaví zpět na log. "0".
  pinMode(cteniVstupu2, OUTPUT);

  nactiVstupy();                                //funkce která načte hodnoty z analog. a binar. vstupů a uloží je do globálních proměnných

  zkontrolujEEPROM();                           //procedura zapíše nulu na místo,kde je v EEPROM 255
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  prectiEEPROM();                                                                                     // načte data z EEPROM

  zobrazNaDisplayi();                                                                                 // Zbrazí na Displayi

  problikavani();                                                                                     // zajištuje blikani kurzoru a a dvojtečky

  prace_s_vystupy();                                                                                  // kontroluje (zapíná/vypíná) výstupy

  if  (zhasnuti >= limitdisplaye) lcd.noBacklight();                                                  // vypnutí podsvícení displaye po uplynutí nastaveného času bez stlačení tlačítka

  Binarnivstup1 = digitalRead(BV1);
  
  if (!Binarnivstup1) hlaseniPoruchy(nastavvystupy1, nastavvystupy2, Binarnivstup1, poc, poc2, i);    // zavolá fci hlášení poruchy, protože binární vstup 1 == 0

  stlaceniTlacitkaLoop();                                                                             // Snímá stlačení tlačítka a pro přísnušlé tlačítko vykoná příslušné operace

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// FUNKCE
int nastaveniParametru(int limitdisplaye, int nasthladiny, int nastvlhkosti) {

  prectiEEPROM();               // načte data z EEPROM
  cas2 = millis() / 1000;
  lcd.backlight();

  for (;;) {

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();

    if (a == 0) {
      lcd.setCursor(0, 0);
      lcd.write(byte(3));
    }
    if (a == 1) {
      lcd.setCursor(0, 1);
      lcd.write(byte(3));
    }
    if (a == 2) {
      lcd.setCursor(0, 2);
      lcd.write(byte(3));
    }
    lcd.setCursor(1, 0);
    lcd.write("NAST. VLHKOSTI=");

    if ((nastvlhkosti >= 10) && (nastvlhkosti < 100)) {
      lcd.setCursor(16, 0);
      lcd.print(" ");
      lcd.setCursor(17, 0);
      lcd.print(nastvlhkosti);
    }
    else if (nastvlhkosti < 10) {
      lcd.setCursor(17, 0);
      lcd.write(" ");
      lcd.setCursor(18, 0);
      lcd.print(nastvlhkosti);
    }
    else if (nastvlhkosti == 100) {
      lcd.setCursor(16, 0);
      lcd.print(nastvlhkosti);
    }
    lcd.setCursor(19, 0);
    lcd.print("%");
    lcd.setCursor(1, 1);
    lcd.write("NAST. HLADINY =");

    if ((nasthladiny >= 10) && (nasthladiny < 100)) {
      lcd.setCursor(16, 1);
      lcd.print(" ");
      lcd.setCursor(17, 1);
      lcd.print(nasthladiny);
    }
    else if (nasthladiny < 10) {
      lcd.setCursor(17, 1);
      lcd.write(" ");
      lcd.setCursor(18, 1);
      lcd.print(nasthladiny);
    }
    else if (nasthladiny == 100) {
      lcd.setCursor(16, 1);
      lcd.print(nasthladiny);
    }

    lcd.setCursor(19, 1);
    lcd.print("%");
    lcd.setCursor(1, 2);
    lcd.write("LIMIT DISPLAYE=");

    if ((limitdisplaye >= 10) && (limitdisplaye < 100)) {
      lcd.setCursor(16, 2);
      lcd.print(" ");
      lcd.setCursor(17, 2);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }
    else if (limitdisplaye < 10) {
      lcd.setCursor(17, 2);
      lcd.write(" ");
      lcd.setCursor(18, 2);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }
    else if (limitdisplaye >= 100) {
      lcd.setCursor(16, 2);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }

    char tlacitko = stisknutiTlacitka.getKey();
    KeyState stav = stisknutiTlacitka.getState();
    if (stav == PRESSED && tlacitko != NO_KEY) {
      posledniZmackle = tlacitko;
      pusteniTlacitka = false;
      if (posledniZmackle == 'C')    //konec (ESC)
      {
        lcd.clear();
        lcd.backlight();
        cas2 = millis() / 1000;
        break;
      }
      if (posledniZmackle == 'E')
      {
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);
        lcd.clear();
        EEPROM.write(0, nasthladiny);
        EEPROM.write(8, nastvlhkosti);
        EEPROM.write(16, limitdisplaye);
        lcd.backlight();
        cas2 = millis() / 1000;
      }
      if (posledniZmackle == '<')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          a = 2;
          delay(5);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          continue;
        }
        if (a == 1) {
          a = 0;
          delay(5);
          lcd.setCursor(0, 1);
          lcd.write(" ");
          continue;
        }
        if (a == 2) {
          a = 1;
          lcd.setCursor(0, 2);
          lcd.print(" ");
          continue;
        }
      }
      if (posledniZmackle == '>')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          a = 1;
          delay(5);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          continue;
        }
        if (a == 1) {
          a = 2;
          delay(5);
          lcd.setCursor(0, 1);
          lcd.write(" ");
          continue;
        }
        if (a == 2) {
          a = 0;
          lcd.setCursor(0, 2);
          lcd.print(" ");
          continue;
        }
      }
      if (posledniZmackle == '^')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti <= 99) {
            nastvlhkosti = nastvlhkosti + 1;
          }
        }
        if (a == 1) {
          if (nasthladiny <= 99) {
            nasthladiny = nasthladiny + 1;
          }
        }
        if (a == 2) {
          if (limitdisplaye <= 998) {
            limitdisplaye = limitdisplaye + 1;
          }
        }
      }
      if (posledniZmackle == 'v')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti >= 1) {
            nastvlhkosti = nastvlhkosti - 1;
          }
        }
        if (a == 1) {
          if (nasthladiny >= 1) {
            nasthladiny = nasthladiny - 1;
          }
        }
        if (a == 2) {
          if (limitdisplaye >= 1) {
            limitdisplaye = limitdisplaye - 1;
          }
        }
      }
    }
    else if (stav == RELEASED && !pusteniTlacitka) {
      pusteniTlacitka = true;
    }
    else if (stav == HOLD) {
      if (posledniZmackle == '^')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti <= 99) {
            nastvlhkosti = nastvlhkosti + 1;
          }
        }
        if (a == 1) {
          if (nasthladiny <= 99) {
            nasthladiny = nasthladiny + 1;
          }
        }
        if (a == 2) {
          if (limitdisplaye <= 998) {
            limitdisplaye = limitdisplaye + 1;
          }
        }
      }
      if (posledniZmackle == 'v')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          if (nastvlhkosti >= 1) {
            nastvlhkosti = nastvlhkosti - 1;
          }
        }
        if (a == 1) {
          if (nasthladiny >= 1) {
            nasthladiny = nasthladiny - 1;
          }
        }
        if (a == 2) {
          if (limitdisplaye >= 1) {
            limitdisplaye = limitdisplaye - 1;
          }
        }
      }
    }
  }
  lcd.clear();
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Proměnná "i" se nastaví podle dne.

int cas(int limitdisplaye) {                //funkce cas
  lcd.backlight();
  int promenna = 1;
  lcd.setCursor(6, 2);
  lcd.print(dny[i].min1);
  lcd.print(dny[i].min10);
  lcd.setCursor(12, 2);
  lcd.print(dny[i].min2);
  if (i == 6) lcd.print(nedele2);
  else lcd.print(dny[i].min20);

  for (;;) {

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    // Nacteni hodin a minut
    cteniORC(&sekundy, &minuty, &hodiny, &denVTydnu, &denVMesici);

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();
    else lcd.backlight();

    lcd.setCursor(0, 1);
    if (hodiny < 10) {
      lcd.print("0");
    }
    lcd.print(hodiny, DEC);          //vypsani casu

    lcd.print(":");
    if (minuty < 10) {
      lcd.print("0");
    }
    lcd.print(minuty, DEC);

    lcd.setCursor(5, 1);
    switch (denVTydnu) {
      case 1:
        lcd.print(" Pondeli");
        break;
      case 2:
        lcd.print(" Utery");
        break;
      case 3:
        lcd.print(" Streda");
        break;
      case 4:
        lcd.print(" Ctvrtek");
        break;
      case 5:
        lcd.print(" Patek");
        break;
      case 6:
        lcd.print(" Sobota");
        break;
      case 7:
        lcd.print(" Nedele");
        break;
    }

    lcd.setCursor(2, 0);
    lcd.print(":");

    lcd.setCursor(1, 3);
    lcd.print("Soucasny beh - ");
    if (soucasnyBeh) lcd.print("Zap");
    else lcd.print("Vyp");

    if (promenna == 1) {
      lcd.setCursor(1, 0);
      lcd.print(dny[i].hodiny2);
      lcd.setCursor(3, 0);
      lcd.print(dny[i].minuty1);
      lcd.setCursor(4, 0);
      lcd.print(dny[i].minuty2);
      lcd.setCursor(13, 2);
      if (i == 6) lcd.print(nedele2);
      else lcd.print(dny[i].min20);
    }
    if (promenna == 2) {
      lcd.setCursor(0, 0);
      lcd.print(dny[i].hodiny1);
      lcd.setCursor(3, 0);
      lcd.print(dny[i].minuty1);
      lcd.setCursor(4, 0);
      lcd.print(dny[i].minuty2);
    }
    if (promenna == 3) {
      lcd.setCursor(0, 0);
      lcd.print(dny[i].hodiny1);
      lcd.setCursor(1, 0);
      lcd.print(dny[i].hodiny2);
      lcd.setCursor(4, 0);
      lcd.print(dny[i].minuty2);
    }
    if (promenna == 4) {
      lcd.setCursor(0, 0);
      lcd.print(dny[i].hodiny1);
      lcd.setCursor(1, 0);
      lcd.print(dny[i].hodiny2);
      lcd.setCursor(3, 0);
      lcd.print(dny[i].minuty1);
      lcd.setCursor(6, 2);
      lcd.print(dny[i].min1);
    }
    if (promenna == 5) {
      lcd.setCursor(3, 0);
      lcd.print(dny[i].minuty1);
      lcd.setCursor(7, 2);
      lcd.print(dny[i].min10);
    }
    if (promenna == 6) {
      lcd.setCursor(12, 2);
      lcd.print(dny[i].min2);
      lcd.setCursor(6, 2);
      lcd.print(dny[i].min1);
    }
    if (promenna == 7) {
      lcd.setCursor(13, 2);
      if (i == 6) lcd.print(nedele2);
      else lcd.print(dny[i].min20);
      lcd.setCursor(7, 2);
      lcd.print(dny[i].min10);
    }
    if (promenna == 8) {
      lcd.setCursor(12, 2);
      lcd.print(dny[i].min2);
      lcd.setCursor(1, 0);
      lcd.print(dny[i].hodiny2);
    }

    if ((promenna == 6) || (promenna == 1)) {
      lcd.setCursor(11, 2);
      lcd.write(" ");
    }
    if ((promenna == 7) || (promenna == 4)) {
      lcd.setCursor(5, 2);
      lcd.write(" ");
    }

    if ((promenna == 1) || (promenna == 2)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Hodiny");
      lcd.setCursor(0, 3);
      lcd.write(" ");
    }
    if ((promenna == 3) || (promenna == 4)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Minuty");
    }
    if ((promenna == 5) || (promenna == 6)) {
      lcd.setCursor(6, 0);
      lcd.write("             ");
      lcd.setCursor(5, 2);
      lcd.write(byte(3));
    }
    if ((promenna == 7) || (promenna == 8)) {
      lcd.setCursor(6, 0);
      lcd.write("             ");
      lcd.setCursor(11, 2);
      lcd.write(byte(3));
      lcd.setCursor(0, 3);
      lcd.write(" ");
    }

    if (promenna == 9) {
      lcd.setCursor(0, 3);
      lcd.write(byte(3));
      lcd.setCursor(0, 0);
      lcd.print(dny[i].hodiny1);
      lcd.setCursor(11, 2);
      lcd.print(" ");
      lcd.setCursor(13, 2);
      if (i == 6) lcd.print(nedele2);
      else lcd.print(dny[i].min20);
    }


    if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
      switch (promenna) {
        case 1:
          lcd.setCursor(0, 0);
          lcd.write(byte(4));
          break;
        case 2:
          lcd.setCursor(1, 0);
          lcd.write(byte(4));
          break;
        case 3:
          lcd.setCursor(3, 0);
          lcd.write(byte(4));
          break;
        case 4:
          lcd.setCursor(4, 0);
          lcd.write(byte(4));
          break;
        case 5:
          lcd.setCursor(6, 2);
          lcd.write(byte(4));
          break;
        case 6:
          lcd.setCursor(7, 2);
          lcd.write(byte(4));
          break;
        case 7:
          lcd.setCursor(12, 2);
          lcd.write(byte(4));
          break;
        case 8:
          lcd.setCursor(13, 2);
          lcd.write(byte(4));
          break;
      }
    }


    if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
      switch (promenna) {
        case 1:
          lcd.setCursor(0, 0);
          lcd.print(dny[i].hodiny1);
          break;
        case 2:
          lcd.setCursor(1, 0);
          lcd.print(dny[i].hodiny2);
          break;
        case 3:
          lcd.setCursor(3, 0);
          lcd.print(dny[i].minuty1);
          break;
        case 4:
          lcd.setCursor(4, 0);
          lcd.print(dny[i].minuty2);
          break;
        case 5:
          lcd.setCursor(6, 2);
          lcd.print(dny[i].min1);
          break;
        case 6:
          lcd.setCursor(7, 2);
          lcd.print(dny[i].min10);
          break;
        case 7:
          lcd.setCursor(12, 2);
          lcd.print(dny[i].min2);
          break;
        case 8:
          lcd.setCursor(13, 2);
          if (i == 6) lcd.print(nedele2);
          else lcd.print(dny[i].min20);
          break;
      }
    }

    if (millis() > cekej + 1000) cekej = millis();


    char tlacitko = stisknutiTlacitka.getKey();

    if (tlacitko) {
      if (tlacitko == '0')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 1) {
          dny[i].hodiny1 = 0;
          delay(200);
        }
        if (promenna == 2) {
          dny[i].hodiny2 = 0;
          delay(200);
        }
        if (promenna == 3) {
          dny[i].minuty1 = 0;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 0;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 0;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 0;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 0;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 0;
          else dny[i].min20 = 0;
          delay(200);
        }
      }
      if (tlacitko == '1')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 1) {
          dny[i].hodiny1 = 1;
          delay(200);
        }
        if (promenna == 2) {
          dny[i].hodiny2 = 1;
          delay(200);
        }
        if (promenna == 3) {
          dny[i].minuty1 = 1;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 1;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 1;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 1;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 1;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 1;
          else dny[i].min20 = 1;
          delay(200);
        }
      }
      if (tlacitko == '2')
      {
        cas2 = millis() / 1000;
        lcd.backlight();
        if (dny[i].hodiny2 <= 3) {
          if (promenna == 1) {
            dny[i].hodiny1 = 2;
            delay(200);
          }
        }
        if (promenna == 2) {
          dny[i].hodiny2 = 2;
          delay(200);
        }
        if (promenna == 3) {
          dny[i].minuty1 = 2;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 2;
        }
        delay(200);
        if (promenna == 5) {
          dny[i].min1 = 2;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 2;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 2;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 2;
          else dny[i].min20 = 2;
          delay(200);
        }

      }
      if (tlacitko == '3')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          dny[i].hodiny2 = 3;
          delay(200);
        }
        if (promenna == 3) {
          dny[i].minuty1 = 3;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 3;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 3;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 3;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 3;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 3;
          else dny[i].min20 = 3;
          delay(200);
        }
      }
      if (tlacitko == '4')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 4;
            delay(200);
          }
        }
        if (promenna == 3) {
          dny[i].minuty1 = 4;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 4;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 4;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 4;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 4;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 4;
          else dny[i].min20 = 4;
          delay(200);
        }
      }
      if (tlacitko == '5')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 5;
            delay(200);
          }
        }

        if (promenna == 3) {
          dny[i].minuty1 = 5;
          delay(200);
        }
        if (promenna == 4) {
          dny[i].minuty2 = 5;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 5;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 5;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 5;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 5;
          else dny[i].min20 = 5;
          delay(200);
        }
      }
      if (tlacitko == '6')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 6;
            delay(200);
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 6;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 6;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 6;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 6;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 6;
          else dny[i].min20 = 6;
          delay(200);
        }
      }
      if (tlacitko == '7')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 7;
            delay(200);
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 7;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 7;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 7;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 7;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 7;
          else dny[i].min20 = 7;
          delay(200);
        }
      }
      if (tlacitko == '8')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 8;
            delay(200);
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 8;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 8;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 8;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 8;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 8;
          else dny[i].min20 = 8;
          delay(200);
        }
      }
      if (tlacitko == '9')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 9;
            delay(200);
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 9;
          delay(200);
        }
        if (promenna == 5) {
          dny[i].min1 = 9;
          delay(200);
        }
        if (promenna == 6) {
          dny[i].min10 = 9;
          delay(200);
        }
        if (promenna == 7) {
          dny[i].min2 = 9;
          delay(200);
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 9;
          else dny[i].min20 = 9;
          delay(200);
        }
      }

      if (tlacitko == '>')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna < 10) {
          promenna++;
          delay(200);
          if (promenna == 5) {
            lcd.setCursor(4, 0);
            lcd.print(dny[i].minuty2);
          }
          if (promenna == 10) {
            promenna = 1;
          }
        }
      }
      if (tlacitko == '<')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 0) {
            promenna = 9;
          }
        }
      }
      if (tlacitko == '^')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (promenna == 9) {
          if (soucasnyBeh == false) {
            soucasnyBeh = true;
            delay(50);
            continue;
          }
          if (soucasnyBeh == true) {
            soucasnyBeh = false;
            delay(50);
            continue;
          }
          break;
        }
      }
      if (tlacitko == 'v')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (promenna == 9) {
          if (soucasnyBeh == false) {
            soucasnyBeh = true;
            delay(50);
            continue;
          }
          if (soucasnyBeh == true) {
            soucasnyBeh = false;
            delay(50);
            continue;
          }
          break;
        }
      }
      if (tlacitko == 'C')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        break;
      }
      if (tlacitko == 'E')
      {
        lcd.backlight();
        cas2 = millis() / 1000;

        dny[i].hodinycelk = (dny[i].hodiny1 * 10) + dny[i].hodiny2;

        dny[i].minutycelk = (dny[i].minuty1 * 10) + dny[i].minuty2;

        dny[i].min1celk = ((dny[i].min1 * 10) + dny[i].min10);

        if (i == 6) dny[i].min2celk = ((dny[i].min2 * 10) + nedele2);
        else dny[i].min2celk = ((dny[i].min2 * 10) + dny[i].min20);

        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);
        lcd.clear();

        EEPROM.write(24, nedele2);
        EEPROM.write(32, soucasnyBeh);

        int c = 40;
        for (int a = 0; a < 7; a++) {
          EEPROM.write(c, dny[a].hodiny1);
          c += 8;
          EEPROM.write(c, dny[a].hodiny2);                      // zápis struktury do eeprom!
          c += 8;
          EEPROM.write(c, dny[a].hodinycelk);
          c += 8;
          EEPROM.write(c, dny[a].minuty1);
          c += 8;
          EEPROM.write(c, dny[a].minuty2);
          c += 8;
          EEPROM.write(c, dny[a].minutycelk);
          c += 8;
          EEPROM.write(c, dny[a].min1);
          c += 8;
          EEPROM.write(c, dny[a].min10);
          c += 8;
          EEPROM.write(c, dny[a].min1celk);
          c += 8;
          EEPROM.write(c, dny[a].min2);
          c += 8;
          if (a != 6) {
            EEPROM.write(c, dny[a].min20);
            c += 8;
          }
          EEPROM.write(c, dny[a].min2celk);
          c += 8;
        }
        promenna = 1;
        lcd.setCursor(6, 2);
        lcd.print(dny[i].min1);
        lcd.print(dny[i].min10);
        lcd.setCursor(12, 2);
        lcd.print(dny[i].min2);
        if (i == 6) lcd.print(nedele2);
        else lcd.print(dny[i].min20);
        cas2 = millis() / 1000;
        continue;
      }
    }
  }
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int nastavcas(int limitdisplaye) {
  int promenna = 1;

  // Nacteni hodin a minut
  cteniORC(&sekundy, &minuty, &hodiny, &denVTydnu, &denVMesici);

  nastavORC[7] = denVTydnu;
  nastavORC[2] = minuty / 10;
  nastavORC[8] = hodiny / 10;
  nastavORC[4] = minuty % 10;
  nastavORC[3] = hodiny % 10;

  lcd.backlight();
  cekej = millis();
  delay(100);

  for (;;) {

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();

    lcd.setCursor(0, 2);
    if (hodiny < 10) {
      lcd.print("0");
    }
    lcd.print(hodiny, DEC);          //vypsani casu

    lcd.print(":");
    if (minuty < 10) {
      lcd.print("0");
    }
    lcd.print(minuty, DEC);

    lcd.setCursor(9, 2);
    lcd.print("NAST.DNE");

    lcd.setCursor(6, 3);
    switch (nastavORC[7]) {
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

    if (promenna == 1) {
      lcd.setCursor(1, 0);
      lcd.print(nastavORC[3]);
      lcd.setCursor(3, 0);
      lcd.print(nastavORC[2]);
      lcd.setCursor(4, 0);
      lcd.print(nastavORC[4]);
    }
    if (promenna == 2) {
      lcd.setCursor(0, 0);
      lcd.print(nastavORC[8]);
      lcd.setCursor(3, 0);
      lcd.print(nastavORC[2]);
      lcd.setCursor(4, 0);
      lcd.print(nastavORC[4]);
    }
    if (promenna == 3) {
      lcd.setCursor(0, 0);
      lcd.print(nastavORC[8]);
      lcd.setCursor(1, 0);
      lcd.print(nastavORC[3]);
      lcd.setCursor(4, 0);
      lcd.print(nastavORC[4]);
    }
    if (promenna == 4) {
      lcd.setCursor(0, 0);
      lcd.print(nastavORC[8]);
      lcd.setCursor(1, 0);
      lcd.print(nastavORC[3]);
      lcd.setCursor(3, 0);
      lcd.print(nastavORC[2]);
    }

    if ((promenna == 1) || (promenna == 2)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Hodiny");
      lcd.setCursor(5, 3);
      lcd.write(" ");
    }
    if ((promenna == 3) || (promenna == 4)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Minuty");
    }
    if (promenna == 5) {
      lcd.setCursor(0, 0);
      lcd.print(nastavORC[8]);
      lcd.setCursor(6, 0);
      lcd.write("             ");
      lcd.setCursor(5, 3);
      lcd.write(byte(3));
    }

    lcd.setCursor(0, 1);
    lcd.write("Konec= Esc,Uloz= Ent");

    nastavORC[5] = (nastavORC[8] * 10) + nastavORC[3];

    nastavORC[6] = (nastavORC[2] * 10) + nastavORC[4];

    if ((millis() > cekej + 500) && (millis() < cekej + 1000)) {
      switch (promenna) {
        case 1:
          lcd.setCursor(0, 0);
          lcd.write(byte(4));
          break;
        case 2:
          lcd.setCursor(1, 0);
          lcd.write(byte(4));
          break;
        case 3:
          lcd.setCursor(3, 0);
          lcd.write(byte(4));
          break;
        case 4:
          lcd.setCursor(4, 0);
          lcd.write(byte(4));
          break;
      }
    }


    if ((millis() > cekej + 1) && (millis() < cekej + 500)) {
      switch (promenna) {
        case 1:
          lcd.setCursor(0, 0);
          lcd.print(nastavORC[8]);
          break;
        case 2:
          lcd.setCursor(1, 0);
          lcd.print(nastavORC[3]);
          break;
        case 3:
          lcd.setCursor(3, 0);
          lcd.print(nastavORC[2]);
          break;
        case 4:
          lcd.setCursor(4, 0);
          lcd.print(nastavORC[4]);
          break;
      }
    }

    if (millis() > cekej + 1000) cekej = millis();

    char tlacitko = stisknutiTlacitka.getKey();

    if (tlacitko) {
      if (tlacitko == '^')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (nastavORC[7] <= 6) {
            nastavORC[7]++;
          }
          delay(200);
        }
      }
      if (tlacitko == 'v')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 5) {
          if (nastavORC[7] >= 2) {
            nastavORC[7]--;
          }
          delay(200);
        }
      }

      if (tlacitko == '0')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 1) {
          nastavORC[8] = 0;
          delay(200);
        }
        if (promenna == 2) {
          nastavORC[3] = 0;
          delay(200);
        }
        if (promenna == 3) {
          nastavORC[2] = 0;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 0;
          delay(200);
        }
      }
      if (tlacitko == '1')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 1) {
          nastavORC[8] = 1;
          delay(200);
        }
        if (promenna == 2) {
          nastavORC[3] = 1;
          delay(200);
        }
        if (promenna == 3) {
          nastavORC[2] = 1;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 1;
          delay(200);
        }
      }
      if (tlacitko == '2')
      {
        cas2 = millis() / 1000;
        lcd.backlight();
        if (nastavORC[3] <= 3) {
          if (promenna == 1) {
            nastavORC[8] = 2;
            delay(200);
          }
        }
        if (promenna == 2) {
          nastavORC[3] = 2;
          delay(200);
        }
        if (promenna == 3) {
          nastavORC[2] = 2;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 2;
          delay(200);
        }
      }
      if (tlacitko == '3')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          nastavORC[3] = 3;
          delay(200);
        }
        if (promenna == 3) {
          nastavORC[2] = 3;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 3;
          delay(200);
        }
      }
      if (tlacitko == '4')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 4;
            delay(200);
          }
        }
        if (promenna == 3) {
          nastavORC[2] = 4;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 4;
          delay(200);
        }
      }
      if (tlacitko == '5')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 5;
            delay(200);
          }
        }

        if (promenna == 3) {
          nastavORC[2] = 5;
          delay(200);
        }
        if (promenna == 4) {
          nastavORC[4] = 5;
          delay(200);
        }
      }
      if (tlacitko == '6')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 6;
            delay(200);
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 6;
          delay(200);
        }
      }
      if (tlacitko == '7')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 7;
            delay(200);
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 7;
          delay(200);
        }
      }
      if (tlacitko == '8')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 8;
            delay(200);
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 8;
          delay(200);
        }
      }
      if (tlacitko == '9')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 9;
            delay(200);
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 9;
          delay(200);
        }
      }

      if (tlacitko == '>')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna < 6) {
          promenna++;
          if (promenna == 5) {
            lcd.setCursor(4, 0);
            lcd.print(nastavORC[4]);
          }
          delay(200);
          if (promenna == 6) {
            promenna = 1;
          }
        }
      }
      if (tlacitko == '<')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        if (promenna >= 1) {
          promenna--;
          delay(200);
          if (promenna == 4) {
            lcd.setCursor(5, 3);
            lcd.print(" ");
          }
          if (promenna == 0) {
            promenna = 5;
          }
        }
      }
      if (tlacitko == 'C')
      {
        lcd.backlight();
        cas2 = millis() / 1000;

        break;
      }
      if (tlacitko == 'E')
      {
        cas2 = millis() / 1000;
        lcd.backlight();

        zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
        lcd.clear();
        lcd.setCursor(6, 1);
        lcd.print("*ULOZENO*");
        delay(1000);
        lcd.setCursor(6, 1);
        lcd.print("         ");
        promenna = 1;
        continue;
      }
    }
  }
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int nastavvystupy(int limitdisplaye) {

  int promenna = 1;
  x = 0;
  y = 2;

  for (;;) {

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    promenna = y;
    lcd.setCursor(x, y);
    lcd.write(byte(3));
    delay(20);

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();

    lcd.setCursor(0, 0);
    lcd.print("*NASTAVENI  VYSTUPU*");

    lcd.setCursor(1, 1);
    lcd.print("OUT1=");
    if (((nastavvystupy1 == true) || (nastavvystupy2 == true)) || (soucasnyBehVystupy == true) || ((nastav_vystup[0] == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1)) || ((nastav_vystup[0] == 2) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2))) {
      lcd.write(byte(1));
      lcd.setCursor(8, 1);
      lcd.print("AKTIVNI  ");
    }
    else {
      lcd.write(byte(2));
      lcd.setCursor(8, 1);
      lcd.print("NEAKTIVNI");
    }

    lcd.setCursor(1, 2);
    lcd.print("OUT2=");
    if ((nastav_vystup[0] == 1) || (soucasnyBehVystupy == true) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1) || (nastavvystupy1 == true)) {
      lcd.write(byte(1));
      lcd.setCursor(8, 2);
      lcd.print("AKTIVNI  ");
    }
    else {
      lcd.write(byte(2));
      lcd.setCursor(8, 2);
      lcd.print("NEAKTIVNI");
    }

    lcd.setCursor(1, 3);
    lcd.print("OUT3=");
    if ((nastav_vystup[0] == 2) || (soucasnyBehVystupy == true) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2) || (nastavvystupy2 == true)) {
      lcd.write(byte(1));
      lcd.setCursor(8, 3);
      lcd.print("AKTIVNI  ");
    }
    else {
      lcd.write(byte(2));
      lcd.setCursor(8, 3);
      lcd.print("NEAKTIVNI");
    }

    char tlacitko = stisknutiTlacitka.getKey();

    if (tlacitko) {
      if (tlacitko == '^')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        lcd.setCursor(x, y);
        lcd.print(" ");
        y--;
        if (y == 1) {
          y = 3;
        }
        delay(100);
      }
      if (tlacitko == 'v')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        lcd.setCursor(x, y);
        lcd.print(" ");
        y++;
        if (y == 4) {
          y = 2;
        }
        delay(100);
      }
      if (tlacitko == 'C')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        break;
      }
      if (tlacitko == 'E')
      {
        lcd.backlight();
        cas2 = millis() / 1000;

        if (promenna == 2) {
          if (nastavvystupy1 == false) {
            nastavvystupy1 = true;
            delay(200);
            continue;
          }
          if (nastavvystupy1 == true) {
            nastavvystupy1 = false;
            delay(200);
            continue;
          }
        }
        if (promenna == 3) {
          if (nastavvystupy2 == false) {
            nastavvystupy2 = true;
            delay(200);
            continue;
          }
          if (nastavvystupy2 == true) {
            nastavvystupy2 = false;
            delay(200);
            continue;
          }
        }
      }
    }
  }
  x = 0;
  y = 3;
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int hlaseniPoruchy(bool nastavvystupy1, bool nastavvystupy2, int Binarnivstup1, int poc, int poc2, int i) {
  lcd.setCursor(1, 3);
  lcd.print("OUT:1");
  lcd.write(byte(2));
  lcd.print("2");
  lcd.write(byte(2));
  lcd.print("3");
  lcd.write(byte(2));
  lcd.setCursor(x, y);
  lcd.print(" ");
  lcd.setCursor(17, 2);
  lcd.write("   ");

  for (;;) {
    for (i = 0; i <= 6; i++) {
      nastav_vystup[i] = 0;
    }

    nastavvystupy1 = false;
    nastavvystupy2 = false;
    soucasnyBehVystupy = false;
    
    digitalWrite(rele[0], LOW);
    digitalWrite(rele[1], LOW);
    digitalWrite(rele[2], LOW);
    digitalWrite(rele[3], HIGH);

    for (poc = 0; poc < 20; poc++) {
      digitalWrite(signal, HIGH);
      delay(5);
      digitalWrite(signal, LOW);
      delay(5);
    }
    for (poc = 0; poc < 8; poc++) {
      digitalWrite(signal, LOW);
      delay(25);
    }

    poc2++;
    if ((poc2 > 3) && (poc2 <= 6)) {
      lcd.setCursor(17, 2);
      lcd.write("   ");
      if (poc2 == 6) poc2 = 0;
    }
    if ((poc2 >= 0) && (poc2 <= 3)) {
      lcd.setCursor(17, 2);
      lcd.write("P=");
      lcd.setCursor(19, 2);
      lcd.write(byte(1));
    }
    Binarnivstup1 = digitalRead(BV1);

    if (Binarnivstup1) break;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void stlaceniTlacitkaLoop() {
  cas2 = millis() / 1000;
  nactiVstupy();
  char tlacitko = stisknutiTlacitka.getKey();

  if (tlacitko) {
    zhasnuti = 0;
    lcd.backlight();
  }
  for (;;) {
    if (tlacitko == '^')
    {
      if (y == 0) {
        lcd.setCursor(x, y);
        lcd.write(" ");
        y = 3;
        x = 0;
        delay(20);
        break;
      }

      if (y == 1) {
        y = 0;
        x = 0;
        delay(20);
        lcd.setCursor(0, 1);
        lcd.write(" ");
        break;
      }
      if ((y == 3) && (x == 11)) {
        lcd.setCursor(11, 3);
        lcd.write(" ");
        y = 1;
        x = 0;
        delay(20);
        break;
      }
      if ((y == 3) && (x == 0)) {
        lcd.setCursor(0, 3);
        lcd.write(" ");
        y = 1;
        x = 0;
        delay(20);
        break;
      }
    }
    if (tlacitko == 'v')
    {
      if ((y == 1) && (x == 0)) {
        lcd.setCursor(0, 1);
        lcd.write(" ");
        x = 0;
        y = 3;
        delay(20);
        break;
      }
      if (y == 3) {
        lcd.setCursor(x, y);
        lcd.write(" ");
        x = 0;
        y = 0;
        delay(20);
        break;
      }
      if (y == 0) {
        y = 1;
        x = 0;
        delay(20);
        o = 0;
        while (o <= 18) {
          lcd.setCursor(o, 0);
          lcd.write(" ");
          o = o + 3;
        }
        break;
      }
    }
    if (tlacitko == '>')
    {
      if ((x == 11) && (y == 3)) {
        lcd.setCursor(11, 3);
        lcd.print(" ");
        x = 0;
        delay(50);
        break;
      }
      if ((x == 18) && (y == 0)) {
        lcd.setCursor(18, 0);
        lcd.print(" ");
        x = 0;
        delay(50);
        break;
      }
      if ((x == 15) && (y == 0)) {
        lcd.setCursor(15, 0);
        lcd.print(" ");
        x = 18;
        delay(50);
        break;
      }
      if ((x == 12) && (y == 0)) {
        lcd.setCursor(12, 0);
        lcd.print(" ");
        x = 15;
        delay(50);
        break;
      }
      if ((x == 9) && (y == 0)) {
        lcd.setCursor(9, 0);
        lcd.print(" ");
        x = 12;
        delay(50);
        break;
      }
      if ((x == 6) && (y == 0)) {
        lcd.setCursor(6, 0);
        lcd.print(" ");
        x = 9;
        delay(50);
        break;
      }
      if ((x == 3) && (y == 0)) {
        lcd.setCursor(3, 0);
        lcd.print(" ");
        x = 6;
        delay(50);
        break;
      }
      if ((x == 0) && (y == 0)) {
        lcd.setCursor(0, 0);
        lcd.print(" ");
        x = 3;
        delay(50);
        break;
      }
      if ((x == 0) && (y == 3)) {
        lcd.setCursor(0, 3);
        lcd.print(" ");
        x = 11;
        delay(50);
        break;
      }
    }
    if (tlacitko == '<')
    {
      if ((x == 0)&&(y == 3)) {
        lcd.setCursor(0, 3);
        lcd.print(" ");
        x = 11;
        delay(50);
        break;
      }
      if ((x == 0)&&(y == 0)) {
        lcd.setCursor(0, 0);
        lcd.print(" ");
        x = 18;
        delay(50);
        break;
      }
      if ((x == 3)&&(y == 0)) {
        lcd.setCursor(3, 0);
        lcd.print(" ");
        x = 0;
        delay(50);
        break;
      }
      if ((x == 6)&&(y == 0)) {
        lcd.setCursor(6, 0);
        lcd.print(" ");
        x = 3;
        delay(50);
        break;
      }
      if ((x == 9)&&(y == 0)) {
        lcd.setCursor(9, 0);
        lcd.print(" ");
        x = 6;
        delay(50);
        break;
      }
      if ((x == 12)&&(y == 0)) {
        lcd.setCursor(12, 0);
        lcd.print(" ");
        x = 9;
        delay(50);
        break;
      }
      if ((x == 15)&&(y == 0)) {
        lcd.setCursor(15, 0);
        lcd.print(" ");
        x = 12;
        delay(50);
        break;
      }
      if ((x == 18)&&(y == 0)) {
        lcd.setCursor(18, 0);
        lcd.print(" ");
        x = 15;
        delay(50);
        break;
      }
      if ((x == 11) && (y == 3)) {
        lcd.setCursor(11, 3);
        lcd.print(" ");
        x = 0;
        delay(50);
        break;
      }
    }
    break;
  }
  if (tlacitko == 'E')
  {
    if ((y == 3) && (x == 11)) {
      lcd.clear();
      nastavcas(limitdisplaye);
    }
    if ((y == 0) && (x == 0)) {
      lcd.clear();
      i = 0;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 3)) {                            // vyber v menu a skoky do fci
      lcd.clear();
      i = 1;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 6)) {
      lcd.clear();
      i = 2;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 9)) {
      lcd.clear();
      i = 3;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 12)) {
      lcd.clear();
      i = 4;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 15)) {
      lcd.clear();
      i = 5;
      cas(limitdisplaye);
    }
    if ((y == 0) && (x == 18)) {
      lcd.clear();
      i = 6;
      cas(limitdisplaye);
    }
    if ((y == 1) && (x == 0)) {
      lcd.clear();
      nastaveniParametru(limitdisplaye, nasthladiny , nastvlhkosti);
    }
    if ((y == 3) && (x == 0)) {
      lcd.clear();
      nastavvystupy(limitdisplaye);
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Funkce s převody pro obvod reálného času

byte DECnaBCD(byte hodnota) {
  return ( (hodnota / 10 * 16) + (hodnota % 10) );
}

byte BCDnaDEC(byte hodnota) {
  return ( (hodnota / 16 * 10) + (hodnota % 16) );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void prectiEEPROM() {
  nasthladiny = EEPROM.read(0);      // čtení z EEPROM
  nastvlhkosti = EEPROM.read(8);
  limitdisplaye = EEPROM.read(16);
  nedele2 = EEPROM.read(24);
  soucasnyBeh = EEPROM.read(32);

  int c = 40;
  for (int a = 0; a < 7; a++) {
    dny[a].hodiny1 = EEPROM.read(c);
    c += 8;
    dny[a].hodiny2 = EEPROM.read(c);
    c += 8;
    dny[a].hodinycelk = EEPROM.read(c);
    c += 8;
    dny[a].minuty1 = EEPROM.read(c);
    c += 8;
    dny[a].minuty2 = EEPROM.read(c);
    c += 8;
    dny[a].minutycelk = EEPROM.read(c);
    c += 8;
    dny[a].min1 = EEPROM.read(c);
    c += 8;
    dny[a].min10 = EEPROM.read(c);
    c += 8;
    dny[a].min1celk = EEPROM.read(c);
    c += 8;
    dny[a].min2 = EEPROM.read(c);
    c += 8;
    if (a != 6) {
      dny[a].min20 = EEPROM.read(c);
      c += 8;
    }
    dny[a].min2celk = EEPROM.read(c);
    c += 8;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void zobrazNaDisplayi() {
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
  }
  else {
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
  if (AN1 > nasthladiny) {
    lcd.write(byte(1));
  }
  else {
    lcd.write(byte(2));
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

  // Nacteni hodin a minut z obvodu reálného času
  cteniORC(&sekundy, &minuty, &hodiny, &denVTydnu, &denVMesici);

  lcd.setCursor(12, 3);
  switch (denVTydnu) {
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

  lcd.setCursor(15, 3);
  if (hodiny < 10) {
    lcd.print("0");
  }
  lcd.print(hodiny, DEC);          //vypsani casu
  lcd.setCursor(18, 3);
  if (minuty < 10) {
    lcd.print("0");
  }
  lcd.print(minuty, DEC);

  if (Binarnivstup1) {
    digitalWrite(rele[3], LOW);   //porucha
    lcd.setCursor(17, 2);
    lcd.write("P=");
    lcd.setCursor(19, 2);
    lcd.write(byte(2));
  }

  lcd.setCursor(1, 3);
  lcd.print("OUT:");
  lcd.print("1");
  if (((nastavvystupy1 == true) || (soucasnyBehVystupy == true) || (nastavvystupy2 == true)) || ((nastav_vystup[0] == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1)) || ((nastav_vystup[0] == 2) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2))) {
    lcd.write(byte(1));
    digitalWrite(rele[0], HIGH);

    nactiVstupy();
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele[0], LOW);
  }
  lcd.print("2");
  if ((nastav_vystup[0] == 1) || (soucasnyBehVystupy == true) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1) || (nastavvystupy1 == true)) {
    lcd.write(byte(1));
    digitalWrite(rele[1], HIGH);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele[1], LOW);
  }
  lcd.print("3");
  if ((nastav_vystup[0] == 2) || (soucasnyBehVystupy == true) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2) || (nastavvystupy2 == true)) {
    lcd.write(byte(1));
    digitalWrite(rele[2], HIGH);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele[2], LOW);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void problikavani() {
  if ((millis() > cekej01 + 1) && (millis() < cekej01 + 1500)) {
    lcd.setCursor(x, y);
    lcd.write(byte(3));        //blikání kurzoru
    delay(20);
  }

  if ((millis() > cekej01 + 1500) && (millis() < cekej01 + 1600)) {
    lcd.setCursor(x, y);
    lcd.write(" ");            //blikání kurzoru
    delay(20);
  }

  if (millis() > cekej01 + 1600) cekej01 = millis();

  if ((millis() > cekej + 1) && (millis() < cekej + 1000)) {
    lcd.setCursor(17, 3);
    lcd.print(":");                   //blikani dvojtecky v casu ve vterinovem intervalu
  }


  if ((millis() > cekej + 1000) && (millis() < cekej + 2000)) {
    lcd.setCursor(17, 3);
    lcd.print(" ");                   //blikani dvojtecky v casu ve vterinovem intervalu

  }
  if (millis() > cekej + 2000) {
    cekej = millis();
    zhasnuti += 2;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// funkce pro zápis a čtení z/do obvodu reálného času přes Wire.h
void zapisORC(byte sekundy, byte minuty, byte hodiny, byte denVTydnu, byte denVMesici) {
  Wire.beginTransmission(ORC_I2C_ADRESA);
  Wire.write(0);
  Wire.write(DECnaBCD(sekundy));
  Wire.write(DECnaBCD(minuty));
  Wire.write(DECnaBCD(hodiny));
  Wire.write(DECnaBCD(denVTydnu));
  Wire.write(DECnaBCD(denVMesici));
  Wire.endTransmission();
}

void cteniORC(byte *sekundy, byte *minuty, byte *hodiny, byte *denVTydnu, byte *denVMesici) {
  Wire.beginTransmission(ORC_I2C_ADRESA);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(ORC_I2C_ADRESA, 7);
  *sekundy = BCDnaDEC(Wire.read() & 0x7f);
  *minuty = BCDnaDEC(Wire.read());
  *hodiny = BCDnaDEC(Wire.read() & 0x3f);
  *denVTydnu = BCDnaDEC(Wire.read());
  *denVMesici = BCDnaDEC(Wire.read());
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void prace_s_vystupy() {

  for (i = 0; i < 7; i++) {
    if ((nastav_vystup[i] == 1) || (nastav_vystup[i] == 2)) {
      nastavvystupy1 = false;
      nastavvystupy2 = false;
    }
  }

  if ((AN2 > nastvlhkosti) || (AN1 < nasthladiny)) {
    for (i = 0; i <= 6; i++) {
      nastav_vystup[i] = 0;
    }
    soucasnyBehVystupy = false;
  }

  for (i = 0; i <= 6; i++) {
    if ((hodiny == dny[i].hodinycelk) && (minuty == (dny[i].minutycelk - 1)) && (denVTydnu == (i + 1))) {
      //nacteni vstupu minutu pred spuštěním výstupů
      if (nacitaniVstupu == true) {
        nactiVstupy();
      }
    }
  }

  if (!soucasnyBeh) {
    for (i = 0; i <= 6; i++) {

      if ((hodiny == dny[i].hodinycelk) && (minuty == dny[i].minutycelk) && (nastav_vystup[i] == 0) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (denVTydnu == (i + 1)) && ((dny[0].min1celk > 0) || (dny[0].min2celk > 0))) {
        nastav_vystup[i] = 1;
        nacitaniVstupu = true;
        if ((dny[i].min2celk > 0) && (dny[i].min1celk == 0)) nastav_vystup[i] = 2;
        cas_vystupu = millis() / 1000;
      }

      if (nastav_vystup[i] == 1) {
        if ((millis() / 1000) > (cas_vystupu + (dny[i].min1celk * 60))) {
          cas_vystupu = millis() / 1000;
          nastav_vystup[i] = 2;
        }
      }

      if (nastav_vystup[i] == 2) {
        if (dny[i].min2celk > 0) {
          if ((millis() / 1000) > (cas_vystupu + (dny[i].min2celk * 60))) {
            nastav_vystup[i] = 0;
            cas_vystupu = 0;
          }
        }
        else nastav_vystup[i] = 0;
      }
    }
  }
  else {
    for (i = 0; i <= 6; i++) {
      if ((hodiny == dny[i].hodinycelk) && (minuty == dny[i].minutycelk) && (soucasnyBehVystupy == false) && (AN2 < nastvlhkosti) && (AN1 > nasthladiny) && (denVTydnu == (i + 1)) && ((dny[0].min1celk > 0) || (dny[0].min2celk > 0))) {
        soucasnyBehVystupy = true;
        nacitaniVstupu = true;
        if (soucasnyBehVystupy == false) cas_vystupu = millis() / 1000;
        identifik = i;
      }

      if ((soucasnyBehVystupy == true) && (i == identifik)) {
        if ((millis() / 1000) > (cas_vystupu + ((dny[i].min1celk * 60) + (dny[i].min2celk * 60)))) {
          soucasnyBehVystupy = false;
          cas_vystupu = millis() / 1000;
        }
      }
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void nactiVstupy() {
  digitalWrite(cteniVstupu2, HIGH);
  digitalWrite(cteniVstupu1, HIGH);

  AN1 = analogRead(analogvstup1);
  AN1 = map(AN1, 0, 1023, 0, 100);

  AN2 = analogRead(analogvstup2);
  AN2 = map(AN2, 0, 1023, 0, 100);

  digitalWrite(cteniVstupu2, LOW);
  digitalWrite(cteniVstupu1, LOW);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void zkontrolujEEPROM() {
  int kontrolaEEPROM;
  for (int vycistiEEPROM = 0; vycistiEEPROM < 1000; vycistiEEPROM++) {
    kontrolaEEPROM = EEPROM.read(vycistiEEPROM);                                 // čtení z EEPROM
    if (kontrolaEEPROM == 255) {                                                 // Tam,kde je v EEPROM zapsano 255,to přepíše nulou
      EEPROM.write(vycistiEEPROM, 0);
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

