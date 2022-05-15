#include <SPI.h> //RFID
#include <MFRC522.h> //RFID
//#include <Wire.h> 
#include <LiquidCrystal_I2C.h> //use with i2c
#include <Servo.h>


#define RST_PIN 49  //GPIO22
#define SS_PIN 53  //GPIO21

MFRC522 mfrc522(SS_PIN, RST_PIN);  //RFID

LiquidCrystal_I2C lcd(0x27, 16, 2); //lcd_i2cy7u685
Servo myservo; //servo

const int btn_mode = 11; //yellow; change mode
const int btn_time = 12; //green; change time only manually
const int btn_emer = 10; //red; emergency mode with led blinked; 
const int btn_holi = 8;
const int btn_unlock = 9;
const int buzzer = 7;
const int LDR1 = A0;
const int LDR2 = A1;
const int LDR3 = A2;
const int LDR4 = A3;
const int led_day1 = 36;
const int led_day2 = 38;
const int led_night1 = 37;
const int led_night2 = 39;
const int led_emer = 35;
int light_val_ave = 0;  //for LDR sensors
int light_val1, light_val2, light_val3, light_val4;
int i = 0;
int day = true;
int holiday = true;
int mode = 0;
int deg_unlock = 90;
int deg_lock = 180;

char Incoming_value = 0; //Bluetooth component that doesn't support in the simulator

const unsigned long eventInterval = 3000;
unsigned long previousTime = 0; //experiment the asynchorus programming
int servoo = -1;

const unsigned long eventInterval_2 = 500;
unsigned long previousTime_2 = 0; //experiment the asynchorus programming
int emerr = -1;

void setup() { 
  Serial.begin(9600);
  Serial1.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(5);
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID"));


  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Lightning System");
  lcd.setCursor(2, 3);
  lcd.print("Launching...");
  delay(2000);

  pinMode(btn_mode, INPUT_PULLUP);
  pinMode(btn_time, INPUT_PULLUP);
  pinMode(btn_emer, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(led_day1, OUTPUT);
  pinMode(led_day2, OUTPUT);
  pinMode(led_night1, OUTPUT);
  pinMode(led_night2, OUTPUT);
  pinMode(led_emer, OUTPUT);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Automatic");

  myservo.attach(2); //set servo to pin 
  myservo.write(deg_lock);
  delay(1000);

  digitalWrite(led_emer, HIGH);



}


void modee(int m) {
  lcd.setCursor(1, 0);
  lcd.print("                ");
  lcd.setCursor(1, 0);

  
  if (mode == 2) {
    mode = 0;
  }

  if (mode == 0) {
    lcd.print("Automatic");
  } else if (mode == 1) {
    lcd.print("Manual");
  }
  
  
}

void check_time() {
  lcd.setCursor(1, 1);
  if (day == true){
    lcd.print("Day  ");
  } else if (day == false) {
    lcd.print("Night");
  }
  lcd.setCursor(7, 1);
  if (holiday == true) {
    lcd.print("Holiday");
  } else if (holiday == false) {
    lcd.print("Work   ");
  }
}
void Emergency(){
    /*
  lcd.setCursor(1,0);
  lcd.print("Emergency");
  lcd.setCursor(1,1);
  lcd.print("                  ");

  for (i=0; i<10; i++) {
    digitalWrite(led_emer, LOW);
    tone(buzzer,300,150);
    delay(800);
    digitalWrite(led_emer, HIGH);
    delay(300);
  } */
  if (emerr == 1 || emerr == 0) {
    emerr = 2;
  } else if (emerr == -1) {
    emerr = 1;
  }
  

}



bool RFID_F(){
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return false;
        return;
    }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
        return false;
        return;
    }
    Serial.print("UID tag : ");
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++){
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
     
    }
    content.toUpperCase();
     if (content.substring(1) == "70 CF 89 32"){
      Serial.println("Access Granted"); 
      /*
      myservo.write(deg_unlock);
      delay(3000);
      myservo.write(deg_lock);*/
      servoo = 1;
    }

    else if (content.substring(1) == "C7 E6 9E 19"){
      if (day == true && holiday == false){
      Serial.println("Access Granted"); 
      /*
      myservo.write(deg_unlock);
      delay(3000);
      myservo.write(deg_lock);*/
      servoo = 1;
      } else {
        Serial.println("Access Denied");
        tone(buzzer,400,150);
      }
    }
    else{
      Serial.println("Access Denied");
      tone(buzzer,400,150);
  }

    mfrc522.PICC_HaltA();

}





