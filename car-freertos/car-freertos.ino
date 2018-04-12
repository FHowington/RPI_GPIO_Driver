#include <SoftwareSerial.h>  
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define in1 7 // Right wheel, forwards
#define in2 6 // Right wheel, backwards
#define in3 5 // Left wheel, forwards
#define in4 4 // Left wheel, backwards
#define en1 9  // Left PWM
#define en2 10 // Right PWM
#define rst 12 // Range sensor trig
#define rse 13 // Range sensor echo
#define DELAY(x) x / portTICK_PERIOD_MS // Gives us time in ms

SemaphoreHandle_t xSerialSemaphore;

void task_RecvInput(void *pvParameters);
void task_SendDistance(void *pvParameters);

void setup() {
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

  // Initialize the semaphore
  if (xSerialSemaphore == NULL) {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive(xSerialSemaphore);
  }

  // Create tasks
  xTaskCreate(task_RecvInput,
              (const portCHAR *)"RecvInput",
              128,
              NULL,
              3,
              NULL);
 
  xTaskCreate(task_SendDistance,
              (const portCHAR *)"SendDistance",
              128,
              NULL,
              1,
              NULL);
}

void loop() {}

void task_SendDistance( void *pvParameters __attribute__((unused)) ) {

  long elapsed, dist;

  for (;;) {
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
    // TODO: Use interrupts
    elapsed = pulseIn(rse, HIGH, 100000);
  
    // 0.034 cm/us is speed of sound
    // dist is in centimeters
    dist = elapsed*0.034/2;
  
    // Send the distance over Bluetooth
    // TODO: Want to make own task, may need another semaphore for dist
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
      Serial.println(dist);
      Serial.flush();
      xSemaphoreGive(xSerialSemaphore);
    }
  
    vTaskDelay(10);
  }
}

void task_RecvInput( void *pvParameters __attribute__((unused)) ) {
  for (;;) {

    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);

    // Handle input coming over Bluetooth
    if (Serial.available() > 0) {

      // TODO: Remove later
//      if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//        Serial.println("Message received!");
//        xSemaphoreGive(xSerialSemaphore);
//      }
          
      // TODO: erase low writes
      char command = Serial.read();
      if (command == 'u') {
        // TODO: Remove later
//        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//          Serial.println("UP");
//          xSemaphoreGive(xSerialSemaphore);
//        }
        analogWrite(en1, 255);
        analogWrite(en2, 200);
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
      }
    
      else if (command == 'd') {
        // TODO: Remove later
//        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//          Serial.println("DOWN");
//          xSemaphoreGive(xSerialSemaphore);
//        }
        analogWrite(en1, 255);
        analogWrite(en2, 200);
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
      }
    
      else if (command == 'l') {
        // TODO: Remove later
//        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//          Serial.println("LEFT");
//          xSemaphoreGive(xSerialSemaphore);
//        }
        analogWrite(en1, 200);
        analogWrite(en2, 200);
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
      }
    
      else if (command == 'r') {
        // TODO: Remove later
//        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//          Serial.println("RIGHT");
//          xSemaphoreGive(xSerialSemaphore);
//        }
        analogWrite(en1, 200);
        analogWrite(en2, 200);
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
      }
    
      else {
        // TODO: Remove later
//        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
//          Serial.println(command);
//          xSemaphoreGive(xSerialSemaphore);
//        }
      }
      
       vTaskDelay(DELAY(100));
    }

    vTaskDelay(10);
  }
}
