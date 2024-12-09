Update 09.12.24: 	Zobrazení teploty z RTC DS3231, na digitronech pro jednotky a desítky sekund.
			Nastavení času pomocí 3 tlačítek.



Ovládání digitronových hodin pomocí čítačů

Tento projekt ukazuje, jak ovládat digitronové hodiny s použitím modulu reálného času DS3231 a vlastního systému výstupů řízených čítačem. Kód využívá Arduino pro přesné časování a generování signálů pro sekundy, minuty a hodiny. Projekt rovněž podporuje multiplexování (duplexní režim) pro efektivní využití pinů.
Funkce

    Integrace reálného času: Využití modulu DS3231 pro přesné měření času.
    Řízení pomocí čítačů: Generování pulsů odpovídajících číslicím sekund, minut a hodin.
    Duplexní režim: Efektivní multiplexování pro zobrazení více číslic s minimem hardwaru.
    Zobrazení na sériovém monitoru: Ladění a zpětná vazba v reálném čase pomocí sériového monitoru Arduina.

Video zobrazující fungování: https://youtube.com/shorts/OE9x49Hq_tQ?feature=share

Konfigurace pinů
Pin	Funkce
9	Výstup pro sekundy a desítky sekund
8	Výstup pro minuty a desítky minut
7	Výstup pro hodiny a desítky hodin
5	Nulování čítače
13	Duplexní pin 1 (řízení multiplexu)
12	Duplexní pin 2 (řízení multiplexu, antiparalelně)

Klíčové funkce
1. Převod číslic

    Převod BCD na desítkové číslo: Převádí formát BCD z RTC na desítkové hodnoty.
    Převod desítkových čísel na BCD: Umožňuje nastavení času přes konverzi na BCD.

2. Multiplexní výstup

    Hodiny střídavě přepínají mezi:
        Režim sekund/minut.
        Režim desítek sekund/minut a hodin.
    Řízeno pomocí pinů duplexPin1 a duplexPin2.

3. Aktualizace času

    Čas z RTC je pravidelně čten a zobrazován:
        Jednotky a desítky jsou odděleny a zasílány jako pulsy na příslušné výstupní piny.
    Příklad:
        Čas 12:34:56:
            Sekundy (jednotky): 6 pulsů na outputPinSec.
            Minuty (desítky): 3 pulsy na outputPinMin.

4. Probliknutí

    Všechny číslice mohou probliknout v režimu „probliknutí“ pro ověření zapojení a funkčnosti.
    Aktivováno v nastavitelných intervalech.

Časování duplexního režimu

    Interval duplexu: 5 ms (nastavitelný).
    Podrobnosti cyklu:
        V každém cyklu se střídají výstupy pro jednotky a desítky číslic.
        Signály pro nulování zajišťují, že čítače začínají vždy od nuly.

Příklad cyklu
Cyklus	Akce	Pin	Pulsy	Poznámky
1	Výstup jednotek sekund	outputPinSec	7,	7 pulsů odpovídá 7 sekundám.
	Výstup jednotek minut	outputPinMin	4,	4 pulsy odpovídají 4 minutám.
	Výstup jednotek hodin	outputPinHour	1,	1 puls odpovídá 1 hodině.
2	Výstup desítek sekund	outputPinSec	0,	Desítky sekund = 0.
	Výstup desítek minut	outputPinMin	3,	3 pulsy pro desítky minut.
	Výstup desítek hodin	outputPinHour	0,	Desítky hodin = 0.

Možnosti přizpůsobení

Kód obsahuje místa pro:

    Nastavení počátečního času RTC při spuštění.
    Nastavení intervalů pro probliknutí a multiplexování.
Tento projekt demonstruje efektivní a škálovatelný přístup k ovládání digitronových hodin s minimem hardwaru a přesným časováním. 
Je navržen tak, aby byl snadno rozšiřitelný o další funkce.
