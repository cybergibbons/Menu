#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
GLCD_ST7565 glcd;

#include <EEPROM.h>



#include "font_metric01.h"
#include "font_metric02.h"
#include "font_metric04.h"
#include "Bounce.h"


const byte LCD_WIDTH = 128;
const byte LCD_HEIGHT = 64;

const byte CHAR_WIDTH = 4;
const byte CHAR_HEIGHT = 6;

const byte MAX_ROWS = LCD_HEIGHT/CHAR_HEIGHT;
const byte MAX_COLS = LCD_WIDTH/CHAR_WIDTH;

const prog_char STRING_FLOW[] PROGMEM = "FLOW TEMP";
const prog_char STRING_RETURN[] PROGMEM = "RETURN TEMP";
const prog_char STRING_DHW[] PROGMEM = "DHW TEMP";
const prog_char STRING_EXT[] PROGMEM = "EXT TEMP";
const prog_char STRING_INT[] PROGMEM = "INT TEMP";
const prog_char STRING_HOUSE_POWER[] PROGMEM = "HOUSE POWER";
const prog_char STRING_PUMP_POWER[] PROGMEM = "HEAT PUMP POWER";


const prog_char* DEVICE_STRING_TABLE[] PROGMEM =
{
    STRING_FLOW,
    STRING_RETURN,
    STRING_DHW,
    STRING_EXT,
    STRING_INT,
    STRING_HOUSE_POWER,
    STRING_PUMP_POWER
};

const byte NUMBER_OF_SENSORS = 7;

const int ENTER_SWITCH  = 15;        // digital pin of enter switch - low when pressed
const int UP_SWITCH   = 19;           // digital pin of up switch - low when pressed
const int DOWN_SWITCH = 16;

Bounce enterSwitch = Bounce(ENTER_SWITCH, 5);
Bounce upSwitch = Bounce(UP_SWITCH,5);
Bounce downSwitch = Bounce(DOWN_SWITCH,5);

static byte state = 0;

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup()
{
    glcd.begin(0x19);    
    glcd.backLight(255);
    //Serial.begin(9600);

    pinMode(ENTER_SWITCH, INPUT);
    pinMode(UP_SWITCH, INPUT);
    pinMode(DOWN_SWITCH, INPUT);

    state = EEPROM.read(0x00);
}

void displayString(byte x, byte y, const char *string, bool invert)
{
    glcd.setFont(font_metric01);
    glcd.drawString(x*CHAR_WIDTH,y*CHAR_HEIGHT,string,invert);
}

void displayString_P(byte x, byte y, PGM_P string, bool invert)
{
    char buffer[MAX_COLS];
    strncpy_P(buffer,string,MAX_COLS);
    displayString(x,y,(char *)buffer, invert);
}


void loop()
{
    static byte flash = 0;
    
    static long mediumUpdate = 0;
    static long fastUpdate = 0;
    static bool writeState = false;

    enterSwitch.update();
    upSwitch.update();
    downSwitch.update();

    if (upSwitch.risingEdge())
        state++;

    if (downSwitch.risingEdge())
        state--;

    if (enterSwitch.risingEdge())
        writeState = true;


    if (millis() - mediumUpdate > 500)
    {
        mediumUpdate = millis();
        flash = !flash;
    }
    

    if (millis() - fastUpdate > 200)
    {
        fastUpdate = millis();
        glcd.clear();
    
        char buffer[MAX_COLS];

        for (int i=0; i<NUMBER_OF_SENSORS; i++)
        {
            itoa(i,buffer,10);
            strcat(buffer,"-");
            displayString(0,i,buffer,false);
            displayString_P(2,i,(char*)pgm_read_word(&(DEVICE_STRING_TABLE[i])),(i==state && flash));
        }

        glcd.refresh();

        if (writeState)
        {
            if (state != EEPROM.read(0x00))
                EEPROM.write(0x00, state);

        }
    }

}
