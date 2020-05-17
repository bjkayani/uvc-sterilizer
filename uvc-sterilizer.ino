#include <ButtonDebounce.h>
#define FAN 9
#define DOOR_LOCK 8
#define TEMP_SENS A7
#define LIGHT 7
#define SWITCH 3
#define DOOR_SENS 2

// timing
long lastControlTime = 0;
long lastActivationTime = 0;
#define CONTROL_LOOP_TIME 100
#define ACTIVATE_TIME 60000
// control
int fan = 0;
int doorLock = 0;
int light = 0;
#define LOWER_TEMP_THRES 6
#define UPPER_TEMP_THRES 7 
// sensors
float tempSens = 0;
int doorSens;
int buttonState;
// state
enum State_enum {REST, ACTIVATE, DEACTIVATE, ACTIVE};
uint8_t state = REST;

ButtonDebounce button(SWITCH, 250);

void setup() {
  pinMode(DOOR_LOCK, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(SWITCH, INPUT);
  pinMode(DOOR_SENS, INPUT);
  Serial.begin(9600);
  delay(1000);
  state = REST;
  
}

void loop() {
  button.update();
  if((millis()-lastControlTime)>CONTROL_LOOP_TIME){
    sensorUpdate();
    stateMachine();
    outputUpdate();
    lastControlTime = millis();
    Serial.println(tempSens);
    
  }
  // faster update bypass for quick response
  if(!doorSens){
    digitalWrite(LIGHT, LOW);
  }
}

void stateMachine(){
  switch(state){
    case REST:
      // activate on button press and door close
      if(buttonState && doorSens){
        state = ACTIVATE;
      }
      doorLock = 0;
      if(tempSens > LOWER_TEMP_THRES){
        fan = 1;
      }
      else if(tempSens < UPPER_TEMP_THRES){
        fan = 0;
      }
      //Serial.println("Rest State");
    break;
    case ACTIVATE:
      // start the timer
      lastActivationTime = millis();
      // turn on everything
      light = 1;
      state = ACTIVE;
      //Serial.println("Activate State");
    break;
    case ACTIVE:
      // check if timer has ran out
      if(((millis()-lastActivationTime)>ACTIVATE_TIME) || !doorSens){
        state = DEACTIVATE;
      }
      if(tempSens > LOWER_TEMP_THRES){
        fan = 1;
      }
      else if(tempSens < UPPER_TEMP_THRES){
        fan = 0;
      }
      //Serial.println("Active State");
    break;
    case DEACTIVATE:
    // turn off everything
    light = 0;
    int unlockTime = millis();
    doorLock = 1;
    state = REST;
    //Serial.println("Deactivate State");
    break;

  }
}

void sensorUpdate(){
  tempSens = 0;
  for(int i=0;i<=10;i++){
    tempSens = tempSens + analogRead(TEMP_SENS);
    delay(10);
  }
  tempSens = tempSens / 10.0;
  tempSens = 1024 / tempSens;
  doorSens = digitalRead(DOOR_SENS);

  buttonState = button.state();
}

void outputUpdate(){
  digitalWrite(FAN, fan);
  digitalWrite(LIGHT, light);
  digitalWrite(DOOR_LOCK, doorLock);
}
