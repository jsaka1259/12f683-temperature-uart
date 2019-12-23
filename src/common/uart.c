#include "uart.h"

void uart_init(void) {
  // 1/4 Pre, 1/1 Post
  T2CON = 0x01;
  // Clear Interrupt Flag
  TMR2IF = 0;
  TXPIN  = 1;
}

void uart_putc(const char data) {
  uint8_t bit_cnt = 0;
  uint8_t bit_pos = 0x01;

  // Set Baudrate
  PR2 = BAUD;
  // Output START Bit
  TXPIN = 0;
  // Initialize TMR2
  TMR2 = 0;
  // 1/4 Pre, 1/1 Post
  T2CON = 0x05;

  // Wait Send Complate
  while(bit_cnt < 10) {
    // Wait Detect TMR2 Flag
    while(!TMR2IF)
      ;

    TMR2IF = 0;
    bit_cnt++;

    // 1-8bit
    if(bit_cnt > 0 && bit_cnt < 9) {
      // Output Data Bit
      TXPIN = (data & bit_pos) ? 1 : 0;
      bit_pos <<= 1;
    } else if(bit_cnt == 9) { // End Data
      // Output STOP Bit
      TXPIN = 1;
    } else if(bit_cnt == 10) { // End STOP Bit
      // Stop TMR2
      T2CON = 0x01;
    }
  }
}

void uart_puts(const char *buf) {
  while(*buf)
    uart_putc(*buf++);
}

char uart_getc(void) {
  uint8_t bit_cnt = 0;
  uint8_t bit_pos = 0x01;
  uint8_t data    = 0;

  // Wait START Bit
  while(RXPIN)
    ;

  // Initialize TMR2
  TMR2 = 0;
  // Set 1/2 Bit Width
  PR2  = DTIME;
  // Defeat TMR2 Interrupt Flag
  TMR2IF = 0;
  // 1/4 Pre, 1/1 Post
  T2CON  = 0x05;

  // Wait Receive Complate
  while(bit_cnt < 10) {
    // Wait TMR2 Interrupt
    while(!TMR2IF)
      ;

    TMR2IF = 0;

    switch(bit_cnt) {
      case 0: // Process START Bit
        if(RXPIN)
          bit_cnt = 0xFF;
        else {
          PR2 = BAUD;
          bit_cnt++;
        }
        break;
      case 9: // Process STOP Bit
        if(RXPIN)
          bit_cnt++;
        else
          bit_cnt = 0xFF;
        // Stop TMR2
        T2CON = 0x01;
        break;
      default: // Process 1-8bit Data
        if(RXPIN)
          data |= bit_pos;
        bit_pos <<= 1;
        bit_cnt++;
    }
  }

  if(bit_cnt == 10) {
    // Return Valid Data
    return data;
  } else {
    // Return Error Flag
    return 0xFF;
  }
}
