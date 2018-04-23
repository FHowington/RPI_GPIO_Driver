#include <SoftwareSerial.h>     // Serial comunication library
#include <Arduino_FreeRTOS.h>   // FreeRTOS
#include <semphr.h>             // Semaphores from FreeRTOS
#include <Dht11.h>              // Weather sensor library (DHT11)

#define TESTING 1

// Define pins
#define bl  8  // For the bell/horn
#define in1 7  // Right wheel, forwards
#define in2 6  // Right wheel, backwards
#define in3 5  // Left wheel, forwards
#define in4 4  // Left wheel, backwards
#define en1 9  // Left PWM
#define en2 10 // Right PWM
#define rst 12 // Range sensor trig
#define rse 13 // Range sensor echo
#define ws  2  // Temp/Humidity sensor

#define MS(x) x / portTICK_PERIOD_MS // Gives us time in ticks from ms

SemaphoreHandle_t xSerialSemaphore; // Only 1 task can use serial at a time
Dht11 dht(ws);                      // Interface for the weather sensor

void task_RecvInput(void *pvParameters);
void task_SendDistance(void *pvParameters);
void task_SendWeatherData(void *pvParameters);

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

  // Create tasks in the OS
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
              2,
              NULL);

  xTaskCreate(task_SendWeatherData,
              (const portCHAR *)"SendWeatherData",
              128,
              NULL,
              1,
              NULL);
}

void loop() {} // Instead, each task has its own loop

void task_SendDistance( void *pvParameters __attribute__((unused)) ) {
  #ifdef TESTING
    unsigned long start, finish, ttime, id; // For testing
    id = 0;
  #endif
  
  long elapsed, dist; // For the range sensor

  for (;;) {
    #ifdef TESTING
      start = micros();
    #endif
    
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
    // TODO: make critical section when printing?
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
      Serial.print("d");
      Serial.println(dist);
      Serial.flush();

      #ifdef TESTING
        finish = micros();
        ttime = finish-start;
        
        Serial.print("t");
        Serial.print(id++);
        Serial.print(",SendDistance,");
        Serial.print(start);
        Serial.print(",");
        Serial.print(finish);
        Serial.print(",");
        Serial.println(ttime);
        Serial.flush();
      #endif

      xSemaphoreGive(xSerialSemaphore);
    }
  
    vTaskDelay(10);
  }
}

void task_RecvInput( void *pvParameters __attribute__((unused)) ) {
  
  #ifdef TESTING
    unsigned long start, finish, ttime, id; // For testing
    id = 0;
  #endif
  
  for (;;) {

    #ifdef TESTING
    start = micros();
    #endif
  
    // Handle input coming over Bluetooth
    if (Serial.available() > 0) {
          
      char command = Serial.read();
      if (command == 'u') {
        analogWrite(en1, 255);
        analogWrite(en2, 200);
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
        analogWrite(en1, 200);
        analogWrite(en2, 200);
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
      }
    
      else {
        // Do nothing
      }
      
       vTaskDelay(MS(125));
    }

    // If no driving commands, just stop
    else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
    }

    #ifdef TESTING
      finish = micros();
      ttime = finish-start;
      
      if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 10) == pdTRUE) {
        Serial.print("t");
        Serial.print(id++);
        Serial.print(",RecvInput,");
        Serial.print(start);
        Serial.print(",");
        Serial.print(finish);
        Serial.print(",");
        Serial.println(ttime);
        Serial.flush();
        xSemaphoreGive(xSerialSemaphore);
      }
    #endif

    // Need this to reduce stutter
    vTaskDelay(10);

  }
}

// Periodic
void task_SendWeatherData( void *pvParameters __attribute__((unused)) ) {
  #ifdef TESTING
    unsigned long start, finish, ttime, id; // For testing
    id = 0;
  #endif
  
  TickType_t xLastWakeTime;               // The time at which the task was last unblocked
  const TickType_t xFreq = MS(30000);     // Cycle time period

  // Initialize xLastWakeTime with the current time in ticks
  xLastWakeTime = xTaskGetTickCount();
  
  for (;;) {
    // Make this task periodic. This will block for xFreq ticks
    vTaskDelayUntil(&xLastWakeTime, xFreq); 

    #ifdef TESTING
      start = micros();
    #endif

    // Read the weather data from pin
    switch (dht.read()) {
      case Dht11::OK:
        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
          Serial.print("w");
          Serial.print(dht.getTemperature());
          Serial.print((char)223); // degree symbol (for lcd)
          Serial.print("C, ");
          Serial.print(dht.getHumidity());
          Serial.println("%");
          Serial.flush();
          xSemaphoreGive(xSerialSemaphore);
        }
        
        break;

//      case Dht11::ERROR_CHECKSUM:
//        Serial.println("Checksum error");
//        break;
//
//      case Dht11::ERROR_TIMEOUT:
//        Serial.println("Timeout error");
//        break;

      default:
        //Serial.println("Unknown error");
        break;
    }



    #ifdef TESTING
      finish = micros();
      ttime = finish-start;
      
      if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 10) == pdTRUE) {
        Serial.print("t");
        Serial.print(id++);
        Serial.print(",SendWeatherData,");
        Serial.print(start);
        Serial.print(",");
        Serial.print(finish);
        Serial.print(",");
        Serial.println(ttime);
        Serial.flush();
        xSemaphoreGive(xSerialSemaphore);
      }
    #endif
    
  }
}
