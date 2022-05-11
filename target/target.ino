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

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(   32, 8, PIN,
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
const int TEST_C = 400; 
const int TEST_D = 400; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

long  the_rand, cal_accum[4], loop_time;

int cal[4], Ain[4], meas[4], max_dev[4], count[4], TEST_VAL[4], outs[4], strike, target_count, update_led, mode;

//time windows
const int CONSTANT_DISPLAY_WINDOW = 1500;
const int TARGET_DISPLAY_WINDOW = 8000; //MS
const int TARGET_SAMPLE_WINDOW = 200; //MS
const int SEL_BUTTON_WINDOW = 2000; //MS

long time_CONSTANT_DISPLAY;
long time_TARGET_DISPLAY;
long time_TARGET_SAMPLE;
long time_SEL_BUTTON;

int     flag_CONSTANT_DISPLAY_WINDOW,
        flag_SEL_BUTTON_WINDOW,
        flag_TARGET_SAMPLE_WINDOW,
        flag_TARGET_DISPLAY_WINDOW;

const int TARGET_COUNTS = 20; // # of targets to display
const int HIT_DELAY = 2000;
const int MODE_COUNT = 3; 

unsigned int sel_stat;

void setup() {
    
    int i,j; 

    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(210);
    matrix.setTextColor(BLUE);

    Ain[0] = A1;
    Ain[1] = A2;
    Ain[2] = A3;
    Ain[3] = A4;

    TEST_VAL[0] = TEST_A;
    TEST_VAL[1] = TEST_B;
    TEST_VAL[2] = TEST_C;
    TEST_VAL[3] = TEST_D;

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
   
    time_CONSTANT_DISPLAY = time_TARGET_DISPLAY = time_TARGET_SAMPLE = millis();
 
}

void loop() {

    int i, the_max_idx, sel_button, tmp;
    char msg[17];
    double test, test_max;

    //////////////////////////////
    // event based actions
    
    if( (target_count == (TARGET_COUNTS+1)) && (mode == 0) ){
       
       lcd.clear();
       lcd.setCursor(0,0); 
       sprintf(msg, "%3u / %3u HIT", count[0] + count[1] + count[2] + count[3], TARGET_COUNTS );   
       lcd.print(msg );  

       matrix.fillScreen(0);    //Turn off all the LEDs   
       matrix.setCursor(11, 0);
       matrix.print(count[0] + count[1] + count[2] + count[3]);   
       matrix.show();

       tmp = analogRead(1);
       
       while(tmp > 800)
        tmp = analogRead(1);

       target_count = 0;
       update_led = 1;
       strike = 0;
       
       for( i = 0; i < 4; i++)
          count[i] = 0;
          
       return;
      
    }
  
    if( update_led ){
        
          matrix.fillScreen(0);    //Turn off all the LEDs
          the_rand = (the_rand + random(1,4))%4;

          if(target_count < TARGET_COUNTS){
               switch(the_rand){
                 case 0: matrix.fillRect(0,0,6,3,RED); break;
                 case 1: matrix.fillRect(32-6,0,6,3,GREEN); break;
                 case 2: matrix.fillRect(0,5,6,3,YELLOW); break;
                 case 3: matrix.fillRect(32-6,5,6,3,CYAN); break;
               
               }
          }
        matrix.setCursor(11, 0);
        matrix.print(count[0] + count[1] + count[2] + count[3]);  

        if(mode == 0){
          matrix.drawLine(0, 7, 31, 7, BLACK); //clear out
          matrix.drawLine(0, 7, (TARGET_COUNTS  - target_count) - 1, 7, RED);
        }

        matrix.show();
        
        if(mode == 0)
            target_count = target_count + 1;
            
        update_led = 0;
        strike = 0;   
    
        for( i = 0; i < 4; i++)
           max_dev[i] = cal[i];

        if(mode == 1)
          delay(HIT_DELAY);

        loop_time = millis();
        time_TARGET_DISPLAY = loop_time; 
        time_TARGET_SAMPLE = loop_time;
        
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

    flag_CONSTANT_DISPLAY_WINDOW = 0;
    flag_SEL_BUTTON_WINDOW = 0;
    flag_TARGET_SAMPLE_WINDOW = 0;
    flag_TARGET_DISPLAY_WINDOW = 0;
   
   if (abs(loop_time - time_CONSTANT_DISPLAY) > CONSTANT_DISPLAY_WINDOW){
       flag_CONSTANT_DISPLAY_WINDOW = 1;
       time_CONSTANT_DISPLAY = loop_time;
   }
  
   if (abs(loop_time - time_SEL_BUTTON) > SEL_BUTTON_WINDOW ){
       flag_SEL_BUTTON_WINDOW = 1;
       time_SEL_BUTTON = loop_time;
   }
 
   if(abs(loop_time - time_TARGET_SAMPLE) > TARGET_SAMPLE_WINDOW){
       flag_TARGET_SAMPLE_WINDOW = 1;
       time_TARGET_SAMPLE = loop_time;
   }else if(abs(loop_time - time_TARGET_DISPLAY) > TARGET_DISPLAY_WINDOW){
             flag_TARGET_DISPLAY_WINDOW = 1;
             time_TARGET_DISPLAY = loop_time; 
             time_TARGET_SAMPLE = loop_time;
   }
   
   if(flag_SEL_BUTTON_WINDOW){
       
        if(sel_stat == 0xFFFF){
          
            mode = (mode + 1)%MODE_COUNT;
            sel_stat = 0;
            sprintf(msg, "MODE = %5u", mode);  
            lcd.clear();          
            lcd.print(msg);  

            strike = 0;
            target_count = 0;
            
            for( i = 0; i < 4; i++)
              count[i] = 0;
        
            matrix.fillScreen(0);    //Turn off all the LEDs
            matrix.show();
  
            delay(SEL_BUTTON_WINDOW);  //delay for release
                              
            update_display();

            if (mode < 2)
              update_led = 1;
              
            return;
                
        }
        
   }
     
    if( flag_TARGET_SAMPLE_WINDOW ){

        the_max_idx = 0;
        test_max = 0.0;

        //choose the max deviated sensor, as a percentage of test value
        for( i = 0; i < 4; i++){
            
            test = abs(cal[i] - max_dev[i]) / TEST_VAL[i];
            
            if( test > test_max){
                test_max = test;
                the_max_idx = i;
            }
            
        }

        // if SEL button pressed during the window, push in a 1 to sel_stat
        if(analogRead(A0) < 800)
           sel_stat = (sel_stat << 1)|1;
        else
           sel_stat = 0;

    }  
   
   if(mode == 0){
             
       if(flag_TARGET_DISPLAY_WINDOW)
           update_led = 1;
              
       if(flag_TARGET_SAMPLE_WINDOW)    {  
        
        if(( abs(cal[the_max_idx] - max_dev[the_max_idx]) > TEST_VAL[the_max_idx] ) && (the_max_idx == the_rand ) && (strike == 0)){

            if(abs(loop_time - time_TARGET_DISPLAY) < (TARGET_DISPLAY_WINDOW/4 + TARGET_SAMPLE_WINDOW) )
              count[the_max_idx]+=3;
            else if(abs(loop_time - time_TARGET_DISPLAY) < (TARGET_DISPLAY_WINDOW/2 + TARGET_SAMPLE_WINDOW) )
                    count[the_max_idx]+=2;
                else
                  count[the_max_idx]+=1;
   
            matrix.fillScreen(0);    //Turn off all the LEDs

            matrix.setCursor(11, 0);
            matrix.print(count[0] + count[1] + count[2] + count[3]);  
            
            matrix.show();
            
            strike = 1;
            update_display();

            update_led = 1;
            delay(HIT_DELAY);
         
          }

          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i];  
            
       }
  
   }
   
   
   if(mode == 1){
       
       if(flag_TARGET_SAMPLE_WINDOW)    {  
       
          if( ( abs(cal[the_max_idx] - max_dev[the_max_idx]) > TEST_VAL[the_max_idx] ) && (the_max_idx == the_rand ) ){
              
            count[the_max_idx]++;    
            update_display();           
            update_led = 1;  
                      
          }
          
          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i]; 
        
       }
       
       
   }
   
  
   if(mode == 2){

        if(( flag_CONSTANT_DISPLAY_WINDOW )){
                  
               update_display();
               
               for( i = 0; i < 4; i++)
                max_dev[i] = cal[i];

                            
        }
        
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
