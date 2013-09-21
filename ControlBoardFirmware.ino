/*
September 2013

sketch intended to run on joystick arduino with xbee on channel 1911 BAUD 19200



*/

#include <ValueSync.h>

byte led = 13;

//Value IDs (they are the pin numbers at the same time)
#define JOY_X A1
#define JOY_Y A0

#define JOY_FIRE_INDEX 6
#define JOY_FIRE_THUMB 7

#define SLIDER A2

#define POT_L A4
#define POT_R A5

#define SWITCH_L 3
#define SWITCH_R 2

byte t = 2;

//Here we sotre the current values
int c_joy_x;
int c_joy_y;

int c_joy_fire_index;
int c_joy_fire_thumb;
int c_slider; 

int c_pot_l;
int c_pot_r;

int c_switch_l;
int c_switch_r;

//Calibration Data Here
/*
int min_joy_x = 0, max_joy_x = 1024;
int min_joy_y = 0, max_joy_y = 1024;
int min_slider = 0, max_slider = 1024;
int min_pot_l = 0, max_pot_l = 1024;
int min_pot_r = 0, max_pot_r = 1024;
*/

int min_joy_x = 57, max_joy_x = 933;
int min_joy_y = 74, max_joy_y = 962;
int min_slider = 0, max_slider = 873;
int min_pot_l = 27, max_pot_l = 1022;
int min_pot_r = 43, max_pot_r = 1022;


ValueSender<9> sender;  //defines number of values to sync

void setup() {                
  //Init the pins
  pinMode(led, OUTPUT);     

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_FIRE_INDEX, INPUT);
  pinMode(JOY_FIRE_THUMB, INPUT);
  pinMode(SLIDER, INPUT);
  pinMode(POT_L, INPUT);
  pinMode(POT_R, INPUT);

  pinMode(SWITCH_L, INPUT);
  pinMode(SWITCH_R, INPUT);

  sender.addValue(&c_joy_x); //adds variable to be synced. This list must match the one on the other arduino
  sender.addValue(&c_joy_y);

  sender.addValue(&c_joy_fire_index);
  sender.addValue(&c_joy_fire_thumb);
  sender.addValue(&c_slider); 

  sender.addValue(&c_pot_l);
  sender.addValue(&c_pot_r);

  sender.addValue(&c_switch_l);
  sender.addValue(&c_switch_r);
  
  //Init the previous values
  readSensorValues();
  
  waitForSensorInit();

  Serial.begin(19200);
  
  if(!c_switch_l && !c_switch_r)
    doCalibration();


}

unsigned long lastCompleteUpdate = 0;

void loop() 
{
  readSensorValues();
  sender.syncValues();
  if(sender.timeSinceLastMessage() > 100)
    sender.sendKeepalive();  //transmitter failsafe
    
  
  if((sender.timeSinceLastMessage() > 20 && millis() - lastCompleteUpdate > 1000) || millis() - lastCompleteUpdate > 2000)
  {
    sender.sendAllValues();
    lastCompleteUpdate = millis();
  }
  
  delay(10);
}

void readSensorValues()
{
  c_joy_x = (analogRead(JOY_X) + c_joy_x)/2;
  c_joy_y = (analogRead(JOY_Y) + c_joy_y)/2;

  c_joy_fire_index = digitalRead(JOY_FIRE_INDEX);
  c_joy_fire_thumb = digitalRead(JOY_FIRE_THUMB);

  c_slider = (analogRead(SLIDER) + c_slider)/2;

  c_pot_l = (analogRead(POT_L) + c_pot_l)/2;
  c_pot_r = (analogRead(POT_R) + c_pot_r)/2;

  c_switch_l = digitalRead(SWITCH_L);
  c_switch_r = digitalRead(SWITCH_R);
}

void waitForSensorInit()
{
  while(millis() < 75) //we skip the first few sensor readings because they are just noise
  {
    readSensorValues();
    delay(10);
  }    
}

/*
Calibration is initiated by switching both switches off on startup. The procedure is:
- Do the joystick trim (so the values are around 512 when the joystick is on neutral)
- Press fire (with your index finger)
- Move all the sliders/knobs/joystick-axes all the way to both extremes so we can figure out the min/max values
- Press fire again
*/
void doCalibration()
{
  digitalWrite(13, HIGH);
  min_joy_x = 1024, max_joy_x = 0;
  min_joy_y = 1024, max_joy_y = 0;


  min_slider = 1024, max_slider = 0;

  min_pot_l = 1024, max_pot_l = 0;
  min_pot_r = 1024, max_pot_r = 0;
  Serial.println("Set the trim now");
  Serial.print("Joy x: ");
      Serial.print(c_joy_x);
      Serial.print("  y: ");
      Serial.println(c_joy_y);
  while(!c_joy_fire_index)
  {
    readSensorValues();
      Serial.print("Joy x: ");
      Serial.print(c_joy_x);
      Serial.print("  y: ");
      Serial.println(c_joy_y);
    delay(100);
  }

  while(!c_joy_fire_thumb)
  {
    readSensorValues();
    min_joy_x = min(c_joy_x, min_joy_x);
    min_joy_y = min(c_joy_y, min_joy_y);
    min_slider = min(c_slider, min_slider);
    min_pot_l = min(c_pot_l, min_pot_l);
    min_pot_r = min(c_pot_r, min_pot_r);
    
    max_joy_x = max(c_joy_x, max_joy_x);
    max_joy_y = max(c_joy_y, max_joy_y);
    max_slider = max(c_slider, max_slider);
    max_pot_l = max(c_pot_l, max_pot_l);
    max_pot_r = max(c_pot_r, max_pot_r);
    
    if(true)
    {
      Serial.print("int min_joy_x = "); Serial.print(min_joy_x); Serial.print(", max_joy_x = "); Serial.print(max_joy_x); Serial.println(";");
      Serial.print("int min_joy_y = "); Serial.print(min_joy_y); Serial.print(", max_joy_y = "); Serial.print(max_joy_y); Serial.println(";");  
  
      Serial.print("int min_slider = "); Serial.print(min_slider); Serial.print(", max_slider = "); Serial.print(max_slider); Serial.println(";");
  
      Serial.print("int min_pot_l = "); Serial.print(min_pot_l); Serial.print(", max_pot_l = "); Serial.print(max_pot_l); Serial.println(";");
      Serial.print("int min_pot_r = "); Serial.print(min_pot_r); Serial.print(", max_pot_r = "); Serial.print(max_pot_r); Serial.println(";");
      delay(100);
    }
    else
    {
      delay(10);
    }
  }
  digitalWrite(13, LOW);
}
