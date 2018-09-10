/* Main.c file generated by New Project wizard
 *
 * Created:   Monday september 3 2018
 * Processor: PIC16F628A
 * Compiler:  MPLAB XC8
 */

#include <xc.h>

__CONFIG (INTIO & UNPROTECT & LVPDIS & BOREN & MCLRDIS & PWRTEN & WDTDIS);

#define LED1_POWER RA2
#define LED2_POWER RA1
#define LED3_POWER RA0

// if rgb led with active cathode use 0 else 1
#define LED_POWER_VAL 0

#define LED_RED RB7
#define LED_GREEN RA6
#define LED_BLUE RA7


#define LED_COLOR_VAL !LED_COLOR_VAL

#define SW_R1 RB6 // row 1
#define SW_R2 RB5 // row 2
#define SW_R3 RB4 // row 3

#define SW_C1 RB1 // column 1
#define SW_C2 RB2 // column 2
#define SW_C3 RB3 // column 3

#define BUTTON RA4
#define BUZ RA3
#define BUZ_SOUND RB0

#define _XTAL_FREQ 4000000

#define BTN_CNT 9

#define BUTTON_MAX_TIME_MS 2000
#define BUTTON_SHORT_CLICK_TIME 10

#define SWITCH_DELAY_TIME 50 // Delay for switch mode
#define SWITCH_DELAY_FLK_COUNT 3 // How many times LED was flick in switch mode
#define SWITCH_FLK_DELAY_MS 500

#define SCAN_START_DELAY 5
#define SCAN_STOP_DELAY 5

#define LED_DELAY 50

#define MODE_SIMPLE    0
#define MODE_DIFFICULT 1

#define COLOR_RED_NUM 0
#define COLOR_GREEN_NUM 1
#define COLOR_BLUE_NUM 2

#define LED_SWITCH_DELAY_MS 100

#define TMR1_SCALAR 192
#define TMR1_TIME (_XTAL_FREQ / 1000) / TMR1_SCALAR // interrupt time ms

#define ENABLE_LED(POWER, COLOR_NUM)           \
    POWER = LED_POWER_VAL;                     \
    if (COLOR_NUM == COLOR_RED_NUM)            \
        LED_RED = LED_COLOR_VAL;               \
    else if (COLOR_NUM = COLOR_GREEN_NUM)      \
        LED_GREEN = LED_COLOR_VAL;             \
    else                                       \
        LED_BLUE = LED_COLOR_VAL;              


#define DISABLE_LED(POWER) \
    POWER = LED_COLOR_VAL;

typedef struct LedState LedState;
struct LedState {
    unsigned short Power; // 0 1 (Off, On)
    unsigned short Color; // 0 1 2 (R, G, B)
};

void init();
void readButtonsState();
void buttonPressed(int time);
void setAllLedsState(short power, short color);
void switchMode();

unsigned short buttonsState[BTN_CNT] = {};
unsigned short mode = 0;
unsigned short modeSwitching = 0;

unsigned short buttonPressedTime = 0;
unsigned short buttonPressedTimeCopy = 0;
unsigned short isButtonPressed = 0;

LedState ledsState[3] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};

void main (void) {
    init();
    while(1) {
        if (modeSwitching == 1) {
            continue;
        }          
        
        if (isButtonPressed == 1) {
            buttonPressed(buttonPressedTimeCopy);
            isButtonPressed = 0;
        }

        readButtonsState();
    }
}

void init() {
    TRISA = 0b00110000;
    TRISB = 0b00001110;
    CMCON = 0x07;
    PORTA = 0;
    PORTB = 0;
    nRBPU = 1;
   
    OPTION_REG &= TMR1_SCALAR; // Make prescalar
    T0IE = 1; // Enable Timer0 interrupt
    GIE = 1; // Enable global interrupts

    for (unsigned int i = 0; i < BTN_CNT; i++) {
        buttonsState[i] = 0;
    }
   
    LED1_POWER = LED_COLOR_VAL;
    LED2_POWER = LED_COLOR_VAL;
    LED3_POWER = LED_COLOR_VAL;
   
    LED_RED = LED_POWER_VAL;
    LED_GREEN = LED_POWER_VAL;
    LED_BLUE = LED_POWER_VAL;

    SW_R1 = 0;
    SW_R2 = 0;
    SW_R3 = 0;
}

