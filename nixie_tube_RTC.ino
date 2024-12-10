#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68


// Definice pinů
const int outputPinSec = 9;     // Výstup pro čítač sekund a desítky sekund
const int outputPinMin = 8;     // Výstup pro čítač minut a desítky minut
const int outputPinHour = 7;    // Výstup pro čítač hodin a desítky hodin
const int outputPinnull = 5;    // Výstup pro nulování čítače
const int duplexPin1 = 13;       // První duplexní pin (přepínání mezi sec/min a hour)
const int duplexPin2 = 12;       // Druhý duplexní pin (antiparalelně)

const int inputpinmode = ?;
const int inputpinbutton1 = ?;     // Tlačítko pro změnu režimu (sekundy, minuty, hodiny) 
const int inputpinbutton2 = ?;     // Tlačítko pro inkrementaci hodnoty

const unsigned int duplexInterval = 4; // Interval přepínání duplex režimu (5 ms)
const unsigned long blinkInterval = 60000; // Interval pro probliknutí všech hodnot
unsigned long previousMillis = 0;           // Pro časování zobrazení času
const long interval = 1000;                 // Interval 1 sekundy pro displayTime
unsigned long previousDuplexMillis = 0;     // Pro časování přepnutí duplex režimu
unsigned long lastBlinkMillis = 0;          // Pro časování volání probliknutí
unsigned long previousTempMillis = 0;
const long tempInterval = 5000;


bool duplex = false;       // Přepínání mezi sekundami a minutami
bool hourDuplex = false;   // Přepínání mezi hodinami a desítkami hodin
bool zpoz = true;


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

    pinMode(inputpinmode, INPUT);
    pinMode(inputpinbutton1, INPUT);
    pinMode(inputpinbutton2, INPUT);
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
        digitalWrite(outputPinnull, HIGH);


        // Zobrazení hodnoty pro aktuální pozici
        for (int j = 0; j < i; ++j) {
            digitalWrite(outputPin, HIGH);
            digitalWrite(outputPin, LOW);
        }


        // Před pokračováním na další hodnotu počkáme 100 ms
        while (millis() - problikMillis < 100);
        problikMillis = millis();
    }
}

void duplexzobrazeni1pul(void (*funkce)()) {
    digitalWrite(duplexPin2, LOW);     
    digitalWrite(duplexPin1, LOW);
    digitalWrite(outputPinnull, LOW);
    digitalWrite(outputPinnull, HIGH);

    funkce();

    digitalWrite(duplexPin1, HIGH);     
    delay(duplexInterval);
}
void duplexzobrazeni2pul(void (*funkce)()) {
    digitalWrite(duplexPin1, LOW);// Nastavení duplexního pinu 1 na 0 nebude zobrazovat
    digitalWrite(duplexPin2, LOW);      // Nastavení duplexního pinu 1 na 0 nebude zobrazovat
    digitalWrite(outputPinnull, LOW);
    digitalWrite(outputPinnull, HIGH);

    funkce();

    digitalWrite(duplexPin2, HIGH); // Nastavení duplexního pinu 1 na 1 bude zobrazovat
    delay(duplexInterval);
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

void duplexcmd() {
    unsigned long currentMillis = millis();
    unsigned long duplexOffMillis = 0; 

    if (currentMillis - previousDuplexMillis >= duplexInterval) {
        previousDuplexMillis = currentMillis;
       
        duplex = !duplex;  // Přepnutí duplex mezi true a false
    }
}

void zobcas1p() {
    secoutput();
    minoutput();
    hodoutput();
}
void zobcas2p() {
    dessecoutput();
    desetminoutput();
    desetHodOutput();
}

// Výstup hodin funkční
void vystuphodinfinal() {
    duplexcmd();

    // Výstup pro sekundy a desítky sekund, minuty a desítky minut, hodiny a desítky hodin
    if (duplex) {
    duplexzobrazeni1pul(zobcas1p);

    } else {
        duplexzobrazeni2pul(zobcas2p);
    }        
}

void displayTimefinal() {
    unsigned long currentMillis = millis();
    unsigned long duplexOffMillis = 0;
    // Zobrazení času každou sekundu
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        displayTime();
    }
}

