#include <xc.h>
#include <stdint.h>

#include "common/common.h"

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#define TMR1_HIGH_VAL 0x0b
#define TMR1_LOW_VAL  0xdc

int16_t  val    = 0;
int16_t  i_val  = 0;
uint16_t f_val  = 0;
uint8_t  mode   = 0;
uint8_t  u_en   = 0;
char     buf[9] = {};
uint8_t  cnt    = 0;

void __interrupt() isr(void) {
  if (T1IF) {
    TMR1H = TMR1_HIGH_VAL;
    TMR1L = TMR1_LOW_VAL;
    T1IF = 0;

    if (cnt > 0) {
      val   = adt7410_read(mode);
      i_val = val >> 7;
      f_val = (uint16_t)val & 0x007f;

      itos(&buf[0], i_val, 10, 3, ' ');
      buf[3] = '.';
      itos(&buf[4], get_frac_part(f_val, 7, 4), 10, 4, '0');

      st7032i_cmd(0x85);
      st7032i_puts(buf);

      if (u_en) {
        uart_puts(buf);
        uart_puts("\r\n");
      }
    }

    cnt = (cnt + 1) % 2;
  }
}

void main(void) {
  OSCCON = 0x70;
  GPIO   = 0x00;
  TRISIO = 0x09;
  ANSEL  = 0x00;
  CMCON0 = 0x07;
  WPU    = 0x00;
  nGPPU  = 1;

  uart_init();
  i2c_init();
  st7032i_init();
  adt7410_init(mode);

  GP2 = 1;
  st7032i_cmd(0x80);
  st7032i_puts("TEMP:");
  st7032i_cmd(0xc0);
  st7032i_puts("UART: ");
  st7032i_puts(u_en ? "SEND" : "NONE");

  T1CON = 0x30;
  TMR1H = TMR1_HIGH_VAL;
  TMR1L = TMR1_LOW_VAL;

  TMR1IE = 1;
  PEIE   = 1;
  GIE    = 1;
  TMR1ON = 1;
  
  while(1) {
    char c = uart_getc();

    TMR1ON = 0;
    switch (c) {
      case 't':
        GP2 = GP2 ^ 1;
        break;
      case 'u':
        u_en = u_en ^ 1;
        st7032i_cmd(0xc6);
        st7032i_puts(u_en ? "SEND" : "NONE");
        break;
    }
    TMR1ON = 1;
  }
}
