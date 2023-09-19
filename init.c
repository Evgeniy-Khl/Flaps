// Input/Output Ports initialization
// Port B initialization
PORTB=0x00; // State7=T State6=T State5=T State4=T State3=T State2=T State1=0 State0=0
DDRB=0x03;  // Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=Out Func0=Out

// Port C initialization
PORTC=0x03; // State6=T State5=T State4=T State3=T State2=T State1=P State0=P
DDRC=0x00;  // Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In

// Port D initialization
PORTD=0x04; // State7=0 State6=0 State5=T State4=T State3=T State2=P State1=T State0=T
DDRD=0xC0;  // Func7=Out Func6=Out Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In

// Timer/Counter 0 initialization
TCCR0=0x03; // Clock value: 125,000 kHz clk/64 (From prescaler)
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 125,000 kHz
// Mode: Fast PWM top=ICR1
// OC1A output: Non-Inverted PWM  // !! неправильная работа китайского процессора !!
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 20 ms
// Output Pulse(s):
// !! неправильная работа китайского процессора !!
// OC1A Period: 8,192 ms Width: 0,50449 ms -> OCR1AL=0x3F; Width: 1,6807 ms -> OCR1BL=0xD2;
// !! неправильная работа китайского процессора !!
// Timer1 Overflow Interrupt: On
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
//TCCR1A=(1<<COM1A1) | (1<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
// !! неправильная работа китайского процессора !!
TCCR1A=(1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
// !! неправильная работа китайского процессора !!
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (1<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
ICR1H=0x09;
ICR1L=0xC3;
pstStep = PRESET1b-flpClose;  // единиц на 1%
setflap(flpNow);
OCR1AL=pvOcr1al;

// Timer/Counter 2 initialization
// Timer Period: 8,192 ms
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x06;   // Clock value: 31,250 kHz clk/256 (From prescaler);
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
GICR|=0x40; // INT0: On;
MCUCR=0x02; // INT0 Mode: Falling Edge
GIFR=0x40;


// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (1<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1) | (0<<TOIE0); // TOV0 Off; TOV1 On; TOV2 On;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

delay_ms(1000);
//--------------------------------------------------------------------
if(KEYUp==0 || KEYDn==0){  
  while (OCR1AL>flpClose) {OCR1AL--; delay_ms(8);};
  while (1){         
    if(KEYUp==0){if(--flpClose<1) flpClose=1;}
    if(KEYDn==0){if(++flpClose>PRESET1b-50) flpClose=PRESET1b-50;}
    if(OCR1AL != flpClose){
        OCR1AL = flpClose;
        BEEPER=1; delay_ms(250); BEEPER=0;
    }
    delay_ms(500);
   }
}
//--------------------------------------------------------------------
#asm("cli")
// 1 Wire Bus initialization
w1_init();

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048k
#pragma optsize-
WDTCR=0x1F;
WDTCR=0x0F;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Global enable interrupts
#asm("sei")
/*
setflap(100);
delay_ms(1000);
setflap(0);
delay_ms(1000);
 */