void loop() {
  //LDR
  light_val1 = analogRead(LDR1);
  light_val2 = analogRead(LDR2);
  light_val3 = analogRead(LDR3);
  light_val4 = analogRead(LDR4);
  light_val_ave = (light_val1 + light_val2 + light_val3 + light_val4) / 4; 
/*
  Serial.println("Emergency: ");
  Serial.println(digitalRead(btn_emer));
  Serial.println("Mode: ");
  Serial.println(digitalRead(btn_mode));
  */

  //bluetooth compoennt
    //Serial.println(Serial1.available());



    if (Serial1.available() > 0) {
        Incoming_value = Serial1.read();
        if (Incoming_value == '1') { //change mode Automatic
              mode = 0;
              modee(mode);
              delay(50);
              
              
        } else if (Incoming_value == '2') { //change mode Manual
              mode = 1;
              modee(mode);
              delay(50);
              
        } else if (Incoming_value == '3'){ //change time day (manual)
          
          if (mode == 1) {
            day = true;  
          }
          
        } else if (Incoming_value == '4'){ //change time night (manual)  
           
          if (mode == 1) {
            day = false;    
          }
          
        } else if (Incoming_value == '5') { //change day holiday
          holiday = true;
          
        } else if (Incoming_value == '6') { //change day work
          holiday = false;
          
        } else if (Incoming_value == '7') {
          Emergency();
          check_time();
          modee(mode);
          delay(5);
        } 
    } 
  
  if (digitalRead(btn_emer) == HIGH){
      Emergency(); /*
      check_time();
      modee(mode); */
      while(digitalRead(btn_emer) == HIGH);
      delay(5);
        
  }

  else if (digitalRead(btn_mode) == HIGH) {
    mode++;
    modee(mode);
    while(digitalRead(btn_mode) == HIGH);
    delay(50);

  } else if (digitalRead(btn_mode) == LOW) {
    check_time(); //check time before run if condition

     


    if (digitalRead(btn_holi) == HIGH){
        
        if (holiday == true) {
          holiday = false;
        } else if (holiday == false) {
          holiday = true;
        }
        
        while(digitalRead(btn_holi) == HIGH);
        delay(50);
    }



    if (mode == 0) {
      /*
      Serial.print("Average: ");
      Serial.println(light_val_ave);
      Serial.print("1: ");
      Serial.println(light_val1);
      Serial.print("2: ");
      Serial.println(light_val2);
      Serial.print("3: ");
      Serial.println(light_val3);
            Serial.print("4: ");
      Serial.println(light_val4);
      */
      //Serial.println(light_val_ave);

      if (light_val_ave <= 110) {
        day = true;
      } else {
        day = false;
      }

      

    } else if (mode == 1) {

      
        
      if (digitalRead(btn_time) == HIGH) {
        //

        if (day == true){
          day = false;
        } else if (day == false) {
          day = true;
        }

        while(digitalRead(btn_time) == HIGH);
        delay(50);

        
      }

      
    } 
      
    }

   


  //servo part, led
  if (day == true) {
    //myservo.write(40);

    if (holiday == false) { //close only night
      digitalWrite(led_day1, LOW);
      digitalWrite(led_day2, LOW);
      digitalWrite(led_night1, HIGH);
      digitalWrite(led_night2, HIGH);

      


   

    } else if (holiday == true) { //clost all
      digitalWrite(led_day1, HIGH);
      digitalWrite(led_day2, HIGH);
      digitalWrite(led_night1, HIGH);
      digitalWrite(led_night2, HIGH);
    }

  } else if (day == false) {
    //myservo.write(90);
    digitalWrite(led_day1, HIGH);
    digitalWrite(led_day2, HIGH);
    digitalWrite(led_night1, LOW);
    digitalWrite(led_night2, LOW);
  } 

  RFID_F();

  if (digitalRead(btn_unlock) == HIGH) {
      /*myservo.write(deg_unlock);
      delay(3000);
      myservo.write(deg_lock); */
      servoo = 1;
      while(digitalRead(btn_unlock) == HIGH);

      
      
      
  }

  unsigned long currentTime = millis();

  //Serial.println(previousTime);
  if (servoo == 1 || servoo == 0) {
    /* This is the event */
  
    if (currentTime - previousTime >= eventInterval) { //eventInterval = 3000 x 2
      /* Event code */
      if (servoo == 1) {
        myservo.write(deg_unlock);
        servoo = 0;
      }
      else if (servoo == 0) {
        myservo.write(deg_lock);
        servoo = -1;
      }
      
      previousTime = currentTime;

      if (servoo == -1) {
        previousTime = 0;
      }
          
          
     /* Update the timing for the next time around */
      
    }
  }

  if (emerr == 1 || emerr == 0 || emerr == 2) {
    /* This is the event */
  
    if (currentTime - previousTime_2 >= eventInterval_2) { //eventInterval = 500 x 2
      /* Event code */
      if (emerr == 1) {
        
          digitalWrite(led_emer, LOW);
          tone(buzzer,300,400);
          //delay(800);
          
          //delay(300);

          emerr = 0;

      }
      else if (emerr == 0) {
        digitalWrite(led_emer, HIGH);
        emerr = 1;
      }
      
      previousTime_2 = currentTime;

      if (emerr == 2) {
        digitalWrite(led_emer, HIGH);
        emerr = -1;
      }
      
      if (emerr == -1) {
        previousTime_2 = 0;
      }
          
          
     /* Update the timing for the next time around */
      
    }
  }





}
