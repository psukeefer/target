/*
Arduino 2x16 LCD - Detect Buttons
modified on 18 Feb 2019
by Saeed Hosseini @ Electropeak
https://electropeak.com/learn/
*/

#include <LiquidCrystal.h>
//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 

int count_a; 
int count_b; 
int count_c; 
int count_d; 

const int TEST_A = 512; 
const int TEST_B = 512; 
const int TEST_C = 512; 
const int TEST_D = 512; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

int mode;
int y_last;

long the_rand;
int a_max, b_max, c_max, d_max;

void setup() {

 lcd.begin(16, 2);

 count_a = 0;
 count_b = 0;
 count_c = 0;
 count_d = 0;

 a_max = 0;
 b_max = 0;
 c_max = 0;
 d_max = 0;
       
 update_display();
 randomSeed(analogRead(4)+analogRead(3)+analogRead(2)+analogRead(1)+analogRead(0));
}



void loop() {

 int HIT_DELAY = 1000;
 int a,b,c,d;
 long new_rand;
 
 a = random(0,1023); //analogRead(1);
 b = random(0,1023); //analogRead(2);
 c = random(0,1023); //analogRead(3);
 d = random(0,1023); //analogRead(4);
 delay(50);
 
 a_max = (a > a_max) ? a : a_max;
 b_max = (b > b_max) ? b : b_max;
 c_max = (c > c_max) ? c : c_max;
 d_max = (d > d_max) ? d : d_max; 

if( millis()%1000 < 50 ){
  count_a = a_max;
  count_b = b_max;
  count_c = c_max;
  count_d = d_max;
  update_display();

  a_max = b_max = c_max = d_max = 0;

/*  
 do{ new_rand = random(0,4); }while(new_rand == the_rand);
 the_rand = new_rand;

 if((a > TEST_A) || (the_rand == 0) ){
    count_a++;
    update_display();
 //   delay(HIT_DELAY);
 }

 if((b > TEST_B) || (the_rand == 1) ){
    count_b++;
    update_display();
//    delay(HIT_DELAY);
 }
 
 if((c > TEST_C) || (the_rand == 2) ){
    count_c++;
    update_display();
//    delay(HIT_DELAY);
 }
 
 if((d > TEST_D) || (the_rand == 3) ){
    count_d++;
    update_display();
//    delay(HIT_DELAY);
 } 
 */
}

}

void update_display() {
  
     char msg[17];
      
     lcd.setCursor(0,0);           
     sprintf(msg, "A:%5u B:%5u", count_a, count_b);
     lcd.print(msg);  
            
     lcd.setCursor(0,1);
     sprintf(msg, "C:%5u D:%5u", count_c, count_d);
     lcd.print(msg);  

      return;

}