void readButtonsState() {
    SW_R1 = 1;
    __delay_ms(SCAN_START_DELAY);
    buttonsState[0] = SW_C1;
    buttonsState[1] = SW_C2;
    buttonsState[2] = SW_C3;
    __delay_ms(SCAN_STOP_DELAY);
    SW_R1 = 0;
   
    SW_R2 = 1;
    __delay_ms(SCAN_START_DELAY);
    buttonsState[3] = SW_C1;
    buttonsState[4] = SW_C2;
    buttonsState[5] = SW_C3;
    __delay_ms(SCAN_STOP_DELAY);
    SW_R2 = 0;
   
    SW_R3 = 1;
    __delay_ms(SCAN_START_DELAY);
    buttonsState[6] = SW_C1;
    buttonsState[7] = SW_C2;
    buttonsState[8] = SW_C3;
    __delay_ms(SCAN_STOP_DELAY);
    SW_R3 = 0;
}

void buttonPressed(int time) {
    if (time > BUTTON_SHORT_CLICK_TIME && time < BUTTON_MAX_TIME_MS) {
        
    } else if (time >= BUTTON_MAX_TIME_MS) {
        switchMode();
    }
}

void setAllLedsState(short power, short color) {
    for (int i = 0; i < 3; i++) {
        ledsState[i].Power =  power;
        ledsState[i].Color = color;
    }
}

void switchMode() {
    modeSwitching = 1;
    unsigned short color = COLOR_GREEN_NUM;
    if (mode == MODE_SIMPLE) {
        mode = MODE_DIFFICULT;
    } else {
        color = COLOR_BLUE_NUM;
        mode = MODE_SIMPLE;
    }

    setAllLedsState(1, color);

    for (unsigned int i = 0; i < SWITCH_DELAY_TIME; i++) {
        BUZ = 1;
        if (i > 0 && i % SWITCH_DELAY_TIME / SWITCH_DELAY_FLK_COUNT + 1 == 0) {
            BUZ = 0;
            setAllLedsState(0, color);
            __delay_ms(SWITCH_FLK_DELAY_MS);
            setAllLedsState(1, color);
            BUZ = 1;
        }
    }

    setAllLedsState(0, color);

    modeSwitching = 0;
}

// the interrupt vector

unsigned short currentLed = 0;
unsigned short ledDelayTime = 0;
void interrupt ISR(void) {
    if(T0IF) {
        T0IF = 0;
    }


    ledDelayTime += TMR1_TIME;
    
    if (ledDelayTime >= LED_SWITCH_DELAY_MS) {
        ledDelayTime = 0;
        if (currentLed > 2) {
            currentLed = 0;
        }
  
        if (ledsState[currentLed].Power == 1) {
            unsigned short color = ledsState[currentLed].Color;
            if (currentLed == 0) {
                DISABLE_LED(LED2_POWER);
                DISABLE_LED(LED3_POWER);
                ENABLE_LED(LED1_POWER, color);
            } else if (currentLed == 1) {
                DISABLE_LED(LED1_POWER);
                DISABLE_LED(LED3_POWER);
                ENABLE_LED(LED2_POWER, color);
            } else if (currentLed == 2) {
                DISABLE_LED(LED1_POWER);
                DISABLE_LED(LED2_POWER);
                ENABLE_LED(LED3_POWER, color);
            }
        } else {
            DISABLE_LED(LED1_POWER);
            DISABLE_LED(LED2_POWER);
            DISABLE_LED(LED3_POWER);
        }

        currentLed++;    
    }
    if(modeSwitching == 1) {
        return;
    }

    if (BUTTON == 1) {
        if (buttonPressedTime < BUTTON_MAX_TIME_MS) {
            buttonPressedTime += TMR1_TIME;
        }
    } else {
        if (buttonPressedTime > 0) {
            buttonPressedTimeCopy = buttonPressedTime;
            isButtonPressed = 1;
        }
        buttonPressedTime = 0;
    }
}