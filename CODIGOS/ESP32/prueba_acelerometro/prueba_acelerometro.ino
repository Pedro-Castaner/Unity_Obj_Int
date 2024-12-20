#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "SPI.h"

LSM6DS3 myIMU; //Default constructor is I2C, addr 0x6B

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
delay(1000); //relax...
Serial.println("Processor came out of reset.\n");

//Call .begin() to configure the IMU
myIMU.begin();

}


void loop()
{
//Get all parameters
Serial.print("\nAccelerometer:\n");
Serial.print(" X = ");
Serial.println(myIMU.readFloatAccelX(), 4);
Serial.print(" Y = ");
Serial.println(myIMU.readFloatAccelY(), 4);
Serial.print(" Z = ");
Serial.println(myIMU.readFloatAccelZ(), 4);

Serial.print("\nGyroscope:\n");
Serial.print(" X = ");
Serial.println(myIMU.readFloatGyroX(), 4);
Serial.print(" Y = ");
Serial.println(myIMU.readFloatGyroY(), 4);
Serial.print(" Z = ");
Serial.println(myIMU.readFloatGyroZ(), 4);

delay(500);
}
