// Assuming hardware serial
#define in1 7
#define in2 6
#define in3 5
#define in4 4
#define en1 9  // Left
#define en2 10 // Right
#define rst 12 // Range sensor trig
#define rse 13 // Range sensor echo

long elapsed, dist;

void setup()
{ 
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);
  
  pinMode(rse, INPUT);
  pinMode(rst, OUTPUT);
  
  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  // Reset trig pin
  digitalWrite(rst, LOW);
  delayMicroseconds(2);

  // Sets the trig pin to high for 10 micro seconds
  // Tells sensor to send out a sonic burst
  digitalWrite(rst, HIGH);
  delayMicroseconds(10);
  digitalWrite(rst, LOW);

  // The sensor receives the sound wave, echo pin gives us 
  // the travel time in microseconds
  elapsed =   pulseIn(rse, HIGH,100000);

  // 0.034 cm/us is speed of sound
  // dist is in centimeters
  dist = elapsed*0.034/2;

  // Send the distance over Bluetooth
  Serial.print(dist);
  Serial.print('\n');

  
  if (Serial.available() > 0) {
    // TODO: erase low writes
    char command = Serial.read();
    if (command == 'u') {
      analogWrite(en1, 255);
      analogWrite(en2, 220);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
    }

    else if (command == 'd') {
      analogWrite(en1, 255);
      analogWrite(en2, 200);
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
    }

    else if (command == 'l') {
      analogWrite(en1, 200);
      analogWrite(en2, 200);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
    }

    else if (command == 'r') {
      Serial.println("RIGHT");
      analogWrite(en1, 200);
      analogWrite(en2, 200);
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
    }

    delay(250);
  
  }
}



