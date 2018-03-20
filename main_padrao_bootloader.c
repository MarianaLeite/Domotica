/* Mariana da Silva Leite   201511950099 */
#include "user.h"

#ifndef _BOOTLOADER  
// CONFIG2
#pragma config POSCMOD = XT     // XT Oscillator mode selected
#pragma config OSCIOFNC = ON    // OSC2/CLKO/RC15 as as port I/O (RC15)
#pragma config FCKSM = CSDCMD   // Clock Switching and Monitor disabled
#pragma config FNOSC = PRI      // Primary Oscillator (XT, HS, EC)
#pragma config IESO = ON        // Int Ext Switch Over Mode enabled

// CONFIG1
#pragma config WDTPS = PS32768  // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128    // WDT Prescaler (1:128)
#pragma config WINDIS = ON      // Watchdog Timer Window Mode disabled
#pragma config FWDTEN = OFF     // Watchdog Timer disabled
#pragma config ICS = PGx2       // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF       // Writes to program memory allowed
#pragma config GCP = OFF        // Code protection is disabled
#pragma config JTAGEN = OFF     // JTAG port is disabled

#endif

#ifdef _BOOTLOADER  
    //Gravação no módulo físico
    #define PS1  1
    #define BR   415
#else
    //Simulação no Proteus
    #define PS1  0
    #define BR   51
#endif

void UART_Init (void);
void OC_Init (void);

#define LAMP_on     OC1CON1bits.OCM = 0b110
#define LAMP_off    OC1CON1bits.OCM = 0
#define LUM_50      OC1R = 999
#define LUM_100     OC1R = 2000
#define TV_switch   LATBbits.LATB0 ^= 1

char function;

int main(void) {
    
    UART_Init();
    OC_Init();
    
    while (1)
    {  
        switch (function) {
            case 't':
                TV_switch;
                function = 0;
                break;
            case 'n':
                LAMP_off;
                function = 0;
                break;
            case 'm':
                LAMP_on;
                LUM_50;
                break;
            case 'f':
                LAMP_on;
                LUM_100;
                break;
            default:
                break;
        }
    }

    return -1;
}

void UART_Init (void) {
    
    RPINR18bits.U1RXR = 10;     //RX remap
    TRISFbits.TRISF4 = 1;
    
    /* STSEL: 1 stop bit | PDSEL: 8-bit data, no parity | BRGH: high baud rate | RXINV: idle state is '1'
    UEN: UxTX and UxRX pins are enable and used */
    U1MODE = 0x8;
    
    /* UTXEN: transmit disabled */
    U1STA = 0;
    
    U1BRG = BR;
    
    //Interrupt
    INTCON1 = 0;
    INTCON2 = 0;
    IFS0bits.U1RXIF = 0;
    IPC2bits.U1RXIP = 7;
    IEC0bits.U1RXIE = 1;
    
    //Enable
    U1MODEbits.UARTEN = 1;
}

void OC_Init (void) {
    
    RPOR14bits.RP29R = 18;      //Compare pin
    
    /* RB0 and RB15 as output */
    AD1PCFG = 0xFFFF;
    TRISB = 0x7FFE;
    LATBbits.LATB15 = 0;
    LATBbits.LATB0 = 0;
    
    /* Timer 1 config */
    T1CON = 0;
    T1CONbits.TCKPS = PS1;
    PR1 = 65535;
    TMR1 = 0;
    
    /* OCM: compare channel is disabled | OCTSEL: timer 1 as clock source */
    OC1CON1 = 0x1000;
    
    /* SYNCSEL: this OC module as synchronization source */
    OC1CON2 = 0x1F;
    OC1R = 0;
    OC1RS = 1999;                   //1ms period
    
    //Enable
    T1CONbits.TON = 1;
}

void __attribute__((interrupt,no_auto_psv)) _U1RXInterrupt(void){
    if(IFS0bits.U1RXIF){    
        IFS0bits.U1RXIF = 0;
		function = U1RXREG;
    }
}