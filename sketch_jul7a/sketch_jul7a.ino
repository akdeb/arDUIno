#define LED 2  

void setup() {
  // put your setup code here, to run once:
 pinMode(LED,OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
 delay(1000); // 500ms  
 digitalWrite(LED,HIGH); // Turn on LED  
 delay(200); // 500ms   
 digitalWrite(LED,LOW); // Turn off LED 
}
