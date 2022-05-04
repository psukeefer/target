#include <LiquidCrystal.h>

//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 

const int TEST_A = 15; 
const int TEST_B = 15; 
const int TEST_C = 15; 
const int TEST_D = 15; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

long  the_rand, cal_accum[4];

int cal[4], max_dev[4], count[4], test[4];

void setup() {

  int i,j;

  test[0] = TEST_A;
  test[1] = TEST_B;
  test[2] = TEST_C;
  test[3] = TEST_D;

 lcd.begin(16, 2);

 for( i = 0; i < 4; i++)
    count[i] = 0;
 
 for(i = 0; i < 4; i++)
    cal_accum[i] = 0;
    
 pinMode(A1, INPUT_PULLUP);
 pinMode(A2, INPUT_PULLUP);
 pinMode(A3, INPUT_PULLUP);
 pinMode(A4, INPUT_PULLUP);

 for(  i = 0; i < 100; i++){
  
   for( j = 0; j < 4; j++)
    cal_accum[j] += analogRead(j+1);
    
   delay(25);
  
 }
 
 for( i = 0; i < 4; i++){
  
  cal[i] = cal_accum[i]/100;
  max_dev[i] = cal[i];
  
 }

 update_display();
 
 randomSeed( analogRead(5) );

}

void loop() {

 int i, the_max_idx, meas[4];
 long new_rand;
 
 for( i = 0; i < 4; i++){
  
  meas[i] = analogRead(i+1);
  max_dev[i] = ( abs(cal[i] - meas[i]) > abs(cal[i] - max_dev[i]) ) ? meas[i] : max_dev[i];

 }
 delay(15);
  
if( millis()%1000 < 50 ){
  
/*  
 do{ new_rand = random(0,4); }while(new_rand == the_rand);
 the_rand = new_rand;
 */
   the_max_idx = 0;
   
   for( i = 0; i < 4; i++)
    the_max_idx = ( abs(cal[i] - max_dev[i]) > abs(cal[the_max_idx] - max_dev[the_max_idx]) ) ? i : the_max_idx;
   

   if( abs(cal[the_max_idx] - max_dev[the_max_idx]) > test[the_max_idx] ){
      count[the_max_idx]++;
      update_display();
      delay(500);
   }
  
   for( i = 0; i < 4; i++)
    max_dev[i] = cal[i];
   
}

}

void update_display() {
  
     char msg[17];
      
     lcd.setCursor(0,0);           
     sprintf(msg, "A:%5u B:%5u", count[0], count[1]);
     lcd.print(msg);  
            
     lcd.setCursor(0,1);
     sprintf(msg, "C:%5u D:%5u", count[2], count[3]);
     lcd.print(msg);  

      return;

}
