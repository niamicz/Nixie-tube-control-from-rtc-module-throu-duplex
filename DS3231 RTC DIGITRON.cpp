#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68




// Definice pinů
const int outputPinSec = 9;     // Výstup pro čítač sekund a desítky sekund
const int outputPinMin = 8;     // Výstup pro čítač minut a desítky minut
const int outputPinHour = 7;    // Výstup pro čítač hodin a desítky hodin
const int outputPinnull = 5;    // Výstup pro nulování čítače
const int duplexPin1 = 13;       // První duplexní pin (přepínání mezi sec/min a hour)
const int duplexPin2 = 12;       // Druhý duplexní pin (antiparalelně)
const unsigned int duplexInterval = 4; // Interval přepínání duplex režimu (5 ms)
const unsigned long blinkInterval = 60000; // Interval pro probliknutí všech hodnot
unsigned long previousMillis = 0;           // Pro časování zobrazení času
const long interval = 1000;                 // Interval 1 sekundy pro displayTime
unsigned long previousDuplexMillis = 0;     // Pro časování přepnutí duplex režimu
unsigned long lastBlinkMillis = 0;          // Pro časování volání probliknutí




bool duplex = false;       // Přepínání mezi sekundami a minutami
bool hourDuplex = false;   // Přepínání mezi hodinami a desítkami hodin




// Převod normálních čísel na BCD a Převod BCD na normální desítkové číslo
// Převod normálních čísel na BCD
byte decToBcd(byte val) {
   return ((val / 10 * 16) + (val % 10));
}


// Převod BCD na normální desítkové číslo
byte bcdToDec(byte val) {
   return ((val / 16 * 10) + (val % 16));
}








void setup() {
   pinMode(outputPinSec, OUTPUT);
   pinMode(outputPinMin, OUTPUT);
   pinMode(outputPinHour, OUTPUT);
   pinMode(outputPinnull, OUTPUT);
   pinMode(duplexPin1, OUTPUT); 
   pinMode(duplexPin2, OUTPUT);
   //setDS3231time(20,29,18,3,12,11,24);       // To že to je komentář neznamená že by to tu nemělo být je to volání void setDS3231time přes void setup
   Wire.begin();
   Serial.begin(9600);




}




// Funkce pro nastavení času v DS3231
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




// Funkce pro čtení času z DS3231
void readDS3231time(byte &second, byte &minute, byte &hour, byte &dayOfWeek, byte &dayOfMonth, byte &month, byte &year) {
   Wire.beginTransmission(DS3231_I2C_ADDRESS);
   Wire.write(0);
   Wire.endTransmission();
   Wire.requestFrom(DS3231_I2C_ADDRESS, 7);








   second = bcdToDec(Wire.read() & 0x7f);
   minute = bcdToDec(Wire.read());
   hour = bcdToDec(Wire.read() & 0x3f);
   dayOfWeek = bcdToDec(Wire.read());
   dayOfMonth = bcdToDec(Wire.read());
   month = bcdToDec(Wire.read());
   year = bcdToDec(Wire.read());
}




// Zobrazení času na sériovém monitoru
void displayTime() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);








   Serial.print(hour);
   Serial.print(":");
   if (minute < 10) Serial.print("0");
   Serial.print(minute);
   Serial.print(":");
   if (second < 10) Serial.print("0");
   Serial.print(second);
   Serial.print(" ");
   Serial.print(dayOfMonth);
   Serial.print("/");
   Serial.print(month);
   Serial.print("/");
   Serial.print(year + 2000);
   Serial.print(" Den v týdnu: ");
   const char* daysOfWeek[] = {"Neděle", "Pondělí", "Úterý", "Středa", "Čtvrtek", "Pátek", "Sobota"};
   Serial.println(daysOfWeek[dayOfWeek - 1]);
}








void problinuti(int outputPin) {
   unsigned long problikMillis = millis(); // Nastavení času pro probliknutí
   for (int i = 0; i <= 9; ++i) {
       // Nulování čítače
       digitalWrite(outputPinnull, LOW);
       delayMicroseconds(1);
       digitalWrite(outputPinnull, HIGH);




       // Zobrazení hodnoty pro aktuální pozici
       for (int j = 0; j < i; ++j) {
           digitalWrite(outputPin, HIGH);
           delayMicroseconds(1);
           digitalWrite(outputPin, LOW);
           delayMicroseconds(1);
       }




       // Před pokračováním na další hodnotu počkáme 100 ms
       while (millis() - problikMillis < 100);
       problikMillis = millis();
   }
}








// Výstup pro sekundy
void secoutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int secondDigit = second % 10;


   // Výstup pro jednotky sekund
   for (int i = 0; i < secondDigit; ++i) {
       digitalWrite(outputPinSec, HIGH);
       digitalWrite(outputPinSec, LOW);
   }
}




// Výstup pro desítky sekund
void dessecoutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int firstDigit = second / 10;


   // Výstup pro desítky sekund
   for (int i = 0; i < firstDigit; ++i) {
       digitalWrite(outputPinSec, HIGH);
       digitalWrite(outputPinSec, LOW);
   }
}




