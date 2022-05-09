#include <LiquidCrystal.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 0
// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF



Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
  NEO_MATRIX_BOTTOM    + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 

const int TEST_A = 500; 
const int TEST_B = 500; 
const int TEST_C = 500; 
const int TEST_D = 500; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

long  the_rand, cal_accum[4];

int cal[4], Ain[4], meas[4], max_dev[4], count[4], test[4], outs[4], strike, target_count, update_led, mode;

const int CONSTANT_DISPLAY_TIME = 1500;
const int TARGET_DISPLAY_TIME = 4100; //MS
const int TARGET_COUNTS = 15; // # of targets to display
const int TARGET_SAMPLE_WINDOW = 200; //MS
const int HIT_DELAY = 2000;
const int MODE_COUNT = 3; 

unsigned int sel_stat;

void setup() {
    
    int i,j; 

    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(5);
    matrix.setTextColor(RED);

    Ain[0] = A1;
    Ain[1] = A2;
    Ain[2] = A3;
    Ain[3] = A4;

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
    strike = 0;
    randomSeed( analogRead(5) );
    the_rand = random(0,4);

    target_count = 0;
    update_led = 1;

    mode = 0;
    sel_stat = 0;
 
}

void loop() {

    int i, the_max_idx, sel_button, tmp;
    long loop_time;
    char msg[17];

    // event based actions
    
    if( (target_count == TARGET_COUNTS) && (mode == 0) ){
       
       lcd.clear();
       lcd.setCursor(0,0); 
       sprintf(msg, "%3u / %3u HIT", count[0] + count[1] + count[2] + count[3], TARGET_COUNTS );   
       lcd.print(msg );  

       matrix.fillScreen(0);    //Turn off all the LEDs   
       matrix.setCursor(11, 0);
       matrix.print(count[0] + count[1] + count[2] + count[3]);   
       matrix.show();

       while(1){} //pause;
      
    }
  
    if(( ( update_led )) ){
        
          matrix.fillScreen(0);    //Turn off all the LEDs
          the_rand = (the_rand + random(1,4))%4;

         switch(the_rand){
           case 0: matrix.fillRect(0,0,8,3,RED); break;
           case 1: matrix.fillRect(0,5,8,3,GREEN); break;
           case 2: matrix.fillRect(32-8,0,8,3,YELLOW); break;
           case 3: matrix.fillRect(32-8,5,8,3,BLUE); break;
         
         }
         
        matrix.setCursor(11, 0);
        matrix.print(count[0] + count[1] + count[2] + count[3]);  

        matrix.show();
  
        target_count = target_count + 1;
        update_led = 0;
        strike = 0;   
    
        for( i = 0; i < 4; i++)
           max_dev[i] = cal[i];

        return;

    } 

    //sample targets
    for( i = 0; i < 4; i++){

        meas[i] = analogRead(Ain[i]);
        max_dev[i] = ( abs(cal[i] - meas[i]) > abs(cal[i] - max_dev[i]) ) ? meas[i] : max_dev[i];
                
    } 
    
    ///////////////////////////////////
    //time based actions
    ////////////////////////////////////
    
    loop_time = millis(); //update time

    if(( loop_time%CONSTANT_DISPLAY_TIME < 20 )){

        if(mode == 2){
                
           update_display();
           
           for( i = 0; i < 4; i++)
            max_dev[i] = cal[i];  
           
        }
          
    
    }
    
    if(( loop_time%TARGET_DISPLAY_TIME < 20 )){

        if( mode == 0){
          update_led = 1;
          delay(20);
        }
        
        if(sel_stat == 0xFFFF){
          
            mode = (mode + 1)%MODE_COUNT;

            sprintf(msg, "MODE = %5u", mode);  
            lcd.clear();          
            lcd.print(msg);  

            strike = 0;
            target_count = 0;
            
            for( i = 0; i < 4; i++)
              count[i] = 0;
        
            matrix.fillScreen(0);    //Turn off all the LEDs
            matrix.show();
  
            delay(2000);  //delay for release
                              
            update_display();
                
        }
        
        return;
    }

    //end of sampling window actions
    if( loop_time%TARGET_SAMPLE_WINDOW < 20 ){
      
       the_max_idx = 0;
   
       for( i = 0; i < 4; i++)
            the_max_idx = ( abs(cal[i] - max_dev[i]) > abs(cal[the_max_idx] - max_dev[the_max_idx]) ) ? i : the_max_idx;
     
       if (mode == 0){    
         
          if(( abs(cal[the_max_idx] - max_dev[the_max_idx]) > test[the_max_idx] ) && (the_max_idx == the_rand ) && (strike == 0)){
          
            count[the_max_idx]++;
            matrix.fillScreen(0);    //Turn off all the LEDs

            matrix.setCursor(11, 0);
            matrix.print(count[0] + count[1] + count[2] + count[3]);  
            
            matrix.show();
            
            strike = 1;
            update_display();
         
          }

          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i]; 
          
       }

       if (mode == 1){    
         
          if(( abs(cal[the_max_idx] - max_dev[the_max_idx]) > test[the_max_idx] )){

            count[the_max_idx]++;
            matrix.fillScreen(0);    //Turn off all the LEDs

             switch(the_max_idx){
               case 0: matrix.fillRect(0,0,8,3,RED); break;
               case 1: matrix.fillRect(0,5,8,3,GREEN); break;
               case 2: matrix.fillRect(32-8,0,8,3,YELLOW); break;
               case 3: matrix.fillRect(32-8,5,8,3,BLUE); break;
             
             }
             
            matrix.setCursor(11, 0);
            matrix.print(count[0] + count[1] + count[2] + count[3]);  
            
            matrix.show();           
            count[the_max_idx]++;
            update_display();
            delay(1500);
            
          }
          
          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i]; 
             
       }
     
       if(analogRead(A0) < 800)
           sel_stat = (sel_stat << 1)|1;
       else
           sel_stat = 0;

       delay(20);
       return;
     
    }


}

void update_display() {
  
     char msg1[17], msg2[17];
     
     switch(mode){
         
          case 0: case 1:sprintf(msg1, "A:%5u B:%5u", count[0], count[1]);        
                    sprintf(msg2, "C:%5u D:%5u", count[2],  count[3]);   
                    break;
                
          case 2:   sprintf(msg1, "A:%5u B:%5u", max_dev[0], max_dev[1]);        
                    sprintf(msg2, "C:%5u D:%5u", max_dev[2], max_dev[3]);   
                    break;                
                
     
     }
     
    lcd.setCursor(0,0);           
    lcd.print(msg1);  
        
    lcd.setCursor(0,1);
    lcd.print(msg2);  

      return;

}
