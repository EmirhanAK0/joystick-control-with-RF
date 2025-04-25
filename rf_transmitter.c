
/* 
   RF Transmitter using PIC16F887 microcontroller 
   CCS C code
   Based on NEC protocol
   Internal oscillator 8MHz
*/

#include <16F887.h>
#fuses NOMCLR, NOBROWNOUT, NOLVP, INTRC_IO
#device ADC=10
#use delay(clock = 8MHz)
#use fast_io(B)

// Specify that ADC will be 10-bit

//------------------------------------------------------------------
// Function to transmit signal according to NEC protocol
//------------------------------------------------------------------
void send_signal(unsigned int32 number){
   int8 i;
   // Send 9ms pulse
   output_high(PIN_B4);
   delay_ms(9);

   // 4.5ms space
   output_low(PIN_B4);
   delay_us(4500);

   // Send 32-bit data sequentially
   for(i = 0; i < 32; i++){
      // If bit is 1 => 560us pulse + 1680us space
      if(bit_test(number, 31 - i)){
         output_high(PIN_B4);
         delay_us(560);
         output_low(PIN_B4);
         delay_us(1680);
      }
      // If bit is 0 => 560us pulse + 560us space
      else{
         output_high(PIN_B4);
         delay_us(560);
         output_low(PIN_B4);
         delay_us(560);
      }
   }

   // Send end bit
   output_high(PIN_B4);
   delay_us(560);
   output_low(PIN_B4);
   delay_us(560);
}

//------------------------------------------------------------------
// Main program
//------------------------------------------------------------------
void main() {
   // Set internal oscillator to 8MHz
   setup_oscillator(OSC_8MHZ);

   // Reset port B
   output_b(0);
   set_tris_b(0x00);  // Set other pins as needed, RB4 is output

   //------------------------------------------------
   // ADC configuration
   //------------------------------------------------
   // Define AN0 and AN1 as ADC inputs
   setup_adc_ports(sAN0 | sAN1);
   // Use internal clock (or suitable clock) for ADC
   setup_adc(ADC_CLOCK_INTERNAL);

   // Variables
   unsigned int16 adc_val = 0;   // 10-bit reading
   unsigned int8 pot1_8bit = 0;  // For scaling to 0-255 range
   unsigned int8 pot2_8bit = 0;

   while(TRUE) {
      int pot1 = 2;  // 0 or 1
      int pot2 = 2;
      
      //--------------------------------------------
      // 1) Read pot1 value from AN0
      //--------------------------------------------
      set_adc_channel(0);
      delay_us(20);                // Small delay for channel switching
      adc_val = read_adc();        // 10-bit read (0-1023)
      pot1_8bit = adc_val >> 2;    // Scale to 0-255

      if(pot1_8bit <= 100){
         pot1 = 0;
      }
      else if(pot1_8bit >= 150){
         pot1 = 1;
      }
      else
         pot1 = 2;  // In range 100-149, do not change pot1 value

      //--------------------------------------------
      // 2) Read pot2 value from AN1
      //--------------------------------------------
      set_adc_channel(1);
      delay_us(20);                  
      adc_val = read_adc();          
      pot2_8bit = adc_val >> 2;      

      if(pot2_8bit <= 100){
         pot2 = 0;
      }
      else if(pot2_8bit >= 150){
         pot2 = 1;
      }
      else
         pot2 = 2;  // No operation for range 100-149

      //--------------------------------------------
      // 3) Send signal based on pot1 and pot2 values
      //    (Different 32-bit values can be sent for
      //     pot1=0,1 and pot2=0,1 for example)
      //--------------------------------------------

      // pot1 = 0
      if(pot1 == 0){
         send_signal(0x00FF00FF);
         delay_ms(500);
      }

      // pot1 = 1
      if(pot1 == 1){
         send_signal(0x00FF807F);
         delay_ms(500);
      }
      if(pot1 == 2){
         send_signal(0x00FF10EF);
         delay_ms(500);
      }

      // pot2 = 0
      if(pot2 == 0){
         send_signal(0x00FF40BF);
         delay_ms(500);
      }

      // pot2 = 1
      if(pot2 == 1){
         send_signal(0x00FF20DF);
         delay_ms(500);
      }
      if(pot2 == 2){
         send_signal(0x00FF08F7);
         delay_ms(500);
      }

      // Short delay before next read cycle
      // (To reduce RF transmission frequency)
      delay_ms(200);
   }
}
