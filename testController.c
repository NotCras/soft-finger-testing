#include "mbed.h"
#include "TextLCD.h"

/********************MacValveControlwithLCD**************************
What this code will do is implement the previous Mac Valve control code for automated testing with an LCD screen
and the ability to determine the pressure control variable.

Automated test code will exist, however the next test phase will have to be initiated with a button press. Furthermore,
I will still keep the pressure ++ after every test, because I still want the code to be as automated as possible. 

I will also add in dual sensor capability, so I can get both flex sensor readings from one test. *Done
*/

LocalFileSystem local("local");

//analog out pin is 18 (the only analog out pin)
AnalogOut control(p18); 

//analog in pins are 19 and 20 (will read sensor values from here)
AnalogIn voltIn(p19);
AnalogIn voltIn2(p20);

//now to define the solenoids
DigitalOut solenoid1(p12);

//this is to define the replenish signal led
DigitalOut moveOn(LED1);

//these are the buttons which will add or subtract values to the p value after each test
DigitalIn pPlus(p13);
DigitalIn pMinus(p14);

//this is to define the button to move to the next psi step
InterruptIn nextStep(p5);

int p, canMove = 0;

//initialize serial for debugging
Serial pc(USBTX, USBRX); // tx, rx

//initialize LCD for testing
TextLCD lcd(p15, p16, p21, p22, p23, p24, TextLCD::LCD16x2); // rs, e, d4-d7

/*****************************************************INTERRUPT FUNCTIONS*********************************************************************/

//makes a variable flag indicate that the program can continue and take another measurement.
void next(){
    canMove = 1;
    pc.printf("canMove: %i", canMove);
}


/*****************************************************SUPPORT FUNCTIONS*********************************************************************/

//only works for pressure up to 33 psi (which is more than we'll need for one finger, so I will not address this problem)
float pToVal(int pressure){
    float c,e;
    //convert pressure to analog control value
    e = (float) pressure;
    c = e / 33; // (/10 to get voltage required, /3.3 to get control value)
    return c;
}

//begins displaying messages on the LCD
void startUpLCD(){
    lcd.cls();
    lcd.printf("Hello! Ready");
    lcd.locate(0,1);
    lcd.printf("to test?");
    wait(1);
    lcd.cls();
}

//updates screen with 'results pending'
void pending(){
    lcd.cls();
    lcd.printf("Results Pending.");        
}

//displays results from most recent test on lcd
void displayResults(float a, float b){
    char buffer[50];
    
    lcd.cls();
    
    //print first column
    lcd.locate(0,0);
    lcd.printf("S1");
    lcd.locate(5,0);
    lcd.printf(",");
    lcd.locate(10,0);
    lcd.printf("S2");
    
    //format data
    sprintf(buffer," %1.2f ,  %1.2f ", a, b);
    
    //print second column
    lcd.locate(0,1);
    lcd.printf(buffer);
}

void displayP(int c){   
    //show pressure, and nothing else
    lcd.locate(0,0);
    lcd.printf("P Value:            ");
    lcd.locate(0,1);
    lcd.printf("%2i             ", c);
}

/************************************************************MAIN FUNCTION************************************************************************/

int main() {
   //start LCD (this is really just for fun)
   startUpLCD();
   
   //pullup button and link interrupt port to interrupt function
   //nextStep.mode(PullUp);
   //pPlus.mode(PullUp);
   //pMinus.mode(PullUp); //do not need pullups
   nextStep.rise(&next);
   pc.printf("Button value: %i", canMove);
   
   float controlVal, read, read2;
   
   //Test File 1_______________________________________________________________________________________________________________
    FILE *fp1 = fopen("/local/test1_.txt", "w");  // Open "out.txt" on the local file system for writing
    
    
    //Test File 2_______________________________________________________________________________________________________________
    FILE *fp2 = fopen("/local/test2_.txt", "w");  // Open "out.txt" on the local file system for writing
    
    for(p=0; p<=25; p++){
        
        while(!canMove){
           displayP(p);
           moveOn = 1;
           if(pPlus == 1){
               p++;
               //added in software debounce since my debouncing circuit doesn't seem to be working
               wait(0.2);
               if(p>=26){
                    p = 26;   
                }
           }
           if(pMinus == 1){
               p--;
               wait(0.2);
               if(p<=0){
                    p = 0;   
                } 
           } 
        }
        
        moveOn = 0;
        canMove = 0;
                      
        //get the value needed to control the pressure
        controlVal = pToVal(p);
        
        //tell LCD test is pending
        pending();
        
        //use value found to control proportional controller
        pc.printf("Pressure value: %i ", p);
        pc.printf("Control value used: %f ", controlVal);
        control.write(controlVal);
        
        //fill the finger
        solenoid1 = 1;
        pc.printf("Finger filled. ");
        wait(10);
        
        //read voltage in value
        read = voltIn.read();
        read2 = voltIn2.read();
        
        //take the value and record it in the txt file
        fprintf(fp1, "%i , %f\n", p, read);
        fprintf(fp2, "%i , %f\n", p, read2);
        
        //display values found
        displayResults(read, read2);
        
        //empty finger -> comment these lines out if you don't want the finger to empty
        control.write(0);
        wait(2);
                
    } 
    
    //close file     
    fclose(fp1);
    pc.printf("Test file 1 closed. ");  
    
    //close file     
    fclose(fp2);
    pc.printf("Test file 2 closed. ");  
}
