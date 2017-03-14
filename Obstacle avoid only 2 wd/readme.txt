it has 2 wheel and only one mode of operation, that is obstacle avoid. ultrasonic sensor continously rotates


Get the AFMotor Library :

https://github.com/adafruit/Adafruit-Motor-Shield-library

Get the NewPing Library :

https://bitbucket.org/teckel12/arduino-new-ping/downloads

References :

- https://learn.adafruit.com/adafruit-motor-shield/overview
- http://playground.arduino.cc/Code/NewPing


Descriptions:

In this tutorial, you will make obstacle avoiding robot. This tutorial involves building a 4WD robot with an ultrasonic sensor that can detect nearby objects and change its direction to avoid these objects. The ultrasonic sensor will be attached to a servo motor which is constantly scanning left and right looking for objects in its way.

------------------------------

A) Hardware Required :

- Arduino Board
- Motor Shield (Adafruit)
- Ultrasonic Sensor (HC-SR04)
- Mini Servo Motor (SG90)
- DC Motor x4
- Wheel x4
- Chassis plate
- Holder for HC-SR04
- Battery 9V x2
- Battery Buckle x2
- Wires
- Button
- Glue Gun

------------------------------

B) Create of the Chassis:

1) The DC motors attach to the chassis
2) The Arduino attach to the chassis
3) About the Motor Shield:

- The motor controllers on this shield are designed to run from 4.5V to 25V.
- There are two places you can get your motor 'high voltage supply' from. One is the DC jack on the Arduino board and the other is the 2-terminal block on the shield that is labeled EXT_PWR.
- To connect a motor, simply solder two wires to the terminals and then connect them to either the M1, M2, M3, or M4.
- Hobby servos are the easiest way to get going with motor control. They have a 3-pin 0.1" female header connection with +5V, ground and signal inputs. The motor shield simply brings out the 16bit PWM output lines to two 3-pin headers so that its easy to plug in and go.
- Power for the Servos comes from the Arduino's on-board 5V regulator, powered directly from the USB or DC power jack on the Arduino.

4) The Motor Shield attach to the Arduino.
5) The Servo motor attach to the chassis
6) The HC-SR04 holder attach to Servo
7) The HC-SR04 sensor attach to Servo

------------------------------

C) Connections :

1) DC Motors connect to the Motor Shield
2) Left Motors connect to M1 and M2 (If the motors run backwards, change the (+) and (-) connections)
3) Right Motors connect to M4 and M3
4) The Servo motor connect to the Motor Shield

- The Servo1 input is connected to the Arduino Digital10 input
- The Servo2 input is connected to the Arduino Digital9 input
- Power for the Servos comes from the Arduino's on-board 5V regulator, powered directly from the USB or DC power jack on the Arduino

5) The HC-SR04 Sensor connections

- The HC-SR04 Sensor VCC connect to the Arduino +5V
- The HC-SR04 Sensor GND connect to the Arduino Ground
- The HC-SR04 Sensor Trig connect to the Arduino Analog 0
- The HC-SR04 Sensor Echo connect to the Arduino Analog 1

6) Battery and on / off button connection

- The (+) from the battery is connect to a leg of the button
- Connect a cable to the other legs of the button. This cable is for Motor Shield (+)

7) 2 seperate DC power supplies for the Arduino and motors

- If you would like to have 2 seperate DC power supplies for the Arduino and motors. Plug in the supply for the Arduino into the DC jack, and connect the motor supply to the PWR_EXT block. Make sure the jumper is removed from the motor shield.

8) Attach the wheels

------------------------------
