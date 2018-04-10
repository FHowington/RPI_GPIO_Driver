// Assuming hardware serial
#define in1 7
#define in2 6
#define in3 5
#define in4 4
#define en1 9  // Left
#define en2 10 // Right

void setup()
{ 
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);
  
  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  Serial.println("Testing 1, 2, 3.......");
  
  if (Serial.available() > 0) {
    Serial.println("Message received!");

    // TODO: erase low writes
    char command = Serial.read();
    if (command == 'u') {
      Serial.println("UP");
      analogWrite(en1, 255);
      analogWrite(en2, 200);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
    }

    else if (command == 'd') {
      Serial.println("DOWN");
      analogWrite(en1, 255);
      analogWrite(en2, 200);
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
    }

    else if (command == 'l') {
      Serial.println("LEFT");
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

    else {
      Serial.println(command);
    }
    delay(200);
  
  }
}



