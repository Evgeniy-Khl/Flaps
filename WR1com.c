void w1_reply(void)
{
 unsigned char *p,i,crc;
  p = data;
  crc=w1_dow_crc8(p,4);
  for (i=0;i<4;i++) w1_write_slave(*p++);
  w1_write_slave(crc);
}

void w1_handler(void)
{
 unsigned char byte, *p;
  byte=w1_read_slave();   // проверка присутствия модуля
  if(byte==ID)
   {
     #asm("wdr") 
     p = buffer;
     for (byte=0; byte<4; byte++) *p++ = w1_read_slave(); // Read cont. byt
     p = buffer;
     byte=w1_dow_crc8(p,3);
     if(byte==buffer[3])  // проверка контрольной суммы
      {
       switch(buffer[0])
        {
          case SETFLAP:  setflap(100); data[0]=100; break;            // аварийное полное открытие заслонки
          case DATAREAD: setflap(flpNow); data[0]=flpNow; break;      //
          case DATAWR:   flpNow=buffer[1]; setflap(flpNow); data[0]=flpNow; break;// заслонка в исходное состояние
        };
       w1_reply();
      };
   };
  TimeSlot=0; Fall=0; LEDYel=OFF; // ВАЖНО на этом месте !!!
  TIMSK&=0xFE;                    // TOV0 Off TOV1 Off
  MCUCR=0x02;                     // INT0 Mode: Falling Edge;
  GIFR=0x40; GICR=0x40;           // INT0: On; Interrupt on any change on pins PCINT0-5: On
}
