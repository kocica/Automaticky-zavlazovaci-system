/*
Kočica Filip, ME 3
 16.5.2015

 Program pro řízení zavlažovacího systému.

 Na display se lze pohybovat kurzorem ve tvaru šipky pomocí tlačítek na membránové klávesnici.

 ----------------------------------------------------------------------------------------------

 Na prvním řádku LCD displaye (20x4) jsou dny Po-Ne, kde můžete nastavit který
 den, v jakou hodinu a na jak dlouho budou sepnuty výstupy(OUT 1,2,3) a buď zapnout, nebo
 vypnout funkci současný běh.

 Na druhém řádku nastavujete vlhkost, hladinu, hysterezi a limit
 displaye. Nastavení vlhkosti a hladiny bude porovnáváno s hodnotami ze vstupů, pokud
 NV<MV(nastavená vlhkost bude menší než měřená vlhkost) a zároveň NH<MH (nastavená
 hladina bude menší než měřená hladina), bude možno výstupy sepnout, v opačném případě
 pokud NV>MV nebo NH>MH výstupy nebude možno sepnout. Nastavení hystereze je
 ošetření proti náhlému vypínání a spínaní relé. Limit displaye znamená, po jaké době se
 vypne podsvícení displaye, opětovné  zapnutí podsvícení displaye je možno provést pomocí
 libovolného tlačítka.

 Na třetím řádku je zobrazena MV (měřená vlhkost), MH (měřená hladina) a P(porucha).
 Pokud se na binárním vstupu (38) objeví Log. 0, Program bude hlásit
 poruchu pískáním piezo bzučáku a blikáním na displaye. Při poruše nelze sepnout žádný z
 výstupů. Poruchu lze vypnout pouze tím, že na binární vstup (38) opět přivedeme Log.1.

 Na čtvrtém řádku jsou zobrazeny výstupy a aktuální den + čas. Obě lze nastavit. Pokud je u
 kteréhokoli výstupu plný obdelník, znamená, že je sepnutý, pokud je u něj prázdný obdelník
 znamená, že je vypnutý. Výstupy lze nastavit výběrem v menu a skokem do funkce. Taktéž
 nastavení času lze nastavit výběrem v menu a skokem do funkce. Po nastavení je nutno
 potvrdit nastavení tlačítkem SELECT (Všechna data se uloží do EEPROM popřípadě Obvodu
 Reálného Času.

 */

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Directiva include pro zahrnutí knihoven do programu
#include <EEPROM.h>                                      //knihovna pro zápis a čtení z/do EEPROM
#include <Wire.h>                                        //knihvna pro práci s čtením/zápisem z/do ORC
#include "Wire.h"                                        //nihvna pro práci s čtením/zápisem z/do ORC
#include <Keypad.h>                                      //knihovna pro komunikaci s membránovou klávesnicí 5x4
#include <LiquidCrystal_I2C.h>                           //práce s displayem přes i2c   
#include <SPI.h>                                         //sériové periferní rozhraní, používá se pro komunikaci
#include <Ethernet.h>                                    //ethernetové rozhraní

#define ORC_I2C_ADRESA 0x68                              //definice adresy v paměti


// Piny použité pro připojení LCD displaye (20x4, přes i2c)
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

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

byte kurzor [8] = {      //kurzor

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

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 10, 145);

EthernetServer server(80);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Zadeklarování prototypů fcí a definice parametrů.
byte DECnaBCD(byte hodnota);                                                                                                 //fce pro převod decimální hodnoty do kodu BCD pro obvod reálného času -
byte BCDnaDEC(byte hodnota);                                                                                                 //- a z BCD na decimální hodnotu
void cteniORC(byte *sekundy, byte *minuty, byte *hodiny, byte *denVTydnu, byte *denVMesici);                                 //procedura s parametry pro čtení z obvodu reálného času
void zapisORC(byte sekundy, byte minuty, byte hodiny, byte denVTydnu, byte denVMesici);                                      //procedura s parametry pro uložení do obvodu reálného času

void zpozdeni(int doba);                                                                                                     //procedura má za úkol zpozdit program na takovou dobu, jaký se jí dá parametr v ms(milisekundách)
void problikavani();                                                                                                         //procedura zajišťující problikávání dvojtečky a šipky
void nactiVstupy(int analogvstup1, int analogvstup2, int cteniVstupu1, int cteniVstupu2);                                    //procedura na načtení vstupů
void zkontrolujEEPROM();                                                                                                     //procedura, která přepíše všechna místa v EEPROM,kde je 255 na 0.

int hlaseniPoruchy(bool nastavvystupy1, bool nastavvystupy2, int Binarnivstup1, int poc, int poc2, int i);                   //fce pro hlášení poruchy (binární vstup == 0)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Globální proměnné

short x, y, i;                                                                  // x,y = kurzor, i = se používá ve všech cyklech for

byte sekundy, minuty, hodiny, denVTydnu, denVMesici;                            // globální proměnné pro obvod reálného času

bool nastavvystupy1 = false, nastavvystupy2 = false, nacitaniVstupu = true, soucasnyBeh = false, refresh[2] {false,false};

unsigned long cekej, cekej01;                                                   //časovače

int identifik = 0, kteryVystup = 2, nedele2 = 0, oba_vystupy = 0, druhy_vystup = 0, BV1 = A8, soucasnyBehVystupy = 0, AN1, AN2, Binarnivstup1, zhasnuti = 0, Menu = 0;
// binarni vstup na desce (BV1) , analogové vstupy 1,2 a binární vstup 1, kam se ukládají přepočítané hodnoty ze vstupů
double cas_vystupu = 0, cas2 = 0;

int nastav_vystup[] = {0, 0, 0, 0, 0, 0, 0}, rele[] = { 47, 49, 51, 53 };       // static array (pole se statickou vazbou)


class Trida {
  private:

    int limitdisplaye,
        nastvlhkosti,
        nasthladiny,
        nasthystereze;

