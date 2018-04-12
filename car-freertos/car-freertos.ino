#include <SoftwareSerial.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Adafruit_Sensor.h> // Unified Sensor Library, for the weather sensor
#include <DHT.h>             // For the weather sensor
#include <DHT_U.h>           // Also for the weather sensor

#define in1 7 // Right wheel, forwards
#define in2 6 // Right wheel, backwards
#define in3 5 // Left wheel, forwards
#define in4 4 // Left wheel, backwards
#define en1 9  // Left PWM
#define en2 10 // Right PWM
#define rst 12 // Range sensor trig
#define rse 13 // Range sensor echo
#define ws  3 // Temp/Humidity sensor
#define DHTTYPE DHT11 // For the DHT library
#define DELAY(x) x / portTICK_PERIOD_MS // Gives us time in ticks from ms

SemaphoreHandle_t xSerialSemaphore;
DHT dht(ws, DHTTYPE);

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
  dht.begin();
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
              2,
              NULL);

  xTaskCreate(task_SendWeatherData,
              (const portCHAR *)"SendWeatherData",
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
          
      // TODO: erase low writes
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
      
       vTaskDelay(DELAY(100));
    }

    vTaskDelay(10);
  }
}

// Periodic
void task_SendWeatherData( void *pvParameters __attribute__((unused)) ) {
  TickType_t xLastWakeTime;     // The time at which the task was last unblocked
  const TickType_t xFreq = DELAY(60000);  // Cycle time period - 1 minute
  float hum, temp;
  char str_hum[6];
  char str_temp[6];
  
  for (;;) {
    // Make this task periodic. This will block for xFreq ticks
    vTaskDelayUntil(&xLastWakeTime, xFreq); 

    // Read in float values
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    // Convert floats to strings
    // Params: __val, __width, __prec, __s
    dtostrf(hum, 2, 0, str_hum);
    dtostrf(temp, 5, 1, str_temp);

    
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
      Serial.print(str_temp);
      Serial.print((char)223); // degree symbol (for lcd)
      Serial.print("C, ");
      Serial.print(str_hum);
      Serial.println("%");  
      Serial.flush();
      xSemaphoreGive(xSerialSemaphore);
    }
    
  }

  vTaskDelay(10);
}