void probliknutifinal() {
    unsigned long currentMillis = millis();
    unsigned long duplexOffMillis = 0;
    // digitalWrite(duplexPin2, HIGH);
// Zavolání probliknutí každých blinkInterval milisekund
    if (currentMillis - lastBlinkMillis >= blinkInterval) {
        lastBlinkMillis = currentMillis;                
        problinuti(outputPinSec);
        problinuti(outputPinMin);
        problinuti(outputPinHour);
    }
}

float readTemperature() {
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0x11); 
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDRESS, 2); 

    byte msb = Wire.read(); 
    byte lsb = Wire.read();

    float temperature = msb + ((lsb >> 6) * 0.25);
    return temperature;
}
void sendTemperature() {
    float temperature = readTemperature();
    Serial.print("Teplota: ");
    Serial.print(temperature);
    Serial.println(" °C");
}


void teplfinal() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousTempMillis >= tempInterval) {
        previousTempMillis = currentMillis;
        sendTemperature();
    }
}
// Výstup pro jednotky teploty
void jedtempout() {
    float temperature = readTemperature();
    int intTemp = static_cast<int>(temperature); // Převod na int číslo
    int secondDigit = intTemp % 10;

    // Výstup pro jednotky teploty do čítače
    for (int i = 0; i < secondDigit; ++i) {
        digitalWrite(outputPinSec, HIGH);
        digitalWrite(outputPinSec, LOW);
    }
}


// Výstup pro desítky teploty do čitače
void destempout() {
    float temperature = readTemperature();
    int intTemp = static_cast<int>(temperature); // Převod na int číslo
    int firstDigit = intTemp / 10;

    // Výstup pro desítky sekund
    for (int i = 0; i < firstDigit; ++i) {
        digitalWrite(outputPinSec, HIGH);
        digitalWrite(outputPinSec, LOW);
    }
}
void vystuptempfinal() {
    duplexcmd();
    // Výstup pro jednotky stupňů a desítky stupňů
    if (duplex) {
        duplexzobrazeni1pul(jedtempout);
    } else {
        duplexzobrazeni2pul(destempout);    
        }
}

void timeset() {
    byte second = 0, minute = 0, hour = 0, dayOfWeek = 0, dayOfMonth = 0, month = 0, year = 0;
    byte mode = 0;
    readDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    if (digitalRead(inputpinbutton1) == HIGH) {
        mode++;
        if (mode > 3) {
            mode = 0;  
        }
        delay(200);
    }

    if (mode == 0) {
        duplexcmd();
        if (duplex) {
            duplexzobrazeni1pul(secoutput);

        } else {
            duplexzobrazeni2pul(dessecoutput);
        }
    }
    else if (mode == 1) {
        duplexcmd();
        if (duplex) {
            duplexzobrazeni1pul(minoutput);

        } else {
            duplexzobrazeni2pul(desetminoutput);
        }
    }
    else if (mode == 2) {
        duplexcmd();
        if (duplex) {
            duplexzobrazeni1pul(hodoutput);

        } else {
            duplexzobrazeni2pul(desetHodOutput);
        }
    }
    else if (mode == 3) {}

    if (digitalRead(inputpinbutton2)) {
        if (mode == 0) {
            second++;
            if (second > 59) second = 0;
            setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
            delay(200);
        } else if (mode == 1) {
            minute++;
            if (minute > 59) minute = 0;  
            setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
            delay(200);
        } else if (mode == 2) {
            hour++;
            if (hour > 23) hour = 0; 
            setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
            delay(200);
        } else if (mode == 3); {}
    }
}


void loop() {
    unsigned long currentMillis = millis();
    unsigned long duplexOffMillis = 0;
   

    if (digitalRead(inputpinmode) == HIGH) {
        timeset();
    }
    else if (digitalRead(inputpinmode) == LOW) {
        vystuphodinfinal();
    }

    //displayTimefinal();       // Zobrazení na sériovém monitoru např přes usb
    //probliknutifinal();
    //teplfinal();            // Zobrazení na sériovém monitoru např přes usb
    //vystuptempfinal();
            /*
            Zapomněl jsem k čemu je duplex pin 2 a teď na to nějak nemužu přijít když můžeme řítit všechno pomocí duplexpinu 1
            ok možná to tam je proto že jsem předtím řešil duplex jiným způsobem jako 2 samostatné cykly ovládané pomocí 
            dulpex pinu a ne jenom závislé na duplex pin
            */
}




// GS G27 GD40161B
// TESLA x68 MH74141