// Výstup pro minuty
void minoutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int minuteDigit = minute % 10;


   // Výstup pro jednotky minut
   for (int i = 0; i < minuteDigit; ++i) {
       digitalWrite(outputPinMin, HIGH);
       digitalWrite(outputPinMin, LOW);
   }
}




// Výstup pro desítky minut
void desetminoutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int firstDigit = minute / 10;


   // Výstup pro desítky minut
   for (int i = 0; i < firstDigit; ++i) {
       digitalWrite(outputPinMin, HIGH);
       digitalWrite(outputPinMin, LOW);
   }
}




// Výstup pro hodiny
void hodoutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int hourDigit = hour % 10;


   // Výstup pro jednotky hodin
   for (int i = 0; i < hourDigit; ++i) {
       digitalWrite(outputPinHour, HIGH);
       digitalWrite(outputPinHour, LOW);
   }
}




// Výstup pro desítky hodin
void desetHodOutput() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
   readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   int firstDigit = hour / 10;


   // Výstup pro desítky hodin
   for (int i = 0; i < firstDigit; ++i) {
       digitalWrite(outputPinHour, HIGH);
       digitalWrite(outputPinHour, LOW);
   }
}




void loop() {
   unsigned long currentMillis = millis();
   unsigned long duplexOffMillis = 0;
 
   // Zobrazení času každou sekundu
   if (currentMillis - previousMillis >= interval) {
       previousMillis = currentMillis;
       displayTime();
   }




   // Přepnutí duplexního režimu každých duplexInterval milisekund s off-time
   if (currentMillis - previousDuplexMillis >= duplexInterval) {
       previousDuplexMillis = currentMillis;
     
       duplex = !duplex;  // Přepnutí duplex mezi true a false
   }




   // Výstup pro sekundy a desítky sekund, minuty a desítky minut, hodiny a desítky hodin
   if (duplex) {
       digitalWrite(duplexPin2, LOW);      // Nastavení duplexního pinu 1 na 0 nebude zobrazovat
       digitalWrite(duplexPin1, LOW);// Nastavení duplexního pinu 1 na 0 nebude zobrazovat


       // Nulování čítače
       digitalWrite(outputPinnull, LOW);
       digitalWrite(outputPinnull, HIGH);


       secoutput();
       minoutput();
       hodoutput();


       digitalWrite(duplexPin1, HIGH);     // Nastavení duplexního pinu 1 na 1 bude zobrazovat


       delay(duplexInterval);
   } else {
       digitalWrite(duplexPin1, LOW);// Nastavení duplexního pinu 1 na 0 nebude zobrazovat
       digitalWrite(duplexPin2, LOW);      // Nastavení duplexního pinu 1 na 0 nebude zobrazovat


       // Nulování čítače
       digitalWrite(outputPinnull, LOW);
       digitalWrite(outputPinnull, HIGH);


       dessecoutput();
       desetminoutput();
       desetHodOutput();


       digitalWrite(duplexPin2, HIGH); // Nastavení duplexního pinu 1 na 1 bude zobrazovat


       delay(duplexInterval);
   }


// digitalWrite(duplexPin2, LOW);       Zapomněl jsem k čemu je duplex pin 2 a teď na to nějak nemužu přijít když můžeme řítit všechno pomocí duplexpinu 1
                                               // ok možná to tam je proto že jsem předtím řešil duplex jiným způsobem jako 2 samostatné cykly ovládané pomocí
                                                   // dulpex pinu a ne jenom závislé na duplex pinu
// digitalWrite(duplexPin2, HIGH);
// Zavolání probliknutí každých blinkInterval milisekund
/*
   if (currentMillis - lastBlinkMillis >= blinkInterval) {
       lastBlinkMillis = currentMillis;               
       problinuti(outputPinSec);
       problinuti(outputPinMin);
       problinuti(outputPinHour);
   }
*/




}






























/*


Cyklus  Akce    Pin Počet pulsů Poznámka
1   Výstup jednotek sekund  outputPinSec    7   7 pulsů odpovídá jednotkám sekund.
   Výstup jednotek minut   outputPinMin    4   4 pulsy odpovídají jednotkám minut.
   Výstup jednotek hodin   outputPinHour   1   1 puls odpovídá jednotkám hodin.
2   Výstup desítek sekund   outputPinSec    0   0 pulsů, protože desítky sekund jsou 0.
   Výstup desítek minut    outputPinMin    3   3 pulsy odpovídají desítkám minut.
   Výstup desítek hodin    outputPinHour   0   0 pulsů, protože desítky hodin jsou 0.
3   Výstup jednotek sekund  outputPinSec    8   8 pulsů odpovídá změně na 8 sekund.
   Výstup jednotek minut   outputPinMin    5   5 pulsy odpovídají změně na 35 minut.
   Výstup jednotek hodin   outputPinHour   2   2 pulsy odpovídají změně na 2 hodiny.
4   Výstup desítek sekund   outputPinSec    0   0 pulsů, protože desítky sekund zůstávají 0.
   Výstup desítek minut    outputPinMin    3   3 pulsy odpovídají desítkám minut (stále 3).
   Výstup desítek hodin    outputPinHour   0   0 pulsů, protože desítky hodin zůstávají 0.
  
*/



