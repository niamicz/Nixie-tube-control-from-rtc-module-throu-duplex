#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68


// Definice pinů
const int outputPinSec = 9;     // Výstup pro čítač sekund a desítky sekund
const int outputPinMin = 8;     // Výstup pro čítač minut a desítky minut
const int outputPinHour = 7;    // Výstup pro čítač hodin a desítky hodin
const int outputPinnull = 5;    // Výstup pro nulování čítače
const int duplexPin1 = 13;       // První duplexní pin (přepínání mezi sec/min a hour)
const int duplexPin2 = 12;       // Druhý duplexní pin (antiparalelně)
const unsigned int duplexInterval = 500; // Interval přepínání duplex režimu (5 ms)
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
    // Nastavení výchozího stavu duplex pinů na LOW
    digitalWrite(duplexPin1, LOW);
    digitalWrite(duplexPin2, LOW);
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

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(10);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro jednotky sekund
    for (int i = 0; i < secondDigit; ++i) {
        digitalWrite(outputPinSec, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinSec, LOW);
        delayMicroseconds(1);
    }
}


// Výstup pro desítky sekund
void dessecoutput() {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    int firstDigit = second / 10;

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(1);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro desítky sekund
    for (int i = 0; i < firstDigit; ++i) {
        digitalWrite(outputPinSec, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinSec, LOW);
        delayMicroseconds(1);
    }
}


// Výstup pro minuty
void minoutput() {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    int minuteDigit = minute % 10;

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(1);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro jednotky minut
    for (int i = 0; i < minuteDigit; ++i) {
        digitalWrite(outputPinMin, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinMin, LOW);
        delayMicroseconds(1);
    }
}


// Výstup pro desítky minut
void desetminoutput() {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    int firstDigit = minute / 10;

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(1);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro desítky minut
    for (int i = 0; i < firstDigit; ++i) {
        digitalWrite(outputPinMin, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinMin, LOW);
        delayMicroseconds(1);
    }
}


// Výstup pro hodiny
void houroutput() {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    int hourDigit = hour % 10;

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(1);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro jednotky hodin
    for (int i = 0; i < hourDigit; ++i) {
        digitalWrite(outputPinHour, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinHour, LOW);
        delayMicroseconds(1);
    }
}


// Výstup pro desítky hodin
void desethouroutput() {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    int firstDigit = hour / 10;

    // Nulování čítače
    digitalWrite(outputPinnull, LOW);
    delayMicroseconds(1);
    digitalWrite(outputPinnull, HIGH);

    // Výstup pro desítky hodin
    for (int i = 0; i < firstDigit; ++i) {
        digitalWrite(outputPinHour, HIGH);
        delayMicroseconds(1);
        digitalWrite(outputPinHour, LOW);
        delayMicroseconds(1);
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Přepínání duplex režimu (sekundy / minuty a hodiny / desítky hodin)
    if (currentMillis - previousDuplexMillis >= duplexInterval) {
        previousDuplexMillis = currentMillis;

        // Krátký off-time před přepnutím duplexních pinů
        digitalWrite(duplexPin1, LOW);
        digitalWrite(duplexPin2, LOW);
        delayMicroseconds(500); // 0.5 ms pauza

        duplex = !duplex;  // Přepnutí duplex mezi true a false
        digitalWrite(duplexPin1, duplex ? HIGH : LOW);
        digitalWrite(duplexPin2, duplex ? LOW : HIGH);
    }

    // Probliknutí na sériovém monitoru
    if (currentMillis - lastBlinkMillis >= blinkInterval) {
        lastBlinkMillis = currentMillis;

        // Zobrazení času na monitoru
        displayTime();
    }

    // Volání funkcí pro zobrazení hodnot
    if (duplex) {
        secoutput();
        
        minoutput();
        desetminoutput();
    } else {
        dessecoutput();
        houroutput();
        desethouroutput();
    }
}
