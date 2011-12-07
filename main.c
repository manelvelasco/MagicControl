#include <p33FJ64MC802.h>

volatile unsigned int conversions=0;

void __attribute__((interrupt)) _T1Interrupt (void) //Manera de declarar interrupcions
{
    PORTBbits.RB15=0;
    conversions=0;
    IFS0bits.T1IF=0;
}
void __attribute__((interrupt)) _ADC1Interrupt (void) //Manera de declarar interrupcions
{
    PORTBbits.RB15=1;
    conversions++;
    IFS0bits.AD1IF=0;
                        // Aquesta RSI S'executa en 2,96us
}

void Init_AD(void)
{
    IFS0bits.AD1IF=0;       // Borrem pendents...
    IPC3bits.AD1IP=7;       // Ojo amb la prioritat

    AD1CON1bits.ADON=0;     // Engeguem AD
    AD1PCFGL=0xFFFE;        // Només el primer AN0 és entrada analògica (bit a 0)
    AD1CON2bits.VCFG2=0;
    AD1CON2bits.VCFG1=0;
    AD1CON2bits.VCFG0=0;    // Referència del AD: Vss-Vdd

    AD1CON3=AD1CON3&0x7F00; // Usem el system clock (bit15=0)
    AD1CON3=AD1CON3|0x0003; // divisor 4 (bit7-0=1) TAD=4*TCY

    AD1CON2bits.CHPS0=0;
    AD1CON2bits.CHPS1=0;    // Usem un únic S&H: canal0
    AD1CSSL=0x0001;         // Canal 0 entrada
    AD1CHS0=0x0000;         // Canal 0 triat
    AD1CON1bits.AD12B=1;    // 12 bits !

    AD1CON1bits.SSRC0=1;
    AD1CON1bits.SSRC1=1;
    AD1CON1bits.SSRC2=1;    // Auto Convert

    AD1CON1bits.ASAM=1;     // Auto Sample

    AD1CON3bits.SAMC0=0;
    AD1CON3bits.SAMC1=0;
    AD1CON3bits.SAMC2=1;
    AD1CON3bits.SAMC3=0;
    AD1CON3bits.SAMC4=0;   // Sample= 4 TAD garantim temps

    AD1CON1bits.FORM0=0;
    AD1CON1bits.FORM1=0;   // Sortida entera 12 bits

    AD1CON1bits.ADON=1;    // Engeguem AD

    IEC0bits.AD1IE=1;      // Enable interrupt AD

    // Temps total, uns 18 TAD => 72 TCY * 1 / 3,7MHz = 19,43us
}

void Init_T1(void)
{
    T1CON=0;                //Reset Timer
    IFS0bits.T1IF=0;        //Borrem pendents...
    IPC0bits.T1IP=6;        //Ojo amb la prioritat
    IEC0bits.T1IE=1;        //Enable timer
    TMR1=0x0000;            //Posem a 0
    T1CONbits.TCKPS0=0;
    T1CONbits.TCKPS1=1;     //Prescaler a 64 sobre Fosc=7.37,Fcy=3.7MHz
    PR1=57812;              //57812 Periode del timer =1s
    T1CONbits.TON=1;        //Engeguem
}

void Init_PWM(void)
{
    //FPOR
    P1TCONbits.PTCKPS=0b00;
    P1TCONbits.PTOPS =0b00;  //Pre i postscaler a 1
    P1TCONbits.PTMOD =0b00;  //Free running

    P1TPER= 37000;          // Periode    = 10ms
    P1DC1 = 740*1;          // Duty Cicle = 1% = 100us garanteix 5 mostres d'AD
    P1TMR=0;                // Iniciem timer dedicat a 0

    PWM1CON1bits.PMOD1=0;
    PWM1CON1bits.PEN1H=1;
    PWM1CON1bits.PEN1L=0;

    PWMCON2bits.IUE=1;
    PWMCON2bits.UDIS=0;

    P1OVDCONbits.POVD1H=1;
    P1OVDCONbits.POVD1L=0;

    P1TCONbits.PTEN = 1;          // Enable PWM
}

void Set_PWM (unsigned int duty)  // duty pot valdre fins 88% tal com està programat
{
    P1DC1 = 740*duty;             // Duty Cicle = 1% = 100us garanteix 5 mostres d'AD
}

int main(void)
{
    conversions=0;
    TRISB=0x0000;
    LATB=0x0000;
    Init_T1();
    Init_AD();
    Init_PWM();
    while(1)
    {
        PORTBbits.RB15=0;
        Set_PWM(0);
    }
}