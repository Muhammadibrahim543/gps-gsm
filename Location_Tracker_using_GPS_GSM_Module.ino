/*
 © Tetra Pico
 https://www.facebook.com/tetra.pico/
 tetrapico4@gmail.com
 https://www.youtube.com/channel/UCVnAlFrK2mxVmnM_HTkAKlQ
*/

#include "TinyGPS++.h"
#include <SoftwareSerial.h>
SoftwareSerial GSM(8,9); // tx,rx

#define red 2
#define green 3 

float a,d;

bool  command_repeat; 
bool storage_type, sms_id;
bool sms_status, cell_no, garbage, msg_date, msg_content;



bool msg_type = true;

char string[50];
char msg[10]; 
byte b;
byte pos = 0;


byte count=0;
int last_sms_id = 0;
bool valid_sender = false;

void reset_string() {
  while (pos>0)
{
  string[pos]=NULL; // clearing the array
  pos--;
  }  
  pos=0;
}

void setup()
{
  GSM.begin(9600);        // GSM module baud rate  (bits/sec)
  Serial.begin(9600);     // arduino baud rate (bits/sec)

  
  pinMode(red, OUTPUT);    // setting pin 2 as an output pin
  pinMode(green, OUTPUT);  // setting pin 3 as an output pin
  
  GSM.println("AT+CMGF=1");
  delay(2000);
  
  //delay(10000); // better, but works at 2/5  seconds delay as well
  for (int i = 1; i <= 5; i++) {
    GSM.print("AT+CMGD="); // deleting previous messages
    GSM.println(i);
    delay(200);

    // Not really necessary but prevents the serial monitor from dropping any input
    while(GSM.available()) {
      Serial.write(GSM.read());   
  }
}
}
void loop()
{
    while(GSM.available()) {
   
    split_text(GSM.read());
  }
}

void split_text(byte b) {
  
  string[pos] = b;
  pos++;
 
  

  
  
   if (msg_type){
   
      if ( b == '\n' )
        reset_string();
      else {   
            
        if ( pos == 3 && strcmp(string, "AT+") == 0 ) {
          command_repeat = true;
           msg_type= false;
          
        }
        else if ( pos == 6 ) {
          
          if ( strcmp(string, "+CMTI:") == 0 ) {
            Serial.println("+CMTI:");
            storage_type = true;
             msg_type= false;
          }
          else if ( strcmp(string, "+CMGR:") == 0 ) {
            Serial.println("+CMGR:");            
            sms_status = true;
             msg_type= false;
          
          }
          reset_string();
        }
       
      }
     
    }
    

  else if(command_repeat)
    {
      if ( b == '\n' ) {
        
        msg_type= true;
        reset_string();
        command_repeat= false;
      }
    }
  

  else if(storage_type)
    {
      if ( b == ',' ) {
        Serial.print("MSG STORAGE : ");
        Serial.println(string);
        sms_id = true;
        storage_type = false;
        reset_string();
      }
    }
    

  else if(sms_id)
    {
      if ( b == '\n' ) {
        last_sms_id = atoi(string);
        Serial.print("MSG ID: ");
        Serial.println(last_sms_id);

        GSM.print("AT+CMGR=");
        GSM.println(last_sms_id);
        

        
        sms_id = false;
        msg_type = true;
        reset_string();
      }
    }
   
 
  else if(sms_status)
    {
      if ( b == ',' ) {
        Serial.print("MSG STATUS ");
        Serial.println(string);
        
        cell_no=true;
        sms_status= false;
        reset_string();
      }
    }
    

  else if (cell_no)
    {
      if ( b == ',' ) {
        Serial.print("CELL NUMBER: ");
        Serial.println(string);

        // to ensure valid sender uncomment these two lines   
        //valid_sender = false;
        //if ( strcmp(buffer, "\"01686412243\",") == 0 )
        valid_sender = true;

        
        garbage= true;
        cell_no=false;
        reset_string();
      }
    }
   

  else if(garbage)
    {
      if ( b == ',' ) {
        Serial.print("GARBAGE: ");
        Serial.println(string);
        
        msg_date=true;
        garbage= false;
        reset_string();
      }
    }
 

  
  else if(msg_date)
    {
      if ( b == '\n' ) {
        Serial.print("MSG DATE: ");
        Serial.println(string);
        
        msg_content = true;
        msg_date=false;
        reset_string();
      }
    }
   

  
  else if(msg_content)
    {
      if ( b == '\n' ) {
        Serial.print("MSG CONTENT: ");
        Serial.print(string);

        read_sms(string);

        GSM.print("AT+CMGD=");
        GSM.println(last_sms_id);
        

        msg_type = true;
        msg_content = false;
        reset_string();
      }
    }
    
  }




void read_sms(char msg[10]) {
   
   if ( msg[0] == 'R' ) {
    
      if ( msg[1] == '1' )
        digitalWrite(red, HIGH);
        
      else if(msg[1]=='0')
        digitalWrite(red, LOW);
    }

    else if (msg[0] == 'G')
    {
      delay(50);
      send_gps();
      
    }    
  }

void send_gps()
{
  GSM.end();
  SoftwareSerial serial_connection(10,11); //tx, rx
  TinyGPSPlus gps;  // GPS object to process the NMEA data

  Serial.begin(9600);                //This opens up communications to the Serial monitor in the Arduino IDE
  serial_connection.begin(9600);     //This opens up communications to the GPS
  Serial.println("GPS Start");      //Just show to the monitor that the sketch has started



  for (int i = 1; i <= 16; i++) {
    while (serial_connection.available()) //While there are incoming characters  from the GPS
    {
      gps.encode(serial_connection.read());  //This feeds the serial NMEA data into the library one char at a time

    }

    a = gps.location.lat();
    d = gps.location.lng();
    Serial.print("latt:");
    Serial.println(a, 6);
    Serial.print("long:");
    Serial.println(d, 6);
    delay(100) ;
    if (gps.location.isUpdated()) //This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
    {
      //Get the latest info from the gps object which it derived from the data sent by the GPS unit

    }

  }
  serial_connection.end();

  GSM.begin(9600);

  GSM.println("AT+CMGF=1");
  delay(500);
  
  GSM.println("AT+CMGS=\"01686412243\"");
  delay(500);

  GSM.print("Lattitude: ");
  GSM.println(a, 6);

  GSM.print(" Longtitude: ");
  GSM.println(d, 6);
  GSM.print("http://www.google.com/maps/place/");
  GSM.print(a, 6);
  GSM.print(",");
  GSM.println(d, 6);
  GSM .write( 0x1a ); // ctrl+Z character


  delay(500);
}
