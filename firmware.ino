/***********************
 * Satran Firmware 1.0
 * Creative Commons BY-NC-SA 2.0
 * Danaco 2021, Daniel Nikolajsen
 ***********************/
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);  

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0); 
String ssid;
String pass;
String ssidList;

#define AZ_RES 3.55 // Gear ratio, # of steps per degree
#define EL_RES 8 // Gear ratio, # of steps per degree

int count = 0;
String page = "";
String page2 = "";
String versioncode = "SATRAN firmware v1.0<br/>&copy; 2021-02-11 Danaco";
String formvalues = "<script>(function() { var queries = new URLSearchParams(window.location.search); var az = document.getElementById('az'); var el = document.getElementById('el'); az.value = queries.get('az'); el.value = queries.get('el');})();</script>";
String favicon = "data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiA/PjxzdmcgaWQ9IkxheWVyXzEiIHN0eWxlPSJlbmFibGUtYmFja2dyb3VuZDpuZXcgMCAwIDI0IDI0OyIgdmVyc2lvbj0iMS4xIiB2aWV3Qm94PSIwIDAgMjQgMjQiIHhtbDpzcGFjZT0icHJlc2VydmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgeG1sbnM6eGxpbms9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkveGxpbmsiPjxzdHlsZSB0eXBlPSJ0ZXh0L2NzcyI+Cgkuc3Qwe2ZpbGw6IzFFMjMyRDt9Cjwvc3R5bGU+PHBhdGggY2xhc3M9InN0MCIgZD0iTTEyLDR2MmMzLDAsNiwzLDYsNmgyQzIwLDgsMTYsNCwxMiw0eiBNMTIsMHYyYzYsMCwxMCw0LDEwLDEwaDJDMjQsNSwxOSwwLDEyLDB6IE0xMy41LDExLjlsMS4yLTEuMiAgYzAuNC0wLjQsMC40LTEsMC0xLjRjLTAuNC0wLjQtMS0wLjQtMS40LDBsLTEuMiwxLjJDOS41LDguMSw2LjMsNi4zLDIuNyw1LjVMMiw1LjRMMS42LDZDMC42LDcuOCwwLDkuOSwwLDEyYzAsNi42LDUuNCwxMiwxMiwxMiAgYzIuMSwwLDQuMi0wLjYsNi0xLjZsMC42LTAuNGwtMC4yLTAuN0MxNy43LDE3LjcsMTUuOSwxNC41LDEzLjUsMTEuOXogTTEyLDIyQzYuNSwyMiwyLDE3LjUsMiwxMmMwLTEuNSwwLjMtMywxLTQuMyAgQzkuNSw5LjQsMTQuNiwxNC41LDE2LjMsMjFDMTUsMjEuNywxMy41LDIyLDEyLDIyeiIvPjwvc3ZnPg==";
String pageheader = "<!DOCTYPE html><html><head><title>SATRAN v1</title><meta name='viewport' content='width=device-width, initial-scale=1.0'><link rel='icon' href='"+favicon+"'><style>body{font-family:verdana,helvetica,sans-serif;padding:25px;}input,select{width:100%;max-width:350px;padding:6px 12px;margin-bottom:10px; box-sizing: border-box;}#submit{margin:20px auto;}</style></head>";
String error = "";

int LED = 2; // PIN for onboard LED
int ElPos = 1234; // Measured position in degrees
int ElState = 0;
int ElEnd = 4; // PIN for elevation limit switch
int ElStep = 14; // PIN for motor driver
int ElDir = 15; // PIN for motor driver
int ResetState = 1;
int ResetBtn = 0; // PIN for reset button
int AzPos = 1234; // Measured position in degrees
int AzState = 0;
int AzEnd = 5; // PIN for azimuth limit switch
int AzStep = 12; // PIN for motor driver
int AzDir = 13; // PIN for motor driver
int conn_count = 0;
  