    //struktura pole
    typedef struct {
      int hodiny1;
      int hodiny2;
      int hodinycelk;
      int minuty1;                      // PRIVATE: (ZAPOUZDŘENÉ PROMĚNNÉ)
      int minuty2;                      // LZE S NIMI OPEROVAT POUZE POMOCÍ ČLENSKÝCH FCÍ / METOD
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

    int nastavORC[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


  public:                                                      // METODY / ČLENSKÉ FCE TŘÍDY
    Trida();                                                                                                                     //KONSTRUKTOR TŘÍDY
    int nastaveniParametru(int limitdisplaye, int nasthladiny, int nastvlhkosti, int nasthystereze);                             //fce pro nastavení hladiny,vlhkosti,limitu displaye
    int cas(int limitdisplaye);                                                                                                  //fce pro nastavení Po-Ne (hodiny,minuty,min1,min2)
    int nastavcas(int limitdisplaye);                                                                                            //fce pro nastavení obvodu reálného času
    int nastavvystupy(int limitdisplaye);                                                                                        //fce pro nastavení výstupu (LOW/HIGH)

    void limit();                                                                                                                //procedura hlídající vypnutí podsvícení displaye
    void prectiEEPROM();                                                                                                         //procedura, která načte data z EEPROM do proměnných
    void zapisEEPROM();                                                                                                          //procedura, která zapíše data do EEPROM
    void prace_s_vystupy();                                                                                                      //procedura, která zajišťuje správný běh výstupů (kdy se sepnou,na jak dlouho,kdy se rozepnou)
    void zobrazNaDisplayi();                                                                                                     //procedura má za úlohu vykreslit celé menu, se všemi hodnotami
    void stlaceniTlacitkaLoop();                                                                                                 //procedura, která zajišťuje správnou fci tlačítek
    void ethernet();                                                                                                             //tato procedura zajištuje celou sitovou komunikaci
    ~Trida();                                                                                                                    //DESTRUKTOR TŘÍDY
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);                           //begin pro objekt Serial,definuje 9600 baudů
  Wire.begin();                                 //begin pro objekt Wire (zápis a čtení z ORČ)
  lcd.begin(20, 4);                             //definuje objekt o velikosti 20-4
  Ethernet.begin(mac, ip);                      // inicalizace sítě
  server.begin();                               // inicalizace serveru
  lcd.backlight();                              //podsvícení displaye


  /*
  for (int i = 0; i < 1000; i++)                //při mazání paměti se musí znovu nastavit obvod reálného času
  EEPROM.write(i, 0);                           //Vycisti EEPROM - 1kB
  */

  //zapisORC(0,0,0,0,0);                        // formát (vteřiny,minuty,hodiny,denVTydnu,denVMesici) - nastavení Obvodu Reálného Času, při deklaraci smazat poznámku (//) mimochodem čas lze nastavit na displayi vpravo dole


  int NapajeniBV1 = A9;                         // Napajeni Binarnich vstupu
  pinMode(NapajeniBV1, OUTPUT);
  digitalWrite(NapajeniBV1, HIGH);              //Binarni vstup, definuje ho jako INPUT a napájení pro něj
  pinMode(BV1, INPUT);

  for (i = 0; i < 4; i++) {
    pinMode(rele[i], OUTPUT);
    if (i == 3) digitalWrite(rele[i], LOW);     //nastaví relátka jako výstupy a 4. relé definuje jako LOW
  }


  lcd.createChar(1, ctverec1);
  lcd.createChar(2, ctverec2);
  lcd.createChar(3, kurzor);                     //Vytvoření objektů znaků
  lcd.createChar(4, objekt);

  nactiVstupy(A14, A12, A15, A13);              //funkce která načte hodnoty z analog. a binar. vstupů a uloží je do globálních proměnných

  zkontrolujEEPROM();                           //procedura zapíše nulu na místo,kde je v EEPROM 255
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  int analogvstup1;                                                               // Analogový vstup
  int analogvstup2;                                                               //      -||-
  int cteniVstupu1;                                                               // Tyto proměnné se nastaví na "HIGH" pouze při čtení vstupů
  int cteniVstupu2;                                                               // a pak se nastaví zpět na "LOW", aby do čidla nešlo neustále napájení
  int poc, poc2;                                                                  //počitadla

  Trida den;                                                                      // vytvoření OBJEKTU den třídy Trida

  den.prectiEEPROM();                                                             // načte data z EEPROM

  den.prace_s_vystupy();                                                          // kontroluje (zapíná/vypíná) výstupy

  den.zobrazNaDisplayi();                                                         // Zobrazí na Displayi

  problikavani();                                                                 // zajištuje blikani kurzoru a a dvojtečky

  den.limit();                                                                    // vypnutí podsvícení displaye po uplynutí nastaveného času bez stlačení tlačítka

  Binarnivstup1 = digitalRead(BV1);                                               // načte hodnotu z BV1 (A8) do proměnné Binarnivstup1

  if (!Binarnivstup1) hlaseniPoruchy(nastavvystupy1, nastavvystupy2, Binarnivstup1, poc, poc2, i);    // zavolá fci hlášení poruchy, když binární vstup 1 == 0

  den.stlaceniTlacitkaLoop();                                                     // Snímá stlačení tlačítka a pro přísnušlé tlačítko vykoná příslušné operace

  den.ethernet();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// FUNKCE
int Trida::nastaveniParametru(int limitdisplaye, int nasthladiny, int nastvlhkosti, int nasthystereze) {

  prectiEEPROM();               // načte data z EEPROM

  cas2 = millis() / 1000;
  lcd.backlight();

  int a = 0;

  for (;;) {

    ethernet();

    if (refresh[2]) {
      nasthladiny = EEPROM.read(0);
      nastvlhkosti = EEPROM.read(1);
      nasthystereze = EEPROM.read(5);
      limitdisplaye = EEPROM.read(2);
      refresh[2] = false;
    }

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();

    lcd.setCursor(0, a);
    lcd.write(byte(3));

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

    lcd.setCursor(1, 3);
    lcd.write("LIMIT DISPLAYE=");

    if ((limitdisplaye >= 10) && (limitdisplaye < 100)) {
      lcd.setCursor(16, 3);
      lcd.print(" ");
      lcd.setCursor(17, 3);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }
    else if (limitdisplaye < 10) {
      lcd.setCursor(17, 3);
      lcd.write(" ");
      lcd.setCursor(18, 3);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }
    else if (limitdisplaye >= 100) {
      lcd.setCursor(16, 3);
      lcd.print(limitdisplaye);
      lcd.print("s");
    }

    lcd.setCursor(1, 2);
    lcd.write("NAST. HYSTEREZ=");

    if ((nasthystereze >= 10) && (nasthystereze < 100)) {
      lcd.setCursor(16, 2);
      lcd.print(" ");
      lcd.setCursor(17, 2);
      lcd.print(nasthystereze);
    }
    else if (nasthystereze < 10) {
      lcd.setCursor(17, 2);
      lcd.write(" ");
      lcd.setCursor(18, 2);
      lcd.print(nasthystereze);
    }
    else if (nasthystereze == 100) {
      lcd.setCursor(16, 2);
      lcd.print(nasthystereze);
    }

    for (int procenta = 0; procenta < 3; procenta++) {
      lcd.setCursor(19, procenta);
      lcd.print("%");
    }

    char tlacitko = stisknutiTlacitka.getKey();
    KeyState stav = stisknutiTlacitka.getState();
    if (stav == PRESSED && tlacitko != NO_KEY) {

      lcd.backlight();
      cas2 = millis() / 1000;

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

        EEPROM.write(0, nasthladiny);
        EEPROM.write(1, nastvlhkosti);
        EEPROM.write(2, limitdisplaye);                        // zápis proměnných do eeprom!
        EEPROM.write(5, nasthystereze);

        lcd.setCursor(0, 0);
        lcd.print("********************");
        lcd.setCursor(7, 1);
        lcd.print("ULOZENO");
        lcd.setCursor(9, 2);
        lcd.print("Do");
        lcd.setCursor(3, 3);
        lcd.print("Pameti EEPROM!");
        zpozdeni(2000);
        lcd.clear();

        lcd.backlight();
        cas2 = millis() / 1000;
      }

      if (posledniZmackle == '<')
      {
        lcd.backlight();
        cas2 = millis() / 1000;
        if (a == 0) {
          a = 3;
          zpozdeni(5);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          continue;
        }
        if (a == 1) {
          a = 0;
          zpozdeni(5);
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
        if (a == 3) {
          a = 2;
          lcd.setCursor(0, 3);
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
          zpozdeni(5);
          lcd.setCursor(0, 0);
          lcd.write(" ");
          continue;
        }
        if (a == 1) {
          a = 2;
          zpozdeni(5);
          lcd.setCursor(0, 1);
          lcd.write(" ");
          continue;
        }
        if (a == 2) {
          a = 3;
          zpozdeni(5);
          lcd.setCursor(0, 2);
          lcd.write(" ");
          continue;
        }
        if (a == 3) {
          a = 0;
          lcd.setCursor(0, 3);
          lcd.print(" ");
          continue;
        }
      }

      if (posledniZmackle == '^')
      {

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
          if (nasthystereze <= 9) {
            nasthystereze = nasthystereze + 1;
          }
        }
        if (a == 3) {
          if (limitdisplaye < 250) {
            limitdisplaye = limitdisplaye + 1;
          }
        }
      }

      if (posledniZmackle == 'v')
      {

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
          if (nasthystereze >= 1) {
            nasthystereze = nasthystereze - 1;
          }
        }
        if (a == 3) {
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

      lcd.backlight();
      cas2 = millis() / 1000;

      if (posledniZmackle == '^')
      {

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
          if (nasthystereze <= 9) {
            nasthystereze = nasthystereze + 1;
          }
        }
        if (a == 3) {
          if (limitdisplaye < 250) {
            limitdisplaye = limitdisplaye + 1;
          }
        }
      }

      if (posledniZmackle == 'v')
      {

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
          if (nasthystereze >= 1) {
            nasthystereze = nasthystereze - 1;
          }
        }
        if (a == 3) {
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

int Trida::cas(int limitdisplaye) {                //funkce cas
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

    ethernet();

    if (refresh[1]) {
      prectiEEPROM();
      refresh[1] = false;
      lcd.setCursor(0, 0);
      lcd.print(dny[i].hodiny1);
      lcd.setCursor(0, 1);
      lcd.print(dny[i].hodiny2);
      lcd.setCursor(0, 3);
      lcd.print(dny[i].minuty1);
      lcd.setCursor(0, 4);
      lcd.print(dny[i].minuty2);
      lcd.setCursor(6, 2);
      lcd.print(dny[i].min1);
      lcd.setCursor(7, 2);
      lcd.print(dny[i].min10);
      lcd.setCursor(12, 2);
      lcd.print(dny[i].min2);
      lcd.setCursor(13, 2);
      if (i == 6) lcd.print(nedele2);
      else lcd.print(dny[i].min20);
    }

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
      lcd.write("Nast. Hodiny  ");
      lcd.setCursor(0, 3);
      lcd.write(" ");
    }

    if ((promenna == 3) || (promenna == 4)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Minuty  ");
    }

    if ((promenna == 5) || (promenna == 6)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. 1.Okruh ");
      lcd.setCursor(5, 2);
      lcd.write(byte(3));
    }

    if ((promenna == 7) || (promenna == 8)) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. 2.Okruh ");
      lcd.setCursor(11, 2);
      lcd.write(byte(3));
      lcd.setCursor(0, 3);
      lcd.write(" ");
    }

    if (promenna == 9) {
      lcd.setCursor(6, 0);
      lcd.write("Nast. Souc.Beh");
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

      cas2 = millis() / 1000;
      lcd.backlight();

      if (tlacitko == '0')
      {

        if (promenna == 1) {
          dny[i].hodiny1 = 0;
        }

        if (promenna == 2) {
          dny[i].hodiny2 = 0;
        }
        if (promenna == 3) {
          dny[i].minuty1 = 0;
        }

        if (promenna == 4) {
          dny[i].minuty2 = 0;
        }

        if (promenna == 5) {
          dny[i].min1 = 0;
        }

        if (promenna == 6) {
          dny[i].min10 = 0;
        }

        if (promenna == 7) {
          dny[i].min2 = 0;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 0;
          else dny[i].min20 = 0;
        }
        zpozdeni(10);
      }

      if (tlacitko == '1')
      {

        if (promenna == 1) {
          dny[i].hodiny1 = 1;
        }

        if (promenna == 2) {
          dny[i].hodiny2 = 1;
        }

        if (promenna == 3) {
          dny[i].minuty1 = 1;
        }

        if (promenna == 4) {
          dny[i].minuty2 = 1;
        }

        if (promenna == 5) {
          dny[i].min1 = 1;
        }

        if (promenna == 6) {
          dny[i].min10 = 1;
        }

        if (promenna == 7) {
          dny[i].min2 = 1;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 1;
          else dny[i].min20 = 1;
        }
        zpozdeni(10);
      }

      if (tlacitko == '2')
      {

        if (dny[i].hodiny2 <= 3) {
          if (promenna == 1) {
            dny[i].hodiny1 = 2;
          }
        }

        if (promenna == 2) {
          dny[i].hodiny2 = 2;
        }

        if (promenna == 3) {
          dny[i].minuty1 = 2;
        }

        if (promenna == 4) {
          dny[i].minuty2 = 2;
        }

        if (promenna == 5) {
          dny[i].min1 = 2;
        }

        if (promenna == 6) {
          dny[i].min10 = 2;
        }

        if (promenna == 7) {
          dny[i].min2 = 2;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 2;
          else dny[i].min20 = 2;
        }
        zpozdeni(10);
      }

      if (tlacitko == '3')
      {

        if (promenna == 2) {
          dny[i].hodiny2 = 3;
        }

        if (promenna == 3) {
          dny[i].minuty1 = 3;
        }

        if (promenna == 4) {
          dny[i].minuty2 = 3;
        }

        if (promenna == 5) {
          dny[i].min1 = 3;
        }

        if (promenna == 6) {
          dny[i].min10 = 3;
        }

        if (promenna == 7) {
          dny[i].min2 = 3;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 3;
          else dny[i].min20 = 3;
        }
        zpozdeni(10);
      }

      if (tlacitko == '4')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 4;
          }
        }
        if (promenna == 3) {
          dny[i].minuty1 = 4;
        }
        if (promenna == 4) {
          dny[i].minuty2 = 4;
        }
        if (promenna == 5) {
          dny[i].min1 = 4;
        }
        if (promenna == 6) {
          dny[i].min10 = 4;
        }
        if (promenna == 7) {
          dny[i].min2 = 4;
        }
        if (promenna == 8) {
          if (i == 6) nedele2 = 4;
          else dny[i].min20 = 4;
        }
        zpozdeni(10);
      }

      if (tlacitko == '5')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 5;
          }
        }

        if (promenna == 3) {
          dny[i].minuty1 = 5;
        }

        if (promenna == 4) {
          dny[i].minuty2 = 5;
        }

        if (promenna == 5) {
          dny[i].min1 = 5;
        }

        if (promenna == 6) {
          dny[i].min10 = 5;
        }

        if (promenna == 7) {
          dny[i].min2 = 5;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 5;
          else dny[i].min20 = 5;
        }
        zpozdeni(10);
      }

      if (tlacitko == '6')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 6;
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 6;
        }

        if (promenna == 5) {
          dny[i].min1 = 6;
        }

        if (promenna == 6) {
          dny[i].min10 = 6;
        }

        if (promenna == 7) {
          dny[i].min2 = 6;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 6;
          else dny[i].min20 = 6;
        }
        zpozdeni(10);
      }

      if (tlacitko == '7')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 7;
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 7;
        }

        if (promenna == 5) {
          dny[i].min1 = 7;
        }

        if (promenna == 6) {
          dny[i].min10 = 7;
        }

        if (promenna == 7) {
          dny[i].min2 = 7;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 7;
          else dny[i].min20 = 7;
        }
        zpozdeni(10);
      }

      if (tlacitko == '8')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 8;
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 8;
        }

        if (promenna == 5) {
          dny[i].min1 = 8;
        }

        if (promenna == 6) {
          dny[i].min10 = 8;
        }

        if (promenna == 7) {
          dny[i].min2 = 8;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 8;
          else dny[i].min20 = 8;
        }
        zpozdeni(10);
      }

      if (tlacitko == '9')
      {

        if (promenna == 2) {
          if (dny[i].hodiny1 <= 1) {
            dny[i].hodiny2 = 9;
          }
        }

        if (promenna == 4) {
          dny[i].minuty2 = 9;
        }

        if (promenna == 5) {
          dny[i].min1 = 9;
        }

        if (promenna == 6) {
          dny[i].min10 = 9;
        }

        if (promenna == 7) {
          dny[i].min2 = 9;
        }

        if (promenna == 8) {
          if (i == 6) nedele2 = 9;
          else dny[i].min20 = 9;
        }
        zpozdeni(10);
      }

      if (tlacitko == '>')
      {
        if (promenna < 10) {
          promenna++;
          zpozdeni(10);
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
        if (promenna >= 1) {
          promenna--;
          zpozdeni(10);
          if (promenna == 0) {
            promenna = 9;
          }
        }
      }
      if (tlacitko == '^')
      {
        if (promenna == 9) {
          if (soucasnyBeh == false) {
            soucasnyBeh = true;
            zpozdeni(10);
            continue;
          }
          if (soucasnyBeh == true) {
            soucasnyBeh = false;
            zpozdeni(10);
            continue;
          }
          break;
        }
      }
      if (tlacitko == 'v')
      {
        if (promenna == 9) {
          if (soucasnyBeh == false) {
            soucasnyBeh = true;
            zpozdeni(10);
            continue;
          }
          if (soucasnyBeh == true) {
            soucasnyBeh = false;
            zpozdeni(10);
            continue;
          }
          break;
        }
      }
      if (tlacitko == 'C')
        break;

      if (tlacitko == 'E')
      {
        dny[i].hodinycelk = (dny[i].hodiny1 * 10) + dny[i].hodiny2;

        dny[i].minutycelk = (dny[i].minuty1 * 10) + dny[i].minuty2;

        dny[i].min1celk = ((dny[i].min1 * 10) + dny[i].min10);

        if (i == 6) dny[i].min2celk = ((dny[i].min2 * 10) + nedele2);
        else dny[i].min2celk = ((dny[i].min2 * 10) + dny[i].min20);

        zapisEEPROM();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("********************");
        lcd.setCursor(7, 1);
        lcd.print("ULOZENO");
        lcd.setCursor(9, 2);
        lcd.print("Do");
        lcd.setCursor(3, 3);
        lcd.print("Pameti EEPROM!");
        zpozdeni(2000);
        lcd.clear();

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

int Trida::nastavcas(int limitdisplaye) {
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
  zpozdeni(100);

  for (;;) {

    ethernet();

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
      lcd.write("Nast. Dne   ");
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

      cas2 = millis() / 1000;
      lcd.backlight();

      if (tlacitko == '^')
      {
        if (promenna == 5) {
          if (nastavORC[7] <= 6) {
            nastavORC[7]++;
          }
          zpozdeni(20);
        }
      }
      if (tlacitko == 'v')
      {
        if (promenna == 5) {
          if (nastavORC[7] >= 2) {
            nastavORC[7]--;
          }
          zpozdeni(20);
        }
      }

      if (tlacitko == '0')
      {
        if (promenna == 1) {
          nastavORC[8] = 0;
        }
        if (promenna == 2) {
          nastavORC[3] = 0;
        }
        if (promenna == 3) {
          nastavORC[2] = 0;
        }
        if (promenna == 4) {
          nastavORC[4] = 0;
        }
        zpozdeni(20);
      }

      if (tlacitko == '1')
      {
        if (promenna == 1) {
          nastavORC[8] = 1;
        }
        if (promenna == 2) {
          nastavORC[3] = 1;
        }
        if (promenna == 3) {
          nastavORC[2] = 1;
        }
        if (promenna == 4) {
          nastavORC[4] = 1;
        }
        zpozdeni(20);
      }

      if (tlacitko == '2')
      {
        if (nastavORC[3] <= 3) {
          if (promenna == 1) {
            nastavORC[8] = 2;
          }
        }
        if (promenna == 2) {
          nastavORC[3] = 2;
        }
        if (promenna == 3) {
          nastavORC[2] = 2;
        }
        if (promenna == 4) {
          nastavORC[4] = 2;
        }
        zpozdeni(20);
      }

      if (tlacitko == '3')
      {
        if (promenna == 2) {
          nastavORC[3] = 3;
        }
        if (promenna == 3) {
          nastavORC[2] = 3;
        }
        if (promenna == 4) {
          nastavORC[4] = 3;
        }
        zpozdeni(20);
      }

      if (tlacitko == '4')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 4;
          }
        }
        if (promenna == 3) {
          nastavORC[2] = 4;
        }
        if (promenna == 4) {
          nastavORC[4] = 4;
        }
        zpozdeni(20);
      }

      if (tlacitko == '5')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 5;
          }
        }

        if (promenna == 3) {
          nastavORC[2] = 5;
        }
        if (promenna == 4) {
          nastavORC[4] = 5;
        }
        zpozdeni(20);
      }

      if (tlacitko == '6')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 6;
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 6;
        }
        zpozdeni(20);
      }

      if (tlacitko == '7')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 7;
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 7;
        }
        zpozdeni(20);
      }

      if (tlacitko == '8')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 8;
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 8;
        }
        zpozdeni(20);
      }

      if (tlacitko == '9')
      {
        if (promenna == 2) {
          if (nastavORC[8] <= 1) {
            nastavORC[3] = 9;
          }
        }

        if (promenna == 4) {
          nastavORC[4] = 9;
        }
        zpozdeni(20);
      }

      if (tlacitko == '>')
      {
        if (promenna < 6) {
          promenna++;
          if (promenna == 5) {
            lcd.setCursor(4, 0);
            lcd.print(nastavORC[4]);
          }
          zpozdeni(20);
          if (promenna == 6) {
            promenna = 1;
          }
        }
      }
      if (tlacitko == '<')
      {
        if (promenna >= 1) {
          promenna--;
          zpozdeni(20);
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
        break;

      if (tlacitko == 'E')
      {
        zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("********************");
        lcd.setCursor(7, 1);
        lcd.print("ULOZENO");
        lcd.setCursor(9, 2);
        lcd.print("Do");
        lcd.setCursor(0, 3);
        lcd.print("Obvodu Realneho Casu");
        zpozdeni(2000);
        lcd.setCursor(0, 0);
        lcd.clear();
        promenna = 1;
        continue;
      }
    }
  }
  lcd.write("                                                                                ");
  return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int Trida::nastavvystupy(int limitdisplaye) {

  int promenna = 1;
  x = 0;
  y = 2;

  for (;;) {

    ethernet();

    Binarnivstup1 = digitalRead(BV1);
    if (!Binarnivstup1) break;

    promenna = y;
    lcd.setCursor(x, y);
    lcd.write(byte(3));
    zpozdeni(20);

    if ((millis() / 1000) > (cas2 + (1 * limitdisplaye))) lcd.noBacklight();

    lcd.setCursor(0, 0);
    lcd.print("*NASTAVENI  VYSTUPU*");

    lcd.setCursor(1, 1);
    lcd.print("OUT1=");
    if (((nastavvystupy1 == true) || ((soucasnyBehVystupy == 2) && ((kteryVystup == 0) || (kteryVystup == 1))) || (soucasnyBehVystupy == 1) || (nastavvystupy2 == true)) || ((nastav_vystup[0] == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1)) || ((nastav_vystup[0] == 2) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2))) {
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
    if ((nastav_vystup[0] == 1) || ((soucasnyBehVystupy == 2) && kteryVystup == 0) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1) || (nastavvystupy1 == true)) {
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
    if ((nastav_vystup[0] == 2) || ((soucasnyBehVystupy == 2) && kteryVystup == 1) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2) || (nastavvystupy2 == true)) {
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

      lcd.backlight();
      cas2 = millis() / 1000;

      if (tlacitko == '^')
      {
        lcd.setCursor(x, y);
        lcd.print(" ");
        y--;
        if (y == 1) {
          y = 3;
        }
        zpozdeni(20);
      }

      if (tlacitko == 'v')
      {
        lcd.setCursor(x, y);
        lcd.print(" ");
        y++;
        if (y == 4) {
          y = 2;
        }
        zpozdeni(20);
      }

      if (tlacitko == 'C')
        break;

      if (tlacitko == 'E')
      {
        if (promenna == 2) {
          if (nastavvystupy1 == false) {
            nastavvystupy1 = true;
            zpozdeni(20);
            continue;
          }
          if (nastavvystupy1 == true) {
            nastavvystupy1 = false;
            zpozdeni(20);
            continue;
          }
        }
        if (promenna == 3) {
          if (nastavvystupy2 == false) {
            nastavvystupy2 = true;
            zpozdeni(20);
            continue;
          }
          if (nastavvystupy2 == true) {
            nastavvystupy2 = false;
            zpozdeni(20);
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

  int plus = 53;                                                                  // Piezo bzučák
  int signal = 2;                                                                 //     -||-
  pinMode(plus, OUTPUT);
  pinMode(signal, OUTPUT);                                                        //PiezoBzučák

  lcd.setCursor(1, 3);
  lcd.print("OUT:1");
  lcd.write(byte(2));
  lcd.print("2");
  lcd.write(byte(2));
  lcd.print("3");
  lcd.write(byte(2));
  lcd.setCursor(x, y);
  lcd.print(" ");
  lcd.setCursor(17, 3);
  lcd.write(":");
  lcd.setCursor(17, 2);
  lcd.write("   ");
  cekej01 = millis();

  for (;;) {

    //ethernet();

    for (i = 0; i <= 6; i++) {
      nastav_vystup[i] = 0;
    }

    nastavvystupy1 = false;
    nastavvystupy2 = false;
    soucasnyBehVystupy = 0;

    digitalWrite(rele[0], LOW);
    digitalWrite(rele[1], LOW);
    digitalWrite(rele[2], LOW);
    digitalWrite(rele[3], HIGH);

    if ((millis() > cekej01 + 1) && (millis() < cekej01 + 500)) {
      lcd.backlight();
      lcd.setCursor(17, 2);
      lcd.write("P=");
      lcd.setCursor(19, 2);
      lcd.write(byte(1));
    }

    if ((millis() > cekej01 + 500) && (millis() < cekej01 + 1000)) {
      lcd.noBacklight();
      lcd.setCursor(17, 2);
      lcd.write("   ");
    }

    if (millis() > cekej01 + 1000) cekej01 = millis();

    for (poc = 0; poc < 20; poc++) {
      digitalWrite(signal, HIGH);
      zpozdeni(5);
      digitalWrite(signal, LOW);
      zpozdeni(5);
    }
    for (poc = 0; poc < 8; poc++) {
      digitalWrite(signal, LOW);
      zpozdeni(25);
    }

    Binarnivstup1 = digitalRead(BV1);

    if (Binarnivstup1) break;
  }
  lcd.backlight();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Trida::stlaceniTlacitkaLoop() {
  cas2 = millis() / 1000;
  char tlacitko = stisknutiTlacitka.getKey();

  if (tlacitko) {

    nactiVstupy(A14, A12, A15, A13);

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
        zpozdeni(20);
        break;
      }

      if (y == 1) {
        y = 0;
        x = 0;
        zpozdeni(20);
        lcd.setCursor(0, 1);
        lcd.write(" ");
        break;
      }
      if ((y == 3) && (x == 11)) {
        lcd.setCursor(11, 3);
        lcd.write(" ");
        y = 1;
        x = 0;
        zpozdeni(20);
        break;
      }
      if ((y == 3) && (x == 0)) {
        lcd.setCursor(0, 3);
        lcd.write(" ");
        y = 1;
        x = 0;
        zpozdeni(20);
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
        zpozdeni(20);
        break;
      }
      if (y == 3) {
        lcd.setCursor(x, y);
        lcd.write(" ");
        x = 0;
        y = 0;
        zpozdeni(20);
        break;
      }
      if (y == 0) {
        y = 1;
        x = 0;
        zpozdeni(20);
        int o = 0;
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
        zpozdeni(20);
        break;
      }
      if ((x == 18) && (y == 0)) {
        lcd.setCursor(18, 0);
        lcd.print(" ");
        x = 0;
        zpozdeni(20);
        break;
      }
      if ((x == 15) && (y == 0)) {
        lcd.setCursor(15, 0);
        lcd.print(" ");
        x = 18;
        zpozdeni(20);
        break;
      }
      if ((x == 12) && (y == 0)) {
        lcd.setCursor(12, 0);
        lcd.print(" ");
        x = 15;
        zpozdeni(20);
        break;
      }
      if ((x == 9) && (y == 0)) {
        lcd.setCursor(9, 0);
        lcd.print(" ");
        x = 12;
        zpozdeni(20);
        break;
      }
      if ((x == 6) && (y == 0)) {
        lcd.setCursor(6, 0);
        lcd.print(" ");
        x = 9;
        zpozdeni(20);
        break;
      }
      if ((x == 3) && (y == 0)) {
        lcd.setCursor(3, 0);
        lcd.print(" ");
        x = 6;
        zpozdeni(20);
        break;
      }
      if ((x == 0) && (y == 0)) {
        lcd.setCursor(0, 0);
        lcd.print(" ");
        x = 3;
        zpozdeni(20);
        break;
      }
      if ((x == 0) && (y == 3)) {
        lcd.setCursor(0, 3);
        lcd.print(" ");
        x = 11;
        zpozdeni(20);
        break;
      }
    }

    if (tlacitko == '<')
    {
      if ((x == 0) && (y == 3)) {
        lcd.setCursor(0, 3);
        lcd.print(" ");
        x = 11;
        zpozdeni(20);
        break;
      }
      if ((x == 0) && (y == 0)) {
        lcd.setCursor(0, 0);
        lcd.print(" ");
        x = 18;
        zpozdeni(20);
        break;
      }
      if ((x == 3) && (y == 0)) {
        lcd.setCursor(3, 0);
        lcd.print(" ");
        x = 0;
        zpozdeni(20);
        break;
      }
      if ((x == 6) && (y == 0)) {
        lcd.setCursor(6, 0);
        lcd.print(" ");
        x = 3;
        zpozdeni(20);
        break;
      }
      if ((x == 9) && (y == 0)) {
        lcd.setCursor(9, 0);
        lcd.print(" ");
        x = 6;
        zpozdeni(20);
        break;
      }
      if ((x == 12) && (y == 0)) {
        lcd.setCursor(12, 0);
        lcd.print(" ");
        x = 9;
        zpozdeni(20);
        break;
      }
      if ((x == 15) && (y == 0)) {
        lcd.setCursor(15, 0);
        lcd.print(" ");
        x = 12;
        zpozdeni(20);
        break;
      }
      if ((x == 18) && (y == 0)) {
        lcd.setCursor(18, 0);
        lcd.print(" ");
        x = 15;
        zpozdeni(20);
        break;
      }
      if ((x == 11) && (y == 3)) {
        lcd.setCursor(11, 3);
        lcd.print(" ");
        x = 0;
        zpozdeni(20);
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
      nastaveniParametru(limitdisplaye, nasthladiny , nastvlhkosti, nasthystereze);
    }
    if ((y == 3) && (x == 0)) {
      lcd.clear();
      nastavvystupy(limitdisplaye);
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Funkce s převodem z desítkové soustavy na BCD kod.
byte DECnaBCD(byte hodnota) {
  return ( (hodnota / 10 * 16) + (hodnota % 10) );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Funkce s převodem z BCD kodu do desitkove soustavy.
byte BCDnaDEC(byte hodnota) {
  return ( (hodnota / 16 * 10) + (hodnota % 16) );
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Trida::zapisEEPROM() {
  if ((nasthladiny >= 0) && (nasthladiny <= 100)) EEPROM.write(0, nasthladiny);
  if ((nastvlhkosti >= 0) && (nastvlhkosti <= 100)) EEPROM.write(1, nastvlhkosti);
  if ((limitdisplaye >= 0) && (limitdisplaye <= 250)) EEPROM.write(2, limitdisplaye);                        // zápis proměnných do eeprom!
  if ((nedele2 >= 0) && (nedele2 <= 9)) EEPROM.write(3, nedele2);
  if ((soucasnyBeh >= 0) && (soucasnyBeh <= 1)) EEPROM.write(4, soucasnyBeh);
  if ((nasthystereze >= 0) && (nasthystereze <= 10)) EEPROM.write(5, nasthystereze);

  int c = 6;
  for (int a = 0; a < 7; a++) {
    if ((dny[a].hodiny1 >= 0) && (dny[a].hodiny1 <= 2)) EEPROM.write(c, dny[a].hodiny1);
    c++;
    if ((dny[a].hodiny2 >= 0) && (dny[a].hodiny2 <= 9)) EEPROM.write(c, dny[a].hodiny2);                      // zápis struktury do eeprom!
    c++;
    if ((dny[a].hodinycelk >= 0) && (dny[a].hodinycelk <= 23)) EEPROM.write(c, dny[a].hodinycelk);
    c++;
    if ((dny[a].minuty1 >= 0) && (dny[a].minuty1 <= 5)) EEPROM.write(c, dny[a].minuty1);
    c++;
    if ((dny[a].minuty2 >= 0) && (dny[a].minuty2 <= 9)) EEPROM.write(c, dny[a].minuty2);
    c++;
    if ((dny[a].minutycelk >= 0) && (dny[a].minutycelk <= 59)) EEPROM.write(c, dny[a].minutycelk);
    c++;
    if ((dny[a].min1 >= 0) && (dny[a].min1 <= 9)) EEPROM.write(c, dny[a].min1);
    c++;
    if ((dny[a].min10 >= 0) && (dny[a].min10 <= 9)) EEPROM.write(c, dny[a].min10);
    c++;
    if ((dny[a].min1celk >= 0) && (dny[a].min1celk <= 99)) EEPROM.write(c, dny[a].min1celk);
    c++;
    if ((dny[a].min2 >= 0) && (dny[a].min2 <= 9)) EEPROM.write(c, dny[a].min2);
    c++;
    if (a != 6) {
      if ((dny[a].min20 >= 0) && (dny[a].min20 <= 9)) EEPROM.write(c, dny[a].min20);
      c++;
    }
    if ((dny[a].min2celk >= 0) && (dny[a].min2celk <= 99)) EEPROM.write(c, dny[a].min2celk);
    c++;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Trida::prectiEEPROM() {
  nasthladiny = EEPROM.read(0);      // čtení z EEPROM
  if (nasthladiny > 100) nasthladiny = 0;

  nastvlhkosti = EEPROM.read(1);
  if (nastvlhkosti > 100) nastvlhkosti = 0;

  nasthystereze = EEPROM.read(5);
  if (nasthystereze > 10) nasthystereze = 0;

  limitdisplaye = EEPROM.read(2);
  if (limitdisplaye > 250) limitdisplaye = 0;

  nedele2 = EEPROM.read(3);
  if (nedele2 > 9) nedele2 = 0;

  soucasnyBeh = EEPROM.read(4);
  if (soucasnyBeh > 1) soucasnyBeh = 0;


  int c = 6;
  for (int a = 0; a < 7; a++) {
    dny[a].hodiny1 = EEPROM.read(c);
    if ((dny[a].hodiny1 < 0 ) || (dny[a].hodiny1 > 2)) dny[a].hodiny1 = 0;

    c++;
    dny[a].hodiny2 = EEPROM.read(c);
    if ((dny[a].hodiny2 < 0 ) || (dny[a].hodiny2 > 9)) dny[a].hodiny2 = 0;

    c++;
    dny[a].hodinycelk = EEPROM.read(c);
    if ((dny[a].hodinycelk < 0 ) || (dny[a].hodinycelk > 23)) {
      dny[a].hodiny1 = 0;
      dny[a].hodiny2 = 0;
      dny[a].hodinycelk = 0;
    }

    c++;
    dny[a].minuty1 = EEPROM.read(c);
    if ((dny[a].minuty1 < 0 ) || (dny[a].minuty1 > 5)) dny[a].minuty1 = 0;

    c++;
    dny[a].minuty2 = EEPROM.read(c);
    if ((dny[a].minuty2 < 0 ) || (dny[a].minuty2 > 9)) dny[a].minuty2 = 0;

    c++;
    dny[a].minutycelk = EEPROM.read(c);
    if ((dny[a].minutycelk < 0 ) || (dny[a].minutycelk > 59)) dny[a].minutycelk = 0;

    c++;
    dny[a].min1 = EEPROM.read(c);
    if ((dny[a].min1 < 0 ) || (dny[a].min1 > 9)) dny[a].min1 = 0;

    c++;
    dny[a].min10 = EEPROM.read(c);
    if ((dny[a].min10 < 0 ) || (dny[a].min10 > 9)) dny[a].min10 = 0;

    c++;
    dny[a].min1celk = EEPROM.read(c);
    if ((dny[a].min1celk < 0 ) || (dny[a].min1celk > 99)) dny[a].min1celk = 0;

    c++;
    dny[a].min2 = EEPROM.read(c);
    if ((dny[a].min2 < 0 ) || (dny[a].min2 > 9)) dny[a].min2 = 0;

    c++;
    if (a != 6) {
      dny[a].min20 = EEPROM.read(c);
      if ((dny[a].min20 < 0 ) || (dny[a].min20 > 9)) dny[a].min20 = 0;
      c++;
    }

    dny[a].min2celk = EEPROM.read(c);
    if ((dny[a].min2celk < 0 ) || (dny[a].min2celk > 99)) dny[a].min2celk = 0;
    c++;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Trida::zobrazNaDisplayi() {

  lcd.setCursor(1, 2);
  lcd.print("MV=");
  lcd.print(AN2);
  lcd.print("%");
  if (AN2 < 10) {
    lcd.setCursor(6, 2);
    lcd.print(" ");
  }
  lcd.setCursor(7, 2);
  if ((AN2 - nasthystereze) < nastvlhkosti) {
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
  if (AN1 > (nasthladiny - nasthystereze)) {
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
  if (((nastavvystupy1 == true) || ((soucasnyBehVystupy == 2) && ((kteryVystup == 0) || (kteryVystup == 1))) || (soucasnyBehVystupy == 1) || (nastavvystupy2 == true)) || ((nastav_vystup[0] == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1)) || ((nastav_vystup[0] == 2) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2))) {
    lcd.write(byte(1));
    digitalWrite(rele[0], HIGH);

    nactiVstupy(A14, A12, A15, A13);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele[0], LOW);
  }
  lcd.print("2");
  if ((nastav_vystup[0] == 1) || ((soucasnyBehVystupy == 2) && kteryVystup == 0) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1) || (nastavvystupy1 == true)) {
    lcd.write(byte(1));
    digitalWrite(rele[1], HIGH);
  }
  else {
    lcd.write(byte(2));
    digitalWrite(rele[1], LOW);
  }
  lcd.print("3");
  if ((nastav_vystup[0] == 2) || ((soucasnyBehVystupy == 2) && kteryVystup == 1) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2) || (nastavvystupy2 == true)) {
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
    lcd.write(byte(3));               //blikání kurzoru
  }

  if ((millis() > cekej01 + 1500) && (millis() < cekej01 + 1600)) {
    lcd.setCursor(x, y);
    lcd.write(" ");                   //blikání kurzoru
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

// funkce pro zápis do obvodu reálného času přes Wire.h
void zapisORC(byte sekundy, byte minuty, byte hodiny, byte denVTydnu, byte denVMesici) {
  Wire.beginTransmission(ORC_I2C_ADRESA);
  Wire.write(0);
  if ((sekundy >= 0) && (sekundy < 60)) Wire.write(DECnaBCD(sekundy));
  if ((minuty >= 0) && (minuty < 60)) Wire.write(DECnaBCD(minuty));
  if ((hodiny >= 0) && (hodiny < 24)) Wire.write(DECnaBCD(hodiny));
  if ((denVTydnu > 0) && (denVTydnu < 8)) Wire.write(DECnaBCD(denVTydnu));
  if ((denVMesici >= 0) && (denVMesici < 32)) Wire.write(DECnaBCD(denVMesici));
  Wire.endTransmission();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// funkce pro čtení z obvodu reálného času přes Wire.h
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

void Trida::ethernet() {

  // dostupnost klienta
  EthernetClient client = server.available();
  String buffer = "";

  // dokud je klient připojen
  while (client.connected()) {

    // čti data dokud nenarazíš na znak nového řádku
    if (client.available()) {
      char c = client.read();
      buffer = buffer + c;

      if (c == '\n') {

        // CO MAJI UDELAT TALCITKA ---------------------------------------------------------------------------------------------
        if (buffer.indexOf("out2=1") >= 0)
          nastavvystupy1 = true;
        if (buffer.indexOf("out2=0") >= 0)
          nastavvystupy1 = false;                     // VÝSTUPY
        if (buffer.indexOf("out3=1") >= 0)
          nastavvystupy2 = true;
        if (buffer.indexOf("out3=0") >= 0)
          nastavvystupy2 = false;

        if (buffer.indexOf("soucBeh=0") >= 0) {
          soucasnyBeh = false;
          zapisEEPROM();                             // SOUČASNÝ BĚH
        }

        if (buffer.indexOf("soucBeh=1") >= 0) {
          soucasnyBeh = true;
          zapisEEPROM();
        }

        if (buffer.indexOf("menu=1") >= 0)
          Menu = 1;
        if (buffer.indexOf("menu=2") >= 0)
          Menu = 2;
        if (buffer.indexOf("menu=3") >= 0)          // POHYB V MENU
          Menu = 3;
        if (buffer.indexOf("menu=4") >= 0)
          Menu = 4;
        if (buffer.indexOf("menu=5") >= 0)
          Menu = 5;
        if (buffer.indexOf("menu=6") >= 0)
          Menu = 6;
        if (buffer.indexOf("menu=7") >= 0)
          Menu = 7;
        if (buffer.indexOf("menu=8") >= 0)
          Menu = 8;
        if (buffer.indexOf("menu=9") >= 0)
          Menu = 9;
        if (buffer.indexOf("menu=10") >= 0)
          Menu = 10;
        if (buffer.indexOf("menu=11") >= 0)
          Menu = 11;
        if (buffer.indexOf("menu=12") >= 0)
          Menu = 12;

        //-----------------------------------------------------------------------------------------------------------------------

        client.println("<!DOCTYPE HTML>");

        client.println("<html>");
        client.println("<head>");

        client.println("<meta charset='UTF-8'>");   // diakritika českeho jazyka

        client.print("<a href='http://192.168.10.145?menu=14'><button style='background:black;width:100%;height:80px'><font color='gold'><marquee direction='left'><h1>Ethernetové rozhraní pro zavlažovací systém, Kočica Filip ME4</h1></marquee></font></button></a>");
        client.print("<a href='http://192.168.10.145?menu=6'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>PONDĚLÍ</h1></font></button></a>");       //po
        client.print("<a href='http://192.168.10.145?menu=7'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>ÚTERÝ</h1></font></button></a>");         //ut
        client.print("<a href='http://192.168.10.145?menu=8'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>STŘEDA</h1></font></button></a>");        //st
        client.print("<a href='http://192.168.10.145?menu=9'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>ČTVRTEK</h1></font></button></a>");       //ct
        client.print("<a href='http://192.168.10.145?menu=10'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>PÁTEK</h1></font></button></a>");        //pa
        client.print("<a href='http://192.168.10.145?menu=11'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>SOBOTA</h1></font></button></a>");       //so
        client.print("<a href='http://192.168.10.145?menu=12'><button style='background:blue;width:14.285714%;height:80px'><font color='gold'><h1>NEDĚLE</h1></font></button></a>");       //ne
        client.print("<a href='http://192.168.10.145?menu=2'><button style='background:purple;width:100%;height:80px'><font color='silver'><h1>Nastavení parametrů</h1></font></button></a>");     //nastav parametru
        client.print("<a href='http://192.168.10.145?menu=3'><button style='background:purple;width:80%;height:80px'><font color='silver'><h1>Aktualní hodnoty vstupů</h1></font></button></a>");  //aktual hodnoty vstupu
        if (Binarnivstup1)
          client.print("<a href='http://192.168.10.145?menu=13'><button style='background:green;width:20%;height:80px'><h1>Porucha rozepnuta</h1></button></a>");                                  //porucha
        else
          client.print("<a href='http://192.168.10.145?menu=13'><button style='background:red;width:20%;height:80px'><h1>Porucha sepnuta</h1></button></a>");                                      //porucha
        client.print("<a href='http://192.168.10.145?menu=4'><button style='background:purple;width:60%;height:80px'><font color='silver'><h1>Výstupy</h1></font></button></a>");                  //vystupy
        client.print("<a href='http://192.168.10.145?menu=5'><button style='background:purple;width:40%;height:80px'><font color='silver'><h1>Aktualní čas na Arduinu</h1></font></button></a>");  //aktual cas na ardu

        client.println("<title>* Ethernetové rozhraní pro zavlažovací systém, Kočica Filip ME4 *</title>");
        client.println("<meta http-equiv='refresh' content='60' >");   // refresh každou minutu

        client.println("</head>");
        client.println("<body>");

        client.println("<br />");
        client.println("<h1>");
        client.println("<center>");

        if ((Menu == 6) || (Menu == 7) || (Menu == 8) || (Menu == 9) || (Menu == 10) || (Menu == 11) || (Menu == 12)) {


          //---------------------VYPSANI PO-NE NASTAVENYCH CASU !!!!----------------------------------------------------------------
          int vypisDniNaSit = Menu - 6;

          switch (vypisDniNaSit) {
            case 0:
              client.println("Pondělí ");
              break;
            case 1:
              client.println("Úterý ");
              break;
            case 2:
              client.println("Středa ");
              break;
            case 3:
              client.println("Čtvrtek ");
              break;
            case 4:
              client.println("Pátek ");
              break;
            case 5:
              client.println("Sobota ");
              break;
            case 6:
              client.println("Neděle ");
              break;
          }

          client.print("<FORM action='http://192.168.10.145' method='GET'>");
          client.println("</h1>");
          client.println("<center>");
          client.println(dny[vypisDniNaSit].hodiny1);
          client.println(dny[vypisDniNaSit].hodiny2);
          client.println(":");
          client.println(dny[vypisDniNaSit].minuty1);
          client.println(dny[vypisDniNaSit].minuty2);
          client.println("  -  ");
          client.println("<input type='number' style='background:yellow;width:30px' name='q1' min='0' max='23'>");
          client.println(":");
          client.println("<input type='number' style='background:yellow;width:30px' name='q2' min='0' max='59'>");
          client.println("<br />");
          client.println("<h1>");
          client.println("1.Okruh je nastaven na: (minut)");
          client.println("</h1>");
          client.println(dny[vypisDniNaSit].min1);
          client.println(dny[vypisDniNaSit].min10);
          client.println("  -  ");
          client.println("<input type='number' style='background:yellow;width:30px' name='q3' min='0' max='99'>");
          client.println("<br />");
          client.println("<h1>");
          client.println("2.Okruh je nastaven na: (minut)");
          client.println("</h1>");
          client.println(dny[vypisDniNaSit].min2);
          if (vypisDniNaSit == 6) client.println(nedele2);
          else client.println(dny[vypisDniNaSit].min20);
          client.println("  -  ");
          client.println("<input type='number' style='background:yellow;width:30px' name='q4' min='0' max='99'>");
          client.println("<br />");
          client.println("<h1>");
          client.println("Současný Běh je ");
          if (soucasnyBeh) {
            client.print("<font color='green'>ZAPNUTÝ</font>");
            client.print("<FORM action='http://192.168.10.145' method='GET'>");
            client.print("<P> <INPUT type='radio' name='soucBeh' value='0'>VYPNOUT");
            client.print("<P> <INPUT type='submit' style='background:red;width:100px;height:50px'> </FORM>");
          } else {
            client.print("<font color='red'>VYPNUTÝ</font>");
            client.print("<FORM action='http://192.168.10.145' method='GET'>");
            client.print("<P> <INPUT type='radio' name='soucBeh' value='1'>ZAPNOUT");
            client.print("<P> <INPUT type='submit' style='background:lightgreen;width:100px;height:50px'> </FORM>");
          }


          for (int v = 0; v < 50; v++) {

            //----------------------------------------------------------------------------------------------------------------------------------------------------------------

            if ((buffer[v] == 'q') && (buffer[v + 1] == '1') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // hodiny 2
              dny[vypisDniNaSit].hodiny1 = 0;
              dny[vypisDniNaSit].hodiny2 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].hodinycelk = ((int)buffer[v + 3] - 48);
              zapisEEPROM();
              refresh[1] = true;
            }

            if ((buffer[v] == 'q') && (buffer[v + 1] == '1') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // hodiny 1 + 2
              dny[vypisDniNaSit].hodiny1 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].hodiny2 = ((int)buffer[v + 4] - 48);
              dny[vypisDniNaSit].hodinycelk = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
              zapisEEPROM();
              refresh[1] = true;
            }

            //----------------------------------------------------------------------------------------------------------------------------------------------------------------

            if ((buffer[v] == 'q') && (buffer[v + 1] == '2') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // minuty 2
              dny[vypisDniNaSit].minuty1 = 0;
              dny[vypisDniNaSit].minuty2 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].minutycelk = ((int)buffer[v + 3] - 48);
              zapisEEPROM();
              refresh[1] = true;
            }

            if ((buffer[v] == 'q') && (buffer[v + 1] == '2') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // minuty 1 + 2
              dny[vypisDniNaSit].minuty1 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].minuty2 = ((int)buffer[v + 4] - 48);
              dny[vypisDniNaSit].minutycelk = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
              refresh[1] = true;
            }

            //----------------------------------------------------------------------------------------------------------------------------------------------------------------

            if ((buffer[v] == 'q') && (buffer[v + 1] == '3') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // okruh 10
              dny[vypisDniNaSit].min1 = 0;
              dny[vypisDniNaSit].min10 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].min1celk = ((int)buffer[v + 3] - 48);
              zapisEEPROM();
              refresh[1] = true;
            }

            if ((buffer[v] == 'q') && (buffer[v + 1] == '3') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // okruh 1 + 10
              dny[vypisDniNaSit].min1 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].min10 = ((int)buffer[v + 4] - 48);
              dny[vypisDniNaSit].min1celk = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
              zapisEEPROM();
              refresh[1] = true;
            }

            //----------------------------------------------------------------------------------------------------------------------------------------------------------------

            if ((buffer[v] == 'q') && (buffer[v + 1] == '4') && (buffer[v + 2] == '=') && (buffer[v + 3] != ' ')) {                             // okruh 20
              dny[vypisDniNaSit].min2 = 0;
              if (vypisDniNaSit == 6) nedele2 = ((int)buffer[v + 3] - 48);
              else dny[vypisDniNaSit].min20 = ((int)buffer[v + 3] - 48);
              dny[vypisDniNaSit].min2celk = ((int)buffer[v + 3] - 48);
              zapisEEPROM();
              refresh[1] = true;
            }

            if ((buffer[v] == 'q') && (buffer[v + 1] == '4') && (buffer[v + 2] == '=') && (buffer[v + 3] != ' ') && (buffer[v + 4] != ' ')) {   // okruh 2 + 20
              dny[vypisDniNaSit].min2 = ((int)buffer[v + 3] - 48);
              if (vypisDniNaSit == 6) nedele2 = ((int)buffer[v + 4] - 48);
              else dny[vypisDniNaSit].min20 = ((int)buffer[v + 4] - 48);
              dny[vypisDniNaSit].min2celk = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
              zapisEEPROM();
              refresh[1] = true;
            }

            //----------------------------------------------------------------------------------------------------------------------------------------------------------------
          }

          client.println("<br />");
          client.println("<br />");
          //---------------------VYPSANI PO-NE NASTAVENYCH CASU !!!!----------------------------------------------------------------
        }

        switch (Menu) {
          case 2:
            client.print("<FORM action='http://192.168.10.145' method='GET'>");
            client.print("Nastavit Vlhkost = (max 100%)");
            client.println("</h1>");
            client.println("<center>");
            client.print(nastvlhkosti);
            client.println("  -  ");
            client.println("<input type='number' style='background:yellow;width:30px' name='q5' min='0' max='99'>");
            client.println("<br />");
            client.println("<br />");
            client.println("<h1>");
            client.print("Nastavit Hladinu = (max 100%)");
            client.println("</h1>");
            client.print(nasthladiny);
            client.println("  -  ");
            client.println("<input type='number' style='background:yellow;width:30px' name='q6' min='0' max='99'>");
            client.println("<br />");
            client.println("<br />");
            client.println("<h1>");
            client.print("Nastavit Hysterezi = (max 10 %)");
            client.println("</h1>");
            client.print(nasthystereze);
            client.println("  -  ");
            client.println("<input type='number' style='background:yellow;width:30px' name='q7' min='0' max='10'>");
            client.println("<br />");
            client.println("<br />");
            client.println("<h1>");
            client.print("Nastavit Limit Displaye = (max 250s)");
            client.println("</h1>");
            client.print(limitdisplaye);
            client.println("  -  ");
            client.println("<input type='number' style='background:yellow;width:38px' name='q8' min='0' max='250'>");
            client.println("<br />");
            client.print("<P> <INPUT type='submit' style='background:silver;width:100px;height:50px'> </FORM>");
            client.println("<br />");

            for (int v = 0; v < 50; v++) {

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '5') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // nast vlhkosti 1
                nastvlhkosti = ((int)buffer[v + 3] - 48);
                zapisEEPROM();
                refresh[2] = true;
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '5') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // nast vlhkosti 1 + 2
                nastvlhkosti = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisEEPROM();
                refresh[2] = true;
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '6') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // nast hladiny 1
                nasthladiny = ((int)buffer[v + 3] - 48);
                zapisEEPROM();
                refresh[2] = true;
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '6') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // nast hladiny 1 + 2
                nasthladiny = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisEEPROM();
                refresh[2] = true;
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '7') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // nast hystereze 1
                nasthystereze = ((int)buffer[v + 3] - 48);
                zapisEEPROM();
                refresh[2] = true;
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '7') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // nast hystereze 1 + 2
                nasthystereze = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisEEPROM();
                refresh[2] = true;
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '8') && (buffer[v + 2] == '=') && (buffer[v + 3] != ' ') && (buffer[v + 4] == ' ')) {   // nast limitu displaye 1
                limitdisplaye = ((int)buffer[v + 3] - 48);
                zapisEEPROM();
                refresh[2] = true;
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '8') && (buffer[v + 2] == '=') && (buffer[v + 3] != ' ') && (buffer[v + 4] != '&') && (buffer[v + 5] == ' ')) {   // nast limitu displaye 1 + 2
                limitdisplaye = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisEEPROM();
                refresh[2] = true;
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '8') && (buffer[v + 2] == '=') && (buffer[v + 3] != ' ') && (buffer[v + 4] != ' ') && (buffer[v + 5] != ' ') && (buffer[v + 6] == ' ')) {   // nast limitu displaye 1 + 2 + 3
                limitdisplaye = ((100 * ((int)buffer[v + 3] - 48)) + (10 * ((int)buffer[v + 4] - 48)) + ((int)buffer[v + 5] - 48));
                zapisEEPROM();
                refresh[2] = true;
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------
            }
            break;

          case 3:

            //------------------------ zobrazeni vlhkosti a hladiny a jestli je podminka splnena nebo ne -----------------------------
            client.print("Nastavená Vlhkost: ");
            client.print(nastvlhkosti);
            client.print("%");
            client.println("<br />");

            client.print("Naměřená Vlhkost: ");
            client.print(AN2);
            client.print("%");
            client.println("<br />");

            if ((AN2 - nasthystereze) < nastvlhkosti) {
              client.print("<font color='green'>PODMÍNKA JE SPLNĚNA</font>");
            } else {
              client.print("<font color='red'>PODMÍNKA NENÍ SPLNĚNA</font>");
            }

            client.println("<br />");
            client.println("<br />");

            client.print("Nastavená Hladina: ");
            client.print(nasthladiny);
            client.print("%");
            client.println("<br />");

            client.print("Naměřená Hladina: ");
            client.print(AN1);
            client.print("%");
            client.println("<br />");

            if (AN1 > (nasthladiny - nasthystereze)) {
              client.print("<font color='green'>PODMÍNKA JE SPLNĚNA</font>");
            } else {
              client.print("<font color='red'>PODMÍNKA NENÍ SPLNĚNA</font>");
            }

            client.println("<br />");
            client.println("<br />");

            client.print("Nastavená Hystereze: ");
            client.print(nasthystereze);
            client.println("<br />");

            client.print("Nastavený Limit Displaye: ");
            client.print(limitdisplaye);
            client.print(" sekund");
            client.println("<br />");
            //------------------------- zobrazeni vlhkosti a hladiny a jestli je podminka splnena nebo ne -----------------------------

            break;

          case 4:

            // ------------ VYSTUPY ---------------------------------------------------------------------------------------
            client.print("OUT 1 ");
            if (((nastavvystupy1 == true) || ((soucasnyBehVystupy == 2) && ((kteryVystup == 0) || (kteryVystup == 1))) || (soucasnyBehVystupy == 1) || (nastavvystupy2 == true)) || ((nastav_vystup[0] == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1)) || ((nastav_vystup[0] == 2) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2)))
              client.print("je <font color='green'>ZAPNUTÝ</font>");
            else
              client.print("je <font color='red'>VYPNUTÝ</font>");
            client.println("<br />");
            client.println("<br />");

            client.print("OUT 2 ");
            if ((nastav_vystup[0] == 1) || ((soucasnyBehVystupy == 2) && kteryVystup == 0) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 1) || (nastav_vystup[2] == 1) || (nastav_vystup[3] == 1) || (nastav_vystup[4] == 1) || (nastav_vystup[5] == 1) || (nastav_vystup[6] == 1) || (nastavvystupy1 == true))
              client.print("je <font color='green'>ZAPNUTÝ</font>");
            else
              client.print("je <font color='red'>VYPNUTÝ</font>");

            client.println("<br />");
            client.print("<FORM action='http://192.168.10.145' method='GET'>");
            client.print("<P> <INPUT type='radio' name='out2' value='1'>ON");
            client.print("<P> <INPUT type='radio' name='out2' value='0'>OFF");
            if (nastavvystupy1)
              client.print("<P> <INPUT type='submit' style='background:red;width:100px;height:50px'> </FORM>");
            else
              client.print("<P> <INPUT type='submit' style='background:lightgreen;width:100px;height:50px'> </FORM>");

            client.print("OUT 3 ");
            if ((nastav_vystup[0] == 2) || ((soucasnyBehVystupy == 2) && kteryVystup == 1) || (soucasnyBehVystupy == 1) || (nastav_vystup[1] == 2) || (nastav_vystup[2] == 2) || (nastav_vystup[3] == 2) || (nastav_vystup[4] == 2) || (nastav_vystup[5] == 2) || (nastav_vystup[6] == 2) || (nastavvystupy2 == true))
              client.print("je <font color='green'>ZAPNUTÝ</font>");
            else
              client.print("je <font color='red'>VYPNUTÝ</font>");
            client.println("<br />");
            client.print("<FORM action='http://192.168.10.145' method='GET'>");
            client.print("<P> <INPUT type='radio' name='out3' value='1'>ON");
            client.print("<P> <INPUT type='radio' name='out3' value='0'>OFF");
            if (nastavvystupy2)
              client.print("<P> <INPUT type='submit' style='background:red;width:100px;height:50px'> </FORM>");
            else
              client.print("<P> <INPUT type='submit' style='background:lightgreen;width:100px;height:50px'> </FORM>");
            //--------------------------------------------------------------------------------------------------------------

            break;

          case 5:

            //---------AKTUALNI CAS ----------------------------------------------------------------------------------------

            cteniORC(&sekundy, &minuty, &hodiny, &denVTydnu, &denVMesici);
            client.print("Aktuální čas nastavený na arduinu je ");

            switch (denVTydnu) {
              case 1:
                client.println("Pondělí, ");
                break;
              case 2:
                client.println("Úterý, ");
                break;
              case 3:
                client.println("Středa, ");
                break;
              case 4:
                client.println("Čtvrtek, ");
                break;
              case 5:
                client.println("Pátek, ");
                break;
              case 6:
                client.println("Sobota, ");
                break;
              case 7:
                client.println("Neděle, ");
                break;
            }

            client.println(hodiny);
            client.println(":");
            client.println(minuty);
            client.println(":");
            client.println(sekundy);
            client.println(".");
            client.println("<br />");
            client.println("<h1>");

            client.print("Nastavení:   ");
            client.println("</h1>");
            client.println("<h3>");

            client.print("<FORM action='http://192.168.10.145' method='GET'>");

            client.println("<input type='number' style='background:yellow;width:30px' name='q9' min='0' max='23'>");
            client.println("  :  ");
            client.println("<input type='number' style='background:yellow;width:30px' name='q0' min='0' max='59'>");

            // --------------------------------------------------------------------------------

            client.print("<P> <INPUT type='radio' name='denVTydnu' value='1'>Pondělí");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='2'>Úterý");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='3'>Středa");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='4'>Čtvrtek");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='5'>Pátek");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='6'>Sobota");
            client.print("<P> <INPUT type='radio' name='denVTydnu' value='7'>Neděle");

            // --------------------------------------------------------------------------------

            client.print("<P> <INPUT type='submit' style='background:silver;width:100px;height:50px'> </FORM>");

            for (int v = 0; v < 50; v++) {

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '9') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] == '&')) {   // nast hodiny 1
                nastavORC[5] = ((int)buffer[v + 3] - 48);
                zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '9') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // nast hodiny 1 + 2
                nastavORC[5] = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'q') && (buffer[v + 1] == '0') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && ((buffer[v + 4] == '&') || (buffer[v + 4] == ' '))) {   // nast minuty 1
                nastavORC[6] = ((int)buffer[v + 3] - 48);
                zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
              }

              if ((buffer[v] == 'q') && (buffer[v + 1] == '0') && (buffer[v + 2] == '=') && (buffer[v + 3] != '&') && (buffer[v + 4] != '&')) {   // nast minuty 1 + 2
                nastavORC[6] = ((10 * ((int)buffer[v + 3] - 48)) + ((int)buffer[v + 4] - 48));
                zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

              if ((buffer[v] == 'd') && (buffer[v + 1] == 'n') && (buffer[v + 2] == 'u') && (buffer[v + 3] == '=') && (buffer[v + 4] != ' ')) {   // nast dne
                cteniORC(&sekundy, &minuty, &hodiny, &denVTydnu, &denVMesici);
                nastavORC[6] = minuty;
                nastavORC[5] = hodiny;
                nastavORC[7] = ((int)buffer[v + 4] - 48);
                zapisORC(00, nastavORC[6], nastavORC[5], nastavORC[7], 12);
              }

              //----------------------------------------------------------------------------------------------------------------------------------------------------------------

            }
            client.println("</h3>");

            //--------------------------------------------------------------------------------------------------------------

            break;
        }

        client.println("</body>");
        client.println("</html>");
        client.stop();
      }
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void zpozdeni(int doba) {

  unsigned long aktual;
  aktual = millis();
  aktual += doba;

  while (millis() < aktual) {
    char tlacitko = stisknutiTlacitka.getKey();
    if ((!Binarnivstup1) || (tlacitko == 'C')) break;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Trida::prace_s_vystupy() {

  for (i = 0; i < 7; i++) {
    if ((nastav_vystup[i] == 1) || (nastav_vystup[i] == 2)) {
      nastavvystupy1 = false;
      nastavvystupy2 = false;
    }
  }

  if ((AN2 > (nastvlhkosti + nasthystereze)) || ((AN1 + nasthystereze) < nasthladiny)) {
    for (i = 0; i <= 6; i++) {
      nastav_vystup[i] = 0;
    }
    soucasnyBehVystupy = 0;
  }

  for (i = 0; i <= 6; i++) {
    if ((hodiny == dny[i].hodinycelk) && (minuty == (dny[i].minutycelk - 1)) && (denVTydnu == (i + 1))) {     //nacteni vstupu minutu pred spuštěním výstupů
      if (nacitaniVstupu == true) {
        nactiVstupy(A14, A12, A15, A13);
      }
    }
  }

  if (!soucasnyBeh) {
    for (i = 0; i <= 6; i++) {

      if ((hodiny == dny[i].hodinycelk) && (minuty == dny[i].minutycelk) && (nastav_vystup[i] == 0) && ((AN2 - nasthystereze) < nastvlhkosti) && (AN1 > (nasthladiny - nasthystereze)) && (denVTydnu == (i + 1)) && ((dny[0].min1celk > 0) || (dny[0].min2celk > 0))) {
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
      if ((hodiny == dny[i].hodinycelk) && (minuty == dny[i].minutycelk) && (!soucasnyBehVystupy) && ((AN2 - nasthystereze) < nastvlhkosti) && (AN1 > (nasthladiny - nasthystereze)) && (denVTydnu == (i + 1)) && ((dny[0].min1celk > 0) || (dny[0].min2celk > 0))) {
        nacitaniVstupu = true;
        if (!soucasnyBehVystupy) {
          cas_vystupu = millis() / 1000;
          soucasnyBehVystupy = 1;
        }
        identifik = i;
      }

      if ((soucasnyBehVystupy == 1) && (i == identifik)) {
        if ((millis() / 1000) > (cas_vystupu + oba_vystupy)) {
          soucasnyBehVystupy = 2;
          cas_vystupu = millis() / 1000;
        }
      }
      if ((soucasnyBehVystupy == 2) && (i == identifik)) {
        if ((millis() / 1000) > (cas_vystupu + druhy_vystup)) {
          soucasnyBehVystupy = 0;
          cas_vystupu = millis() / 1000;
        }
      }

      if ((dny[i].min1celk * 60) > (dny[i].min2celk * 60) && (i == identifik)) {
        druhy_vystup = (dny[i].min1celk * 60) - (dny[i].min2celk * 60);
        oba_vystupy = (dny[i].min2celk * 60);
        kteryVystup = 0;
      }
      else if ((dny[i].min2celk * 60) > (dny[i].min1celk * 60) && (i == identifik)) {
        druhy_vystup = (dny[i].min2celk * 60) - (dny[i].min1celk * 60);
        oba_vystupy = (dny[i].min1celk * 60);
        kteryVystup = 1;
      }
      else if ((dny[i].min2celk * 60) == (dny[i].min1celk * 60) && (i == identifik)) {
        druhy_vystup = 0;
        oba_vystupy = (dny[i].min1celk * 60);
        kteryVystup = 2;
      }
    }
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void nactiVstupy(int analogvstup1, int analogvstup2, int cteniVstupu1, int cteniVstupu2) {
  pinMode(analogvstup1, INPUT);
  pinMode(analogvstup2, INPUT);                 //Analogove vstupy
  pinMode(cteniVstupu1, OUTPUT);                //Tyto výstupy se přepnou do log. "1" pouze při čtení vstupů,pak se nastaví zpět na log. "0".
  pinMode(cteniVstupu2, OUTPUT);
  digitalWrite(cteniVstupu2, HIGH);
  digitalWrite(cteniVstupu1, HIGH);                                             // na snímače přivede +5V

  AN1 = analogRead(analogvstup1);
  AN1 = map(AN1, 0, 1023, 0, 100);                                              // přečte hodnotu

  AN2 = analogRead(analogvstup2);
  AN2 = map(AN2, 0, 1023, 0, 100);

  digitalWrite(cteniVstupu2, LOW);                                              // a zase přivede 0V
  digitalWrite(cteniVstupu1, LOW);                                              // ošetření proti opotřebení snímačů
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

void Trida::limit() {
  if  (zhasnuti >= limitdisplaye) lcd.noBacklight();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Trida::Trida() {    //KONSTRUKTOR TŘÍDY
  prectiEEPROM();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Trida::~Trida() {   //DESTRUKTOR TŘÍDY

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


