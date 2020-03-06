#include <SPI.h>
#include <Ethernet.h>

// 2-dimensional array of row pin numbers:
               int R[] = {2,7,A5,5,A0,A4,12,A2};  
// 2-dimensional array of column pin numbers:
int C[] = {6,11,A1,3,A3,4,8,9};    
//Start with No Alert Signals.
int isAlert=0;

//Network related vars
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 56);    //ASSIGN A IP FOR THIS DEVICE "AS WEBSERVER"
EthernetServer server(80);

  
unsigned char full_lights[8][8] =     
{  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
  1,1,1,1,1,1,1,1,  
};  
  
unsigned char half_lights[8][8] =    
{  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,1,1,1,1,0,0,  
  0,0,1,1,1,1,0,0,  
  0,0,1,1,1,1,0,0,  
  0,0,1,1,1,1,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
};  

unsigned char no_lights[8][8] =    
{  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,  
};  

void setup()  
{  
  Serial.begin(9600); //for debugging.
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  
  Ethernet.begin(mac, ip);


  // Check for Ethernet hardware present  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1000); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

     // iterate over the pins and set OUTPUT mode.
  for(int i = 0;i<8;i++)  
  // initialize the output pins:
  {  
    pinMode(R[i],OUTPUT);  
    pinMode(C[i],OUTPUT);  
  }  

  
}  


void loop()  
{
 if (isAlert == 1){
    alert();
    isAlert = 0;
 } else {
   _display(no_lights);
   checkForNewAlert(); //indefinitely waits as a listener.
 }
   
}  

void checkForNewAlert(){
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          isAlert = 1;
          Serial.println("Marking for Alert");
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("Alert received, Thank you.");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
   }
}

void alert(){
  int starttime = millis();
  int endtime = starttime;
  while ((endtime - starttime) <=10000) // do alerting for ~10 seconds.
  {
     for(int i = 0 ; i < 100 ; i++)       
     {  
       _display(full_lights);                   
     }  
     for(int i = 0 ; i < 50 ; i++)      
     {     
       _display(half_lights);
     }  
     endtime = millis();
  }
}
  
void _display(unsigned char dat[8][8])    
{  
  for(int c = 0; c<8;c++)  
  {  
    digitalWrite(C[c],LOW); 
    //loop
    for(int r = 0;r<8;r++)  
    {  
      digitalWrite(R[r],dat[r][c]);  
    }  
    delay(1);  
    Clear();  
  }  
}  
  
void Clear()                        
{  
  for(int i = 0;i<8;i++)  
  {  
    digitalWrite(R[i],LOW);  
    digitalWrite(C[i],HIGH);  
  }  
}  
