/******---------------------------------------------******
    Gain Control for Acoustic Microscope Pulser-Receiver
    Hold pushbutton to select between amplifier 1 and amplifier 2 using pot
    Release pushbutton
    Turn potentiometer to select the value of gain setting
    Quickly press pushbutton to set gain value
    
    Written by: Francia Dauz 
    29Feb2016
******---------------------------------------------******/
//pin definitions
const int sync= 9; //pin for 100Hz pulse
const int Gain_CLOCK =12; //pin SCK
const int Gain_S1=13; //Slave Select
const int Gain_S2=10;// pin 9 for CS2
const int Gain_DATA=11;//MOSI
const int pb=2;//pushbutton pin
const int Gain_pot=A0;//change gain control


boolean amp_sel=0;//0 for amp1 , 1 for amp2
byte gain=0;
boolean buttonState=LOW;
boolean s0=1;
boolean s1=0;
boolean cs=0;//chip select flag

unsigned long previousMillis=0;
const long interval=10;
boolean sync_flag=1;



void setup() {

  pinMode(sync,OUTPUT);
  pinMode(Gain_CLOCK,OUTPUT);
  pinMode(Gain_S1,OUTPUT);
  pinMode(Gain_S2,OUTPUT);
  pinMode(Gain_DATA,OUTPUT);

  pinMode(pb, INPUT);
  pinMode(Gain_pot,INPUT);

  
  digitalWrite(Gain_S1,HIGH);//high to disable chip select for both amplifiers
  digitalWrite(Gain_S2,HIGH);

  Serial.begin(9600);
}

void clearLCD(){
   Serial.write(0xFE);   //command flag
   Serial.write(0x01);   //clear command.
   delay(10);
}

void selectLineOne(){  //puts the cursor at line 0 char 0.
   Serial.write(0xFE);   //command flag
   Serial.write(128);    //position
   delay(10);
}

void selectLineTwo(){  //puts the cursor at line 0 char 0.
   Serial.write(0xFE);   //command flag
   Serial.write(192);    //position
   delay(10);
}



 // create pulses 100 Hz
void sync_start(void){
  digitalWrite(sync,HIGH);
  delayMicroseconds(1);
  digitalWrite(sync,LOW);
}

//change gain setting
void writeGain(boolean amp_sel, boolean button){
  int gain_raw=0;
  boolean set=0;
  while(s1==1){
    while(( button == LOW)){
      clearLCD();
      gain_raw=analogRead(A0);
      gain_raw=map(gain_raw,0,1023,0,15);
      selectLineOne();//first row
      Serial.print("Set gain to");
      selectLineTwo();//second row
      Serial.print(gain_raw);
      if(digitalRead(pb)==HIGH){
        button=HIGH;
        gain=(byte)gain_raw;
        s1=0;
        set=1;
      }
      delay(20);
    }
  }

  //write gain on amp1
  if (set==1 && button==HIGH){
    if(amp_sel==0){
        digitalWrite(Gain_S1,LOW);
        //write data to amp1
        for(int j=3;j>=0;j--){
          if (bitRead(gain,j)){
            digitalWrite(Gain_DATA,HIGH);
            delayMicroseconds(1);
          }
          else {
            digitalWrite(Gain_DATA, LOW);
            delayMicroseconds(1);
          }
          digitalWrite(Gain_CLOCK, HIGH);
          delayMicroseconds(1);
          digitalWrite(Gain_CLOCK,LOW);
        }
          digitalWrite(Gain_S1,HIGH);
          clearLCD();
          selectLineOne();
          Serial.print("gain1 set to:");
          Serial.print(gain);
          selectLineTwo();
          Serial.print(gain,BIN);
          delay(2000);
          clearLCD();
          s0=1;
          sync_flag=1;
    }

    //write gain on amp2
    else{
        digitalWrite(Gain_S2,LOW);
        //write data to amp2
        for (int i=3; i>=0; i--){
          if (bitRead(gain,i)){
            digitalWrite(Gain_DATA,HIGH);
            delayMicroseconds(1);
          }
          else {
            digitalWrite(Gain_DATA,LOW);
            delayMicroseconds(1);
          }
          digitalWrite(Gain_CLOCK, HIGH);
          delayMicroseconds(1);
          digitalWrite(Gain_CLOCK,LOW);
        }
          digitalWrite(Gain_S2,HIGH);
          clearLCD();
          selectLineOne();
          Serial.print("gain2 set to:");
          selectLineTwo();
          Serial.print(gain,BIN);
          delay(2000);
          clearLCD();
          s0=1;
          sync_flag=1;
    } 
  }
}

void loop() {
  delay(100);

  //Set SYNC signal continuously
  while(s0==1){
     while (sync_flag==1 && digitalRead(pb)==LOW){
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      sync_start();   
      }
    }

    //choose between amp1 or amp2
    buttonState=digitalRead(pb);
      
      while (buttonState==HIGH){
        sync_flag=0;
        clearLCD();
        if (analogRead(A0) < (1024/2)) {
          Serial.print("amp1 selected");
          cs=0; //chip select for amp1
          delay(20);
        }
        else {
          Serial.print("amp2 selected");
          cs=1;
          delay(20);
        }
        if (digitalRead(pb)==LOW){
          s0=0;
          s1=1;//activate state 1
          delay(10);
          buttonState=LOW;
          writeGain(cs,buttonState);
      }     
    }
  }
}
