#include <Servo.h>

Servo myServo;

char incomingByte = 0;    //For incoming serial data
int ledPin = 9;           //LED Pin
int servoPin = 10;        //Servo Pin
int brightness;           //LED Brightness Value
int angle;                //Servo Angle

void setup()
{

  myServo.attach(servoPin);
  Serial.begin(9600);

}

void loop()
{

  if (Serial.available() > 0)
    {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
        {
      case '1':
        bright = 20;
        angle = 20;
        break;
      case '2':
        bright = 70;
        angle = 55;
        break;
      case '3':
        bright = 130;
        angle = 90;
        break;
      case '4':
        bright = 200;
        angle = 125;
        break;
      case '5':
        bright = 255;
        angle = 160;
        break;

      default:
        bright = 0;
        angle = 20;
        break;
        }

      analogWrite(ledPin, bright);
    }

    myServo.write(angle);
}
