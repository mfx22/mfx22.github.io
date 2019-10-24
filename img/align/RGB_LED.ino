/*
   Michael Xiao (mfx2)
   10/7/19
   INFO 4320: Rapid Prototyping
   Design Project 1

   Digital Spirit Level with Memory

   Makes use of the IMU unit in the sensor ktt to serve as a level

   program starts in true level mode - meaning it is calibrated
   to find what is actually flat.  pressing the button can reset a
   new target level for the device to reach.  Perhaps you are
   woodworking and cut a 30 degree incline.  You could use your level
   on one piece and compare it to another piece to examine how close
   the two pieces are to being level relative to one another.

   The device takes two inputs, one digital from the user button and one
   analog from the I2C transmission from the IMU.

   The device gives user feedback using the multicolor LED with PWM RGB.
   red signals the y direction.  the brighter it is, the further you are
   away from the target y.  The blue signals the x direction.  the
   brighter it is, the further you are from the x direction.  As you get
   closer and closer to the target, both lights begin to dim.  When you
   the target level, a green light comes on indicating success.

   When setting a new target level, the user presses the button.  The LED
   shines bright white as a response to let the use know that a new
   target level has been set correctly.
*/

// --------------------- Libraries ---------------------
#include <analogWrite.h>
#include<Wire.h>


// --------------------- Global Variables ---------------------
// RGB LED pin declarations
const int red = 18;
const int green = 17;
const int blue = 16;

// two buttons for set/reset of target
const int button = 21;
const int button2 = 19;

// Potentiometer to control thresholding
const int pot = 15;  //A8
int threshold; //threshold to be controlled with Pot

int16_t targetX = 0;
int16_t targetY = 0;

/*
 * 
 * REFERENCE:
 * 
 * Setting up the GY-521 unit was done with reference to this article published
 * by Nicholas_N on the Arduino Forum:
 * 
 * https://create.arduino.cc/projecthub/Nicholas_N/how-to-use-the-accelerometer-gyroscope-gy-521-6dfc19
 * 
 */

// variables for IMU
const int MPU = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int16_t currentX;
int16_t currentY;

int diffX;
int diffY;

void setup() {
  // setup serial
  Serial.begin(9600);
  // setup pins
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(button, INPUT);

  // Wire setup
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

}

void loop() {

  // target set button
  // LED turns bright white to notify user this has been pressed
  if (digitalRead(button) == HIGH) {
    analogWrite(red, 255);
    analogWrite(blue, 255);
    analogWrite(green, 255);
    targetX = currentX;
    targetY = currentY;
  }

  // target reset button - sets target to true level
  // LED turns off to notify user this has been pressed
  else if (digitalRead(button2) == HIGH) {
    analogWrite(red, 0);
    analogWrite(blue, 0);
    analogWrite(green, 0);
    targetX = 0;
    targetY = 0;
  } else {

    // LEVEL toggling LEDs

    // Read I2C to receive gyroscope & accelerometer values
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12, true);
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();


    // for debugging
    //  Serial.print(AcX);
    //  Serial.print(",");
    //  Serial.println(AcY);

    // setting current X,Y orientation
    currentX = AcX;
    currentY = AcY;


    //diffX/Y represents how far away current orientation is from the target
    diffX = abs(currentX - targetX);

    diffY = abs(currentY - targetY);

    // debugging
    Serial.print(diffX);
    Serial.print(" , ");
    Serial.println(diffY);

    diffX = map(diffX, 0, 32000, 0, 255);

    diffY = map(diffY, 0, 32000, 0, 255);

    //adjusting colors to reflect the diff values
    analogWrite(blue, diffX);
    analogWrite(red, diffY);
    analogWrite(green, 0);


    // setting the threshold using the potentiometer
    threshold = analogRead(pot);
    threshold = map(threshold, 0, 4096, 4, 30);


    // if position is within the threshold, shine green light
    if (diffX < threshold && diffY < threshold) {
      analogWrite(blue, 0);
      analogWrite(red, 0);
      analogWrite(green, 255);
    }
  }

  delay(5);

}
