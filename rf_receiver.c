
/* 
   RF Receiver using PIC16F887 microcontroller 
   CCS C code
   Based on NEC protocol
   Internal oscillator 8MHz
*/

#include <16F887.h>
#fuses NOMCLR, NOBROWNOUT, NOLVP, INTRC_IO
#use delay(clock = 8MHz)
#use fast_io(C)
#define DATA_PIN  PIN_C5
#define CLOCK_PIN PIN_C3
#define LOAD_PIN  PIN_C4

void init_spi();
void max7219_send(unsigned char address, unsigned char data);
void max7219_init();
void clear_matrix();

// Holds whether the NEC protocol code is received correctly
short code_ok = 0;       

// State variables used during NEC protocol reception
unsigned int8 nec_state = 0, i;

// Variable holding received 32-bit NEC code
unsigned int32 rf_code;  

//------------------------------------------------------------------
// External interrupt service (INT_EXT)
// NEC protocol timing measured via Timer1
//------------------------------------------------------------------
#INT_EXT
void ext_isr(void){
   unsigned int16 time;
   if(nec_state != 0){
      time = get_timer1();   // Get Timer1 value
      set_timer1(0);         // Reset Timer1
   }

   switch(nec_state){
      case 0: // Initial state when 9ms pulse starts
         setup_timer_1(T1_INTERNAL | T1_DIV_BY_2); // Start Timer1 (internal clock, prescaler=2)
         set_timer1(0);
         nec_state = 1;
         i = 0;
         ext_int_edge(H_TO_L);  // Set interrupt edge to High to Low
         return;

      case 1: // 9ms pulse ends
         if((time > 9500) || (time < 8500)){ // If time is not around 9ms, it's an error
            nec_state = 0;                  
            setup_timer_1(T1_DISABLED);
         } else {
            nec_state = 2; // Move to 4.5ms space
         }
         ext_int_edge(L_TO_H);
         return;

      case 2: // 4.5ms space ends
         if((time > 5000) || (time < 4000)){ // If not 4.5ms, it's an error
            nec_state = 0;
            setup_timer_1(T1_DISABLED);
            return;
         }
         nec_state = 3; // Move to 560µs pulse
         ext_int_edge(H_TO_L);
         return;

      case 3: // 560µs pulse ends
         if((time > 700) || (time < 400)){ // If not 560µs, it's an error
            nec_state = 0;
            setup_timer_1(T1_DISABLED);
         } else {
            nec_state = 4; // Measure the following space
         }
         ext_int_edge(L_TO_H);
         return;

      case 4: // 560µs or 1680µs space ends
         if((time > 1800) || (time < 400)){ // If not within 560µs ~ 1680µs, it's an error
            nec_state = 0;
            setup_timer_1(T1_DISABLED);
            return;
         }
         // If greater than 1ms, write '1', else write '0'
         if(time > 1000)
            bit_set(rf_code, (31 - i));
         else
            bit_clear(rf_code, (31 - i));

         i++;
         if(i > 31){ // 32 bits received
            code_ok = 1;       // Code received successfully
            disable_interrupts(INT_EXT); 
         }
         // Go back to 560µs pulse for next bit measurement
         nec_state = 3; 
         ext_int_edge(H_TO_L);
   }
}

//------------------------------------------------------------------
// Timer1 interrupt service (for timeout situations)
//------------------------------------------------------------------
#INT_TIMER1
void timer1_isr(void){
   nec_state = 0;               
   ext_int_edge(L_TO_H);        
   setup_timer_1(T1_DISABLED);  
   clear_interrupt(INT_TIMER1); 
}