void setup(void){

  Serial.begin(115200);
  EEPROM.begin(512);
  delay(100);  

  /* USED TO CLEAR THE EEPROM ON FIRST FLASH ONLY OF THE NodeMCU 
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
  }
  delay(200);
  EEPROM.commit(); 
  delay(200);  */
   
 if (EEPROM.read(0)+EEPROM.read(1) > 0) {
    Serial.println("Reading saved wifi credentials");
    // First two bytes contains length of string
    int len = char(EEPROM.read(0))*10 + char(EEPROM.read(1));
    len = len + 2;
    for (int i = 2; i < len; ++i) {
      ssid += char(EEPROM.read(i));
    }
    len = char(EEPROM.read(32))*10 + char(EEPROM.read(33));
    len = len + 34;
    for (int i = 34; i < len; ++i) {
      pass += char(EEPROM.read(i));
    }
  } 
  delay(100);
  
  if (ssid.length()>0 && pass.length()>0) {
    
    pinMode(LED, OUTPUT);   
    pinMode(AzStep, OUTPUT);  
    pinMode(AzDir, OUTPUT); 
    pinMode(ElStep, OUTPUT);  
    pinMode(ElDir, OUTPUT); 
    pinMode(AzEnd, INPUT);  
    pinMode(ElEnd, INPUT); 
    pinMode(ResetBtn, INPUT);
    digitalWrite(LED, HIGH); 
    digitalWrite(AzDir, LOW);
    digitalWrite(ElDir, LOW);
   
    delay(50);
    WiFi.hostname("SATRAN"); 
    WiFi.mode(WIFI_STA); // Disable access point
    WiFi.begin(ssid, pass); //begin WiFi connection
    Serial.print("Connecting to wlan "+ssid);
 
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED && conn_count < 25) {
      delay(1000);
      Serial.print(".");
      conn_count++;
    }
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("");
      Serial.print("Connected with IP ");
      Serial.print(WiFi.localIP());
      Serial.println("");
    
      // INITIALIZE THE ELEVATION
      ElState = digitalRead(ElEnd);
      if (ElState == LOW) { 
        // If elevation is low (outside limit)-> First try to turn up
        digitalWrite(ElDir, HIGH);
        int n = 0;
        while(ElState == LOW && n < 200){
          digitalWrite(ElStep, HIGH);
          delay(10);
          digitalWrite(ElStep, LOW);
          delay(10);
          ElState = digitalRead(ElEnd);
          n++;
        }

        Serial.println("Could not turn antenna up");
        
        ElState = digitalRead(ElEnd);
      
        // But if it didnt work, Turn antenna down instead
        if(ElState == LOW){
          digitalWrite(ElDir, LOW);
          int n = 0;
          while(ElState == LOW && n < 600){
           digitalWrite(ElStep, HIGH);
           delay(10);
           digitalWrite(ElStep, LOW);
           delay(10);
           ElState = digitalRead(ElEnd);
           n++;
         }
         Serial.println("Turned antenna down");
         
         // Continue turning within limit
         n = 0;
         while(ElState == HIGH && n < 4000){
           digitalWrite(ElStep, HIGH);
           delay(10);
           digitalWrite(ElStep, LOW);
           delay(10);
           ElState = digitalRead(ElEnd);
           n++;
         }
          // Turn back until within limit again
          n = 0;
          digitalWrite(ElDir, HIGH);
          while(ElState == LOW && n < 50){
            digitalWrite(ElStep, HIGH);
            delay(10);
            digitalWrite(ElStep, LOW);
            delay(10);
            ElState = digitalRead(ElEnd);
            n++;
          }
          // Take an extra step as a precaution
          digitalWrite(ElStep, HIGH);
          delay(10);
          digitalWrite(ElStep, LOW);
        }
        if(ElState == HIGH){ ElPos = -12; } else { Serial.println("Error, cant initialize elevation"); error = "Error initializing elevation"; } 
      
      } else {
        // If elevation limit is high (within limit)-> Point down towards -12degrees
        int n = 0;
        digitalWrite(ElDir, LOW);
        while(ElState == HIGH && n < 4000){
         digitalWrite(ElStep, HIGH);
         delay(10);
         digitalWrite(ElStep, LOW);
         delay(10);
         ElState = digitalRead(ElEnd);
         n++;
        }
        // Continue if success, otherwise throw error
        if(ElState == LOW && n!=4000){
          // Turn back to within limit, only occurs when reached the end
          n = 0;
          digitalWrite(ElDir, HIGH);
          while(ElState == LOW && n < 50){
            digitalWrite(ElStep, HIGH);
            delay(10);
            digitalWrite(ElStep, LOW);
            delay(10);
            ElState = digitalRead(ElEnd);
            n++;
          }
          // Turn back two extra steps to make sure switch is enabled
          for(int i = 0;i<2;i++){
            digitalWrite(ElStep, HIGH);
            delay(10);
            digitalWrite(ElStep, LOW);
            delay(10);
          }
          ElPos = -12; // Save position 
         } else { error = "Error initializing elevation"; }
      
      } // End initialization elevation 

      // INITIALIZE AZIMUTH            
      AzState = digitalRead(AzEnd);
      if (AzState == HIGH) { 
        int n = 0;
        int seq = 800;
        boolean dir = 0;
        while(AzState == HIGH && seq < 10000){
          digitalWrite(AzDir, dir);
          for(int i = 0;i<seq;i++){
            if(AzState == HIGH){
              digitalWrite(AzStep, HIGH);
              delay(8);
              digitalWrite(AzStep, LOW);
              delay(9);
              AzState = digitalRead(AzEnd);
              n++;
            }
          }
          seq = seq + 800;
          dir = !dir; // Toggle direction
        } 
        if(AzState == LOW){ AzPos = 0; Serial.println("Azimuth intialized"); } else { Serial.println("Error, cant initialize azimuth"); error += "\nError initializing azimuth"; }
      } else {
        AzPos = 0;
        Serial.println("Azimuth intialized");
      }
      
    } // End if stuff to do only when have wifi connection

    // CREATE web page listeners and start server

    if(error==""){ 
        page = pageheader+"<body style='text-align:center;'><h1>SATRAN Rotator</h1><p><form action='./manual' method='get'>Azimuth (0 - 360)<br/><input type='number' id='az' name='az' value=\"$_GET['az']\"><br/>Elevation (-10 - 90)<br/><input type='number' id='el' name='el' value=\"$_GET['el']\"><br/><input type='submit' value='Execute'></form></p><p style='font-size:0.8em;color:#888;'>"+versioncode+"</p>"+formvalues+"</body></html>";
      } else { 
        page = pageheader+"<body style='text-align:center;'><h1>SATRAN Rotator</h1><p><strong>"+error+"</strong></p><p style='font-size:0.8em;color:#888;'>"+versioncode+"</p></body></html>";
      }
      
    server.on("/", [](){
      server.send(200, "text/html", page);
    });
    server.on("/ping", [](){
      server.send(200, "text/html", "SATRAN"); //WiFi.localIP()
    });
    server.on("/manual", [](){
      if(error==""){
        // Receive coordinates via GET and turn the rotor
        int azimuth = server.arg("az").toInt();
        int elevation = server.arg("el").toInt();
      
        int steps = 0;
        int targetsteps = (int) (azimuth-AzPos) * AZ_RES;
        targetsteps = abs(targetsteps);
        
        // AZIMUTH direction
        if(AzPos < azimuth){ digitalWrite(AzDir, LOW); } else { digitalWrite(AzDir, HIGH); }
        if(azimuth >= 0 && azimuth <= 360 && AzPos != azimuth){ // Validate input
          while(steps < targetsteps){
            digitalWrite(AzStep, HIGH);
            delay(10);
            digitalWrite(AzStep, LOW);
            delay(8);
            steps++;
            
            // If limit switch is reached
            AzState = digitalRead(AzEnd);
            if(AzState == LOW && steps > 60){
              // High or low depending on previous direction
              if(AzPos < azimuth){ 
                AzPos = 360; 
                // Reverse direction
                digitalWrite(AzDir, HIGH);
              } else { 
                AzPos = 0; 
                // Reverse direction
                digitalWrite(AzDir, LOW);
              }
              // Recalculate (settings have nudged and went too far)
              steps = 0;
              targetsteps = (int) (azimuth-AzPos) * AZ_RES;
              targetsteps = abs(targetsteps);
            }
         } 
         AzPos = azimuth;
       }
       
        // ELEVATION direction
        steps = 0;
        bool dirs = 0;
        if(ElPos < elevation){ digitalWrite(ElDir, HIGH); dirs = 1; } else { digitalWrite(ElDir, LOW); }
        if(elevation >= -12 && elevation <= 90){ // Validate input
          while(ElPos != elevation){
            digitalWrite(ElStep, HIGH);
            delay(9);
            digitalWrite(ElStep, LOW);
            delay(10);
            steps++;
            if(steps == EL_RES && ElPos < elevation){ steps=0; ElPos = ElPos+1; }
            if(steps == EL_RES && ElPos > elevation){ steps=0; ElPos = ElPos-1; }
            ElState = digitalRead(ElEnd);
            if(ElState == LOW){ 
             // If limit switch is reached, reset and reverse
              if(dirs == 0){ digitalWrite(ElDir, HIGH); ElPos = -12; steps=0; } 
              if(dirs == 1){ digitalWrite(ElDir, LOW); ElPos = 91; steps=0; } 
            }  
          }
        }
        server.send(200, "text/html", page);
      } else {
        server.send(200, "text/html", "Error, rotator was not able to initialize. Please restart or reset, and make sure all cables are connected.");
      }
    });
    server.on("/tracker", [](){
      if(error==""){
        // Receive coordinates via GET and turn the rotor
        int azimuth = server.arg("az").toInt();
        int elevation = server.arg("el").toInt();
      
        int steps = 0;
        int targetsteps = (int) (azimuth-AzPos) * AZ_RES;
        targetsteps = abs(targetsteps);
        
        // AZIMUTH direction
        if(AzPos < azimuth){ digitalWrite(AzDir, LOW); } else { digitalWrite(AzDir, HIGH); }
        if(azimuth >= 0 && azimuth <= 360 && AzPos != azimuth){ // Validate input
          while(steps < targetsteps){
            digitalWrite(AzStep, HIGH);
            delay(10);
            digitalWrite(AzStep, LOW);
            delay(8);
            steps++;
            
            // If limit switch is reached
            AzState = digitalRead(AzEnd);
            if(AzState == LOW && steps > 60){
              // High or low depending on previous direction
              if(AzPos < azimuth){ 
                AzPos = 360; 
                // Reverse direction
                digitalWrite(AzDir, HIGH);
              } else { 
                AzPos = 0; 
                // Reverse direction
                digitalWrite(AzDir, LOW);
              }
              // Recalculate (settings have nudged and went too far)
              steps = 0;
              targetsteps = (int) (azimuth-AzPos) * AZ_RES;
              targetsteps = abs(targetsteps);
            }
         } 
         AzPos = azimuth;
       }
       
        // ELEVATION direction
        steps = 0;
        bool dirs = 0;
        if(ElPos < elevation){ digitalWrite(ElDir, HIGH); dirs = 1; } else { digitalWrite(ElDir, LOW); }
        if(elevation >= -12 && elevation <= 90){ // Validate input
          while(ElPos != elevation){
            digitalWrite(ElStep, HIGH);
            delay(9);
            digitalWrite(ElStep, LOW);
            delay(10);
            steps++;
            if(steps == EL_RES && ElPos < elevation){ steps=0; ElPos = ElPos+1; }
            if(steps == EL_RES && ElPos > elevation){ steps=0; ElPos = ElPos-1; }
            ElState = digitalRead(ElEnd);
            if(ElState == LOW){ 
             // If limit switch is reached, reset and reverse
              if(dirs == 0){ digitalWrite(ElDir, HIGH); ElPos = -12; steps=0; } 
              if(dirs == 1){ digitalWrite(ElDir, LOW); ElPos = 91; steps=0; } 
            }  
          }
        }
        server.send(200, "text/html", "success");
      } else {
        server.send(404, "text/html", "Rotator not initialized");
      }
    });
    server.begin();
    Serial.println("Web server started!");

  } else {
    // SETUP PAGE AS ACCESS POINT

    // NETWORK LIST
    int n = WiFi.scanNetworks();
    delay(100);
    ssidList = "<select id=\"newssid\" name=\"newssid\">";
    for (int i = 0; i < n; ++i) {
      ssidList += "<option value=\"";
      ssidList += WiFi.SSID(i);
      ssidList += "\">";
      ssidList += WiFi.SSID(i);
      ssidList += "</option>";
    }
    ssidList += "</select>";
  
    page = pageheader + "<body style='text-align:center;'><h1>SATRAN Wifi Setup</h1><p><form action='/savesettings' method='get'>Network<br/>"+ssidList+"<br/><br/>Password<br/><input type='text' id='newpass' name='newpass'><br/><br/><input type='submit' id='submit' value='Save'></form>";
    page += "</p><p style='font-size:0.8em;color:#888;'>Can't find your access point?<br/> SATRAN only supports b/g/n-type networks.<br/><br/>"+versioncode+"</p></body></html>";
    WiFi.hostname("SATRAN"); 
    WiFi.mode(WIFI_AP); // Enable AP
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP("SATRAN_setup"); // Start AP
    Serial.println("Setup Access Point started");

    // SETUP PAGE after credentials sent
    page2 = pageheader + "<body style='text-align:center;'><em>Saving and restarting. Please connect your device back to your wifi.</em></body></html>";
      
    server.on("/", [](){
      server.send(200, "text/html", page);
    });
    server.on("/savesettings", [](){

      // Clear EEPROM memory 
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }

      // Clean input
      String ssid = server.arg("newssid");
      ssid.trim(); 
      String pass = server.arg("newpass");
      pass.trim(); 
      
      Serial.println("Saving Wifi Credentials");
      
      // Save SSID
      if(ssid.length()<10){
        EEPROM.write(0, 0); 
        EEPROM.write(1, ssid.length());
      } else {
        int firstchar = 0;
        int lastchar = 0;
        if(ssid.length()>20){ 
          /* 20-29 */
          firstchar = 2; 
          lastchar = ssid.length()-20; 
         } else { 
          /* 10-19 */ 
          firstchar = 1; 
          lastchar = ssid.length()-10; 
         }
        EEPROM.write(0, firstchar); 
        EEPROM.write(1, lastchar);
      }
      for (int i = 2; i < ssid.length()+2; ++i) {
        int n = i - 2;
        EEPROM.write(i, ssid[n]);
      }

      // Save password
      if(pass.length()<10){
        EEPROM.write(32, 0); 
        EEPROM.write(33, pass.length()); 
      } else {
        int firstchar = 0;
        int lastchar = 0;
        if(pass.length()>20){ 
          /* 20-29 */
          firstchar = 2; 
          lastchar = pass.length()-20; 
         } else { 
          /* 10-19 */ 
          firstchar = 1; 
          lastchar = pass.length()-10; 
         }
        EEPROM.write(32, firstchar); 
        EEPROM.write(33, lastchar);
      }
      for (int i = 34; i < pass.length()+34; ++i) {
        int n = i - 34;
        EEPROM.write(i, pass[n]);
      }
      EEPROM.commit();

      server.send(200, "text/html", page2);
      delay(1000);
      
      // Restart
      ESP.restart();
    });

    server.begin();
  }
}

/* Thread to read inputs, reset button */
void loop(void){
  server.handleClient();

  // Read state of reset button
  ResetState = digitalRead(ResetBtn);
  if (ResetState == LOW) {
    delay(2500);
    ResetState = digitalRead(ResetBtn);
    if(ResetState == LOW){
      // turn LED on
      digitalWrite(LED, LOW);
      
      // Reset EEPROM wifi configs
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit(); 

      delay(3000);
      // Turn off LED
      digitalWrite(LED, HIGH);

      // Restart
      ESP.restart();
    }
  }
  
}
