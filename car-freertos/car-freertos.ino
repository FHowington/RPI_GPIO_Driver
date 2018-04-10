#include <SoftwareSerial.h>  
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define BLUETOOTH_RX 10
#define BLUETOOTH_TX 11
#define SENSOR_ECHO 0
#define SENSOR_TRIG 1

SoftwareSerial BT(BLUETOOTH_RX, BLUETOOTH_TX);
SemaphoreHandle_t xSerialSemaphore;

void task_GetDistance(void *pvParameters);
void task_RecvInput(void *pvParameters);
void task_SendDistance(void *pvParameters);

void setup() {
  pinMode(SENSOR_ECHO, INPUT);
  pinMode(SENSOR_TRIG, OUTPUT);
  
  BT.begin(9600);
  delay(1000);

  if (xSerialSemaphore == NULL) {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive(xSerialSemaphore);
  }

  xTaskCreate(task_GetDistance,
              (const portCHAR *)"GetDistance",
              128,
              NULL,
              1,
              NULL);

              
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

void loop() {

}

void task_GetDistance( void *pvParameters __attribute__((unused)) ) {

  int elapsed, dist;

  for (;;) {
    // Reset trig pin
    digitalWrite(SENSOR_TRIG, LOW);
    delayMicroseconds(2);
  
    // Sets the trig pin to high for 10 micro seconds
    // Tells sensor to send out a sonic burst
    digitalWrite(SENSOR_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(SENSOR_TRIG, LOW);
  
    // The sensor receives the sound wave, echo pin gives us 
    // the travel time in microseconds
    // TODO: Use interrupts
    elapsed = pulseIn(SENSOR_ECHO, HIGH);
  
    // 0.034 cm/us is speed of sound
    // dist is in centimeters
    dist = elapsed*0.034/2;
  
    // Send the distance over Bluetooth
    // TODO: Want to make own task, may need another semaphore for dist
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
      BT.print("Distance: ");
      BT.println(dist);
      BT.flush();
      xSemaphoreGive(xSerialSemaphore);
    }
  
    vTaskDelay(10);
  }
}

void task_RecvInput( void *pvParameters __attribute__((unused)) ) {
  for (;;) {
    if (BT.available() > 0) {
      if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
        BT.println("Message received!");
        BT.println(BT.readString());
        BT.flush();
        xSemaphoreGive(xSerialSemaphore);
      }
    }

    vTaskDelay(10);
  }
}

void task_SendDistance( void *pvParameters __attribute__((unused)) ) {
  
}