//------------------------------------------------------------------
// Main program
//------------------------------------------------------------------
void main() {
   setup_oscillator(OSC_8MHZ);
   set_tris_c(0b00000000); // Set RC pins as outputs
   output_low(CLOCK_PIN);
   output_high(LOAD_PIN);
   output_low(DATA_PIN);
    
   init_spi();
   max7219_init();

   unsigned char A_char[8] = {0x00, 0x20, 0x60, 0xff, 0xff, 0x60, 0x20, 0x00};
   unsigned char B_char[8] = {0x00, 0x04, 0x06, 0xff, 0xff, 0x06, 0x04, 0x00};
   unsigned char C_char[8] = {0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18};
   unsigned char D_char[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18};

   enable_interrupts(GLOBAL);
   enable_interrupts(INT_EXT);
   clear_interrupt(INT_TIMER1);
   enable_interrupts(INT_TIMER1);

   while (TRUE) {
      clear_matrix();
      int pot1 = 2;
      int pot2 = 2;

      if (code_ok) {
         code_ok = 0;
         nec_state = 0;
         setup_timer_1(T1_DISABLED);

         if (rf_code == 0x00FF00FF)
            pot1 = 0;
         else if (rf_code == 0x00FF807F)
            pot1 = 1;
         else if (rf_code == 0x00FF10EF)
            pot1 = 2;
         else if (rf_code == 0x00FF40BF)
            pot2 = 0;
         else if (rf_code == 0x00FF20DF)
            pot2 = 1;
         else if (rf_code == 0x00FF08F7)
            pot2 = 2;
      }

      // Update display pattern based on pot values
      if (pot1 == 1) {
         if (pot2 == 2) {
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, A_char[i]);
            }
         }
         else if(pot2 == 1){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, A_char[i]);
            }
            delay_ms(500);
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, C_char[i]);
            }
         }
         else if(pot2 == 0){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, D_char[i]);
            }
            delay_ms(500);
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, A_char[i]);
            }
         }
      }

      if (pot1 == 0) {
         if (pot2 == 2){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, B_char[i]);
            }
         }
         else if(pot2 == 1){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, B_char[i]);
            }
            delay_ms(500);
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, C_char[i]);
            }
         }
         else if(pot2 == 0){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, B_char[i]);
            }
            delay_ms(500);
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, D_char[i]);
            }
         }
      }

      if (pot1 == 2){
         if(pot2 == 2){
            clear_matrix();
         }
         else if(pot2 == 1){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, C_char[i]);
            }
         }
         else if(pot2 == 0){
            for (unsigned char i = 0; i < 8; i++) {
               max7219_send(i + 1, D_char[i]);
            }
         }
      }

      enable_interrupts(INT_EXT);
      delay_ms(500); // Wait time
   }
}

void init_spi() {
   // SPI settings are not done with CCS C since software SPI is used
   output_low(CLOCK_PIN);
   output_high(LOAD_PIN);
   output_low(DATA_PIN);
}

void max7219_send(unsigned char address, unsigned char data) {
   output_low(LOAD_PIN); // Start data transmission

   for (unsigned char i = 0; i < 8; i++) {
      output_low(CLOCK_PIN);
      output_bit(DATA_PIN, (address >> (7 - i)) & 0x01); // Send bit
      output_high(CLOCK_PIN);
   }

   for (unsigned char i = 0; i < 8; i++) {
      output_low(CLOCK_PIN);
      output_bit(DATA_PIN, (data >> (7 - i)) & 0x01); // Send bit
      output_high(CLOCK_PIN);
   }

   output_high(LOAD_PIN); // End data transmission
}

void max7219_init() {
   // Initialize MAX7219
   max7219_send(0x09, 0x00); // Disable decode mode
   max7219_send(0x0A, 0x08); // Medium brightness
   max7219_send(0x0B, 0x07); // 8 rows active
   max7219_send(0x0C, 0x01); // Normal operation mode
   max7219_send(0x0F, 0x00); // Disable test mode
}

void clear_matrix() {
   for (unsigned char i = 0; i < 8; i++) {
      max7219_send(i + 1, 0x00); // Send 0 to all rows to turn off LEDs
   }
}
