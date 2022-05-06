#include <LiquidCrystal.h>

//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 

const int TEST_A = 450; 
const int TEST_B = 450; 
const int TEST_C = 450; 
const int TEST_D = 450; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

long  the_rand, cal_accum[4];

int cal[4], meas[4], max_dev[4], count[4], test[4], outs[4], strike, target_count, update_led, mode;

const int CONSTANT_DISPLAY_TIME = 1500;
const int TARGET_DISPLAY_TIME = 5000; //MS
const int TARGET_COUNTS = 25; // # of targets to display
const int TARGET_SAMPLE_WINDOW = 200; //MS
const int START_DELAY = 5000;
const int MODE_COUNT = 3; 

unsigned int sel_stat;

void setup() {
    
    int i,j; 
     
    outs[0] = 0;
    outs[1] = 1;
    outs[2] = 2;
    outs[3] = 3;

    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT); 

    digitalWrite(0, LOW);
    digitalWrite(1, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);

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

    int i, the_max_idx, sel_button;
    long loop_time;
    char msg[17];

    // event based actions

    if( (target_count == TARGET_COUNTS) && (mode == 0) ){

       digitalWrite(outs[the_rand], LOW);   

       lcd.clear();
       lcd.setCursor(0,0); 
       lcd.print(count[0] + count[1] + count[2] + count[3] );  

       while(1){} //pause;
       
    }
  
    if(( ( strike || update_led )) ){

        digitalWrite(outs[the_rand], LOW);   
        the_rand = (the_rand + random(1,4))%4;
        digitalWrite(outs[the_rand], HIGH);
        strike = 0;

        target_count = target_count + 1;
        delay(10); 

        update_led = 0;
        strike = 0;   

        for( i = 0; i < 4; i++)
           max_dev[i] = cal[i];

    } 

    if( mode == 2 ){

      update_display();

    }
    
    
  
    //sample targets
    for( i = 0; i < 4; i++){

        meas[i] = analogRead(i+1);
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
            
     
            for( i = 0; i < 4; i++){
              count[i] = 0;
              digitalWrite(i, LOW);
            }
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
         
          if(( abs(cal[the_max_idx] - max_dev[the_max_idx]) > test[the_max_idx] ) && (the_max_idx == the_rand )){
          
            count[the_max_idx]++;
            strike = 1;
            update_display();
            delay(750);
            
          }

          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i]; 
          
       }

       if (mode == 1){    
         
          if(( abs(cal[the_max_idx] - max_dev[the_max_idx]) > test[the_max_idx] )){

            for( i = 0; i < 4; i++)
              digitalWrite(i, LOW);
              
            digitalWrite(outs[the_max_idx], HIGH);
            
            count[the_max_idx]++;
            update_display();
            delay(750);
            
          }
          
          for( i = 0; i < 4; i++)
            max_dev[i] = cal[i]; 
             
       }
     
       if(analogRead(0) < 800)
           sel_stat = (sel_stat << 1)|1;
       else
           sel_stat = 0;
       
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
