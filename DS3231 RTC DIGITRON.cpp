#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68

// Definice pinů
const int outputPinSec = 4;     // Výstup pro čítač sekund a desítky sekund
const int outputPinMin = 6;     // Výstup pro čítač minut a desítky minut
const int outputPinHour = 7;    // Výstup pro čítač hodin a desítky hodin
const int outputPinnull = 5;    // Výstup pro nulování čítače
const int duplexPin1 = 1;       // První duplexní pin (přepínání mezi sec/min a hour)
const int duplexPin2 = 2;       // Druhý duplexní pin (antiparalelně)
// Tlačítka
const int button1Pin = 8; // Tlačítko pro přepínání režimů
const int button2Pin = 9; // Tlačítko pro změnu hodnoty

unsigned long button2HoldTime = 1000; // Doba podržení tlačítka 2 (ms)
enum SettingMode { SECONDS, MINUTES, HOURS, NONE };
SettingMode currentMode = NONE;
byte second = 0, minute = 0, hour = 0;
bool isEditing = false; // Indikace režimu úprav
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;
bool button2Held = false;


const unsigned int duplexInterval = 5; // Interval přepínání duplex režimu (5 ms)
const unsigned long blinkInterval = 60000; // Interval pro probliknutí všech hodnot
unsigned long previousMillis = 0;           // Pro časování zobrazení času
const long interval = 1000;                 // Interval 1 sekundy pro displayTime
unsigned long previousDuplexMillis = 0;     // Pro časování přepnutí duplex režimu
unsigned long lastBlinkMillis = 0;          // Pro časování volání probliknutí

bool duplex = false;       // Přepínání mezi sekundami a minutami
bool hourDuplex = false;   // Přepínání mezi hodinami a desítkami hodin

// Převod normálních čísel na BCD a Převod BCD na normální desítkové číslo
byte decToBcd(byte val) {
    return ((val / 10 * 16) + (val % 10));
}
byte bcdToDec(byte val) {
    return ((val / 16 * 10) + (val % 10));
}

void setup() {
      pinMode(outputPinSec, OUTPUT);
    pinMode(outputPinMin, OUTPUT);
    pinMode(outputPinHour, OUTPUT);
    pinMode(outputPinnull, OUTPUT);
    pinMode(duplexPin1, OUTPUT);  
    pinMode(duplexPin2, OUTPUT); 
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);

    //setDS3231time(20,29,18,3,12,11,24);       // Nastavení času (pouze příklad)
    Wire.begin();
    Serial.begin(9600);

    byte dayOfWeek, dayOfMonth, month, year;  // Lokální proměnné pro chybějící argumenty
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

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

void saveTimeToRTC() {
    byte currentDayOfWeek = 1;  // Můžete doplnit aktuální hodnoty
    byte currentDayOfMonth = 1; // Např. čtením z RTC při inicializaci
    byte currentMonth = 1;
    byte currentYear = 23;      // Rok 2023 (musí být přizpůsobeno)

    // Aktualizace času do RTC
    setDS3231time(second, minute, hour, currentDayOfWeek, currentDayOfMonth, currentMonth, currentYear);
    Serial.println("Čas uložen do RTC!");
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
void hodoutput() {
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
void desetHodOutput() {
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

// Probliknutí aktuálně nastavované hodnoty
void blinkCurrentField() {
    static unsigned long lastBlink = 0;
    static bool blinkOn = true;

    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        blinkOn = !blinkOn;
    }

    if (blinkOn) {
        displayTime();
    } else {
        switch (currentMode) {
            case SECONDS: Serial.println("Sekundy blikají"); break;
            case MINUTES: Serial.println("Minuty blikají"); break;
            case HOURS: Serial.println("Hodiny blikají"); break;
            default: break;
        }
    }
}
// Zpracování tlačítek
void handleButtons() {
    if (digitalRead(button1Pin) == LOW) {
        delay(50); // Debounce
        if (millis() - lastButton1Press > 300) {
            lastButton1Press = millis();
            handleButton1Press();
        }
    }

    if (digitalRead(button2Pin) == LOW) {
        if (!button2Held && millis() - lastButton2Press > button2HoldTime) {
            button2Held = true;
            handleButton2Hold();
        }
    } else {
        if (button2Held) button2Held = false;
        if (millis() - lastButton2Press > 300) {
            lastButton2Press = millis();
            handleButton2Press();
        }
    }
}

// Tlačítko 1: Přepnutí režimu
void handleButton1Press() {
    if (!isEditing) {
        isEditing = true;
        currentMode = SECONDS;
    } else {
        switch (currentMode) {
            case SECONDS: currentMode = MINUTES; break;
            case MINUTES: currentMode = HOURS; break;
            case HOURS:
                currentMode = NONE;
                isEditing = false;
                saveTimeToRTC();  // Uložit změny do RTC
                break;
            default: break;
        }
    }
}

// Tlačítko 2: Změna hodnoty
void handleButton2Press() {
    switch (currentMode) {
        case SECONDS: second = (second + 1) % 60; break;
        case MINUTES: minute = (minute + 1) % 60; break;
        case HOURS: hour = (hour + 1) % 24; break;
        default: break;
    }
}

// Tlačítko 2: Rychlé přidávání
void handleButton2Hold() {
    switch (currentMode) {
        case SECONDS: second = (second + 5) % 60; break;
        case MINUTES: minute = (minute + 5) % 60; break;
        case HOURS: hour = (hour + 1) % 24; break;
        default: break;
    }
}



void loop() {
    unsigned long currentMillis = millis();
    unsigned long duplexOffMillis = 0;
    
handleButtons();

    // Blikání aktuálně nastavovaného pole, pokud jsme v režimu editace
    if (isEditing) {
        blinkCurrentField();
    } else {
        // Zobrazení času každou sekundu
        static unsigned long lastDisplayMillis = 0;
        if (currentMillis - lastDisplayMillis >= 1000) {
            lastDisplayMillis = currentMillis;
            displayTime();
        }

        // Ovládání digitronů (přepínání mezi režimy sec/min/hour)
        static unsigned long lastDuplexMillis = 0;
        if (currentMillis - lastDuplexMillis >= duplexInterval) {
            lastDuplexMillis = currentMillis;

            // Krátký off-time před přepnutím duplexních pinů
            digitalWrite(duplexPin1, LOW);
            digitalWrite(duplexPin2, LOW);
            delayMicroseconds(500);

            // Přepnutí duplexního režimu
            duplex = !duplex;
            digitalWrite(duplexPin1, duplex ? HIGH : LOW);
            digitalWrite(duplexPin2, duplex ? LOW : HIGH);
        }

        // Generování výstupů na digitrony
        if (duplex) {
            secoutput();
            minoutput();
            hodoutput();
        } else {
            dessecoutput();
            desetminoutput();
            desetHodOutput();
        }
    }

    


// Zavolání probliknutí každých blinkInterval milisekund

    if (currentMillis - lastBlinkMillis >= blinkInterval) {
        lastBlinkMillis = currentMillis;                 
        problinuti(outputPinSec);
        problinuti(outputPinMin);
        problinuti(outputPinHour);
    }


}
