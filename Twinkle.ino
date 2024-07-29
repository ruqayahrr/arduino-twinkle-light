 const char* VERSION_STRING = "Lights Twinkler";

//Light mode values. Each light will have one of these modes defined
#define UP 2
#define DOWN 3

#define POTENTIOMETER_PIN A0

#define MAX_PWM 255.0
#define MIN_PWM 120.0

//Program modes
#define PROG_NORMAL 1
#define PROG_ON 2
#define PROG_OFF 3
#define PROG_SYNC 4
#define PROG_SLOW 5

// Define structure associated with string of lights
struct LIGHT_STRUCT {
  int pinNumber;                  // Specifies the pin number to which the light (or light driver) is attached
  int PWMValue ;                  // Contains this light's PWM value from 0 (off) to 1023 (full on).
  int currentMode;                // Current mode for light string
  int msCycle;                    // milliseconds per cycle
  unsigned long nextStep;         // milliseconds to start next step on
};

// define the data for each led string
LIGHT_STRUCT lights[] = {
  3, 120, UP, 131, 0,
  5, 120, UP, 153, 0,
  6, 120, UP, 185, 0,
  9, 120, UP, 211, 0,
  10, 120, UP, 242, 0,
  11, 120, UP, 145, 0
};

// Define how many strings of lights
#define LIGHT_COUNT 8

unsigned long progress = 0;
unsigned long currentMillis;
int l;
float f1;
int sensorValue = 0;

int minValue = 120;
int maxValue = 255;

char inByte;
String command;

// Define the program mode
int programMode = PROG_NORMAL;

// Used during testing
// unsigned long temp1 = 0;
// int temp2 = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Starting ");
  Serial.println(VERSION_STRING);
  for (l = 0; l < LIGHT_COUNT; l++) {
    pinMode(lights[l].pinNumber, OUTPUT);
    lights[l].nextStep = currentMillis;
  }
}

void loop() {
  doSerial();
  // put your main code here, to run repeatedly:
  currentMillis = millis();

  // Used during testing
  // if (currentMillis > temp1) {
  //  temp2++;
  //  temp1 = currentMillis + 2000;
  //  Serial.print("Progress ");
  //  Serial.println(temp2);
  // }

  // Read the input from the potentiometer and work out a multiplier for dimming
  sensorValue = analogRead(POTENTIOMETER_PIN);
  maxValue = (MAX_PWM * sensorValue) / 1023;
  minValue = (MIN_PWM * sensorValue) / 1023;

  switch (programMode) {
    case PROG_NORMAL: // NORMAL mode
      for (l = 0; l < LIGHT_COUNT; l++) {
        // This check is necessary as the millisecond clock loops back to 0 after 35 days
        // or 4,294,967,295 milliseconds
        if (lights[l].nextStep > currentMillis + 4000000000) {
          lights[l].nextStep = currentMillis;
        }
        
        // Milliseconds to next change.
        if (lights[l].nextStep > currentMillis) {
          progress = (lights[l].nextStep - currentMillis);
        }
        else {
          progress = lights[l].msCycle;
          lights[l].nextStep = currentMillis + lights[l].msCycle;
          if (lights[l].currentMode == UP) {
            lights[l].currentMode = DOWN;
            lights[l].PWMValue = maxValue;
          }
          else {
            lights[l].currentMode = UP;
            lights[l].PWMValue = minValue;
          }
        }
        if (lights[l].currentMode == UP) {
          lights[l].PWMValue = (maxValue - minValue) * progress / lights[l].msCycle;
        }
        else {
          lights[l].PWMValue = (maxValue - minValue) * ((lights[l].msCycle - progress) / lights[l].msCycle);
        }
        lights[l].PWMValue += minValue;
        analogWrite(lights[l].pinNumber, lights[l].PWMValue);
      }
      break;

    case PROG_ON:
      // turn all leds full on
      for (int l = 0; l < LIGHT_COUNT; l++) {
        analogWrite(lights[l].pinNumber, 255);
      }
      break;

    case PROG_OFF:
      // turn all leds off
      for (int l = 0; l < LIGHT_COUNT; l++) {
        analogWrite(lights[l].pinNumber, 0);
      }
      break;

  }
}


void doSerial () {
  // Input serial information:
  if (Serial.available() > 0) {
    inByte = Serial.read();
    // Serial.print(inByte);
    // only input if a letter, number, =,?,+ are typed!
    if ((inByte >= 65 && inByte <= 90) || (inByte >= 97 && inByte <= 122) || (inByte >= 48 && inByte <= 57) || inByte == 43 || inByte == 61 || inByte == 63) {
      command.concat(inByte);
    }
    if (inByte == 10 || inByte == 13) {
      // user hit enter or such like
      Serial.println(command);
      doCommandParser();
      // reset the command buffer
      command = "";
    }
  }// end serial.available
}

void doCommandParser()
// separated this for clarity
{
  command.toUpperCase();
  if (command == "NORMAL")
  {
    Serial.println("NORMAL mode");
    programMode = PROG_NORMAL;
  }
  else if (command == "ON")
  {
    Serial.println("Turning on");
    programMode = PROG_ON;
  }
  else if (command == "OFF")
  {
    Serial.println("Turning off");
    programMode = PROG_OFF;
  }
  else if (command == "SHOW")
  {
    Serial.print("Program mode ");
    switch (programMode) {
      case PROG_NORMAL :
        Serial.println("NORMAL");
        break;
      case PROG_ON :
        Serial.println("ON");
        break;
      case PROG_OFF :
        Serial.println("OFF");
        break;
    }
    // This info was mainly used during debugging
    float potValue = analogRead(POTENTIOMETER_PIN) * 100.0 / 1023.0;
    Serial.print("Potentiometer ");
    Serial.print(potValue);
    Serial.println("%");
    for (int l = 0; l < LIGHT_COUNT; l++) {
      Serial.print("LED No ");
      Serial.print(lights[l].pinNumber);
      Serial.print(" value ");
      Serial.print(lights[l].PWMValue);
      Serial.print(" Current mode ");
      Serial.println(lights[l].currentMode);
    }
  }
  else
  {
    Serial.println("Commands are");
    Serial.println("ON - all full on");
    Serial.println("OFF - all full off");
    Serial.println("NORMAL -Normal operation");
    Serial.println("SHOW - show key values");
  }

}
