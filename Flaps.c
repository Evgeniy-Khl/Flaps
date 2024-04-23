/*****************************************************
Project : Flap_Mega8
Version : 2.0
Chip type           : ATmega8
Clock frequency     : 8,000000 MHz
Program size        : 711 words (1422 bytes), 17,4% of FLASH [0x6E3D] EEPROM [0x000A] 23.04.2024
*****************************************************/

#include <mega8.h>
#include <delay.h>
#include <1wire.h>
#include <1wireslave8.h>
// !! ������������ ������ ���������� ���������� !!
//#define PRESET1a 	    5     // ��� ��������� ����������� 1,50 ms; 90 ���.(������� ��� ��������� HD-1501MG)
#define PRESET1a 	    10    // ��� ��������� ����������� 1,22 ms; 90 ���.(������� ��� ��������� MG996)
#define PRESET1b        130   // ��� ��������� ����������� 0,50 ms; 0 ���. (�������)
// !! ������������ ������ ���������� ���������� !!
#define ID          0xF7    // ������������� �����
#define RESET       0xC1    // Generate Reset Pulse
#define DATAREAD    0xA1    // Read Scratchpad
#define SETFLAP     0xA2    // ��������� ������ �������� �������� 
#define DATAWR      0xA4    // �������� ��������� ��������

#define BEEPER	    PORTB.0
#define KEYUp	    PINC.0
#define KEYDn	    PINC.1
#define LEDRed	    PORTD.6
#define LEDYel	    PORTD.7
#define ON          0
#define OFF         1

/***** used for TIMER0 count initialization *****/
//                              ��������������-�������(���)/��������*������������
//                              ��������������-�������(���)/��������*�����������
//                              ��������������-�������(��)/��������*�������
#define F_XTAL	            8000L               //quartz crystal frequency [kHz]
#define INIT_TIMER1_1MC     TCNT1=0x10000L-F_XTAL/8L*1L+3L // 1ms.

#define TIMING480    60	// (50)*64*0.125=400 us
#define PRESET48    250	// (256 - n)*64*0.125= 48 us  ( Waits 48 us )
#define PRESET120 	240	// (256 - n)*64*0.125= 128 us ( Presence pulse 120 us )
#define PRESET260 	224	// (256 - n)*64*0.125= 256 us ( Waiting 260 us )
#define TIMING1SK    31	// (31)*256*1024*0.125=1000000 us

// 1 Wire Bus functions
#asm
   .equ __w1_port=0x12 ;PORTD
   .equ __w1_bit=2
#endasm
// Declare your global variables here
unsigned char pvOcr1al, beepOn, counter, ledlight, pstStep, data[4], buffer[4];
eeprom unsigned char flpClose=PRESET1a, flpNow=0;

bit TimeSlot;
bit Fall;
bit RstPuise;
bit Waiting;
bit PrsPuise;
bit Edit;

// �������� % � ����� ��������
void setflap(unsigned char pvFlap)
{
  unsigned int tmp;
//  tmp = pvFlap;
// !! ������������ ������ ���������� ���������� !!
  tmp = 100 - pvFlap;           // !! ������������ ������ ���������� ���������� !! 
// !! ������������ ������ ���������� ���������� !!
  tmp *= pstStep; 
  tmp /= 100; 
  tmp += flpClose;               // ��������� ��������
  if(tmp>PRESET1b) tmp = PRESET1b; // open; ����������� ��������� ��������� 
  pvOcr1al = (char)tmp;
}

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{
 if (Fall==0)               // ������ ����
  {
    #asm("wdr")
    TCNT0=0;                // 256*64*0.25 = 4096 us max.
    MCUCR=0x03;             // INT0 Mode: Rising Edge
    Fall=1;
  }
 else                       // ������ �����
  {
    if (TCNT0>=TIMING480)
     {
       RstPuise=1; LEDYel=ON;
       TCNT0=PRESET48; TIFR|=0x01; TIMSK|=0x01; // Waits 48 us
       GICR=0;                                // INT0: Off; Interrupt on any change on pins PCINT0-5: Off
     }
    else {Fall=0; MCUCR=0x02;}                // INT0 Mode: Falling Edge
  };
}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
 if(RstPuise)
   {
     TCNT0=PRESET120;                   // Presence pulse 120 us
     #asm
	cbi  __w1_port,__w1_bit
	sbi  __w1_port-1,__w1_bit       ; set 1Wr
     #endasm
     RstPuise=0; PrsPuise=1;
   }
  else if(PrsPuise)
   {
     TCNT0=PRESET260;                   // Waiting 260 us
     #asm
	cbi  __w1_port-1,__w1_bit	; relise 1Wr
     #endasm
     PrsPuise=0; Waiting=1;
   }
  else if(Waiting) {Waiting=0; TimeSlot=1; TIMSK&=0xFE;}// TOV0 Off
}

// Timer1 overflow interrupt service routine  Timer Period: 20 ms
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
  if(++counter>15) {counter=0; Edit=1;}
  if((KEYUp || KEYDn) & Edit){ // ���� ������ ������ ���� ������   
    Edit = 0;
    if(KEYUp==0) {if(flpNow<100) {flpNow++;} setflap(flpNow); beepOn = 25;}
    if(KEYDn==0) {if(flpNow>0)  {flpNow--;} setflap(flpNow); beepOn = 25;}
   //if(KEYUp==0){flpNow =100; setflap(flpNow); beepOn = 25;}
   //if(KEYDn==0){flpNow = 0; setflap(flpNow); beepOn = 25;}
  }
  if(OCR1AL != pvOcr1al){if(OCR1AL<pvOcr1al) OCR1AL++; else OCR1AL--;}
}


// Timer 2 overflow interrupt service routine
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
  if(--ledlight==0) {ledlight = 1; LEDRed = OFF;}  
  if (beepOn) {--beepOn; BEEPER=1;}	   // �������� ������  1
  else BEEPER=0;                       // ��������� ������ 0
}

#include "WR1com.c"

void main(void)
{
// Declare your local variables here
//unsigned char i;
#include "init.c"

while (1)
  {
   if(TimeSlot)
    {
      if(flpNow) {LEDRed = ON; TCNT2 = 0; ledlight = (flpNow+flpNow/4);}
      else LEDRed = OFF;
      w1_handler();
    }
  };
}
