#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>

const char * host = "esp32";
const char * ssid = "Rumen";
const char * ssid2 = "Softuni";
const char * password = "0898548033";
const char * password2 = "";

const int led = 32;
const int relay1 = 2;
const int relay2 = 15;
const int adc_v_pin = 34;
const int adc_i_pin = 35;

unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 1000; // interval at which to blink (milliseconds)

int ledState = LOW; // ledState used to set the LED
int relay1State = LOW; // relay1State
int relay2State = LOW; // relay2State

float voltages[10000]; // ADC values conatiner
int numberOfTests = 0; // number of tests
int delayBetweenTests = 10; // delay between every ADC test

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//web server
WebServer server(80);

//login page
const char * loginIndex =
  "<form name='loginForm'>"
"<table width='20%' bgcolor='A09F9F' align='center'>"
"<tr>"
"<td colspan=2>"
"<center><font size=4><b>ESP32 Login Page</b></font></center>"
"<br>"
"</td>"
"<br>"
"<br>"
"</tr>"
"<td>Username:</td>"
"<td><input type='text' size=25 name='userid'><br></td>"
"</tr>"
"<br>"
"<br>"
"<tr>"
"<td>Password:</td>"
"<td><input type='Password' size=25 name='pwd'><br></td>"
"<br>"
"<br>"
"</tr>"
"<tr>"
"<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
"</tr>"
"</table>"
"</form>"
"<script>"
"function check(form)"
"{"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{"
"window.open('/serverIndex')"
"}"
"else"
"{"
" alert('Error Password or Username')/*displays error message*/"
"}"
"}"
"</script>";

//server index - DASHBOARD
const char * serverIndex =
  "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css' integrity='sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm' crossorigin='anonymous'>"
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<script src='https://canvasjs.com/assets/script/canvasjs.min.js'></script>"
"<style> .btn {margin: 10px;} .cart{height: 300px !important;} </style>"
"<div class='card-columns' style='max-height: 240px; padding: 15px;'>"
"<div class='card'>"
"<div class='card-body'>"
"<h5 class='card-title'>LCD</h5>"
"<form method='GET' action='/lcd' id='lcd_from'>"
"<input type='text' maxlength='16' name='first_row' placeholder='First row text'><br><br>"
"<input type='text' maxlength='16' name='second_row' placeholder='Second row text'>"
"<button class='btn btn-primary' type='submit'>Show on LCD</button>"
"</form>"
"</div>"
"</div>"
"<div class='card'>"
"<div class='card-body'>"
"<h5 class='card-title'>Motor contoller</h5>"
"<a class='btn btn-primary' href='/motorLeft'>LEFT</a>"
"<a class='btn btn-danger' href='/motorStop'>STOP</a>"
"<a class='btn btn-primary' href='/motorRight'>RIGHT</a>"
"</div>"
"</div>"
"<div class='card'>"
"<div class='card-body'>"
"<h5 class='card-title'>Update firmware</h5>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update'>"
"<input class='btn btn-primary' type='submit' value='Update'>"
"</form>"
"<div class='progress' style='display: none;'>"
"<div class='progress-bar progress-bar-striped bg-success' role='progressbar' style='width: 0%' aria-valuenow='0' aria-valuemin='0' aria-valuemax='100'></div>"
"</div>"
"</div>"
"</div>"
"</div>"
"<div class='card'>"
"<div class='card-body'>"
"<div>"
"<div id='chart_container' style='height: 300px; width: 100%;'></div>"
"</div>"
"</div>"
"</div>"
"<script>"
"window.onload = function () {"
"  var dps = [];"
" var chart = new CanvasJS.Chart('chart_container', {"
"   title :{text: 'Power Data'},"
"   axisY: {includeZero: false, title: 'Voltage', suffix: 'V'},      "
"   axisX: {includeZero: false, title: 'Time', suffix: 's.'},      "
"   data: [{type: 'line', dataPoints: dps}]"
" });"
" var xVal = 0;"
" var yVal = 100; "
" var updateInterval = 1000;"
" var dataLength = 60;"
" var updatePowerChart = function () {"
"   $.get('/readSensors', function(data, status){"
"     yVal = parseFloat(data);"
"     dps.push({x: xVal,y: yVal});"
"     xVal++;"
"     if (dps.length == (dataLength - 1)) dps.shift();"
"     chart.render();"
"   });"
" };"
" setInterval(updatePowerChart, updateInterval);"
"};  "
"  $('#upload_form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$('.progress').show();"
" $.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('.progress-bar').attr('style', 'width: ' + Math.round(per*100) + '%;');"
"$('.progress-bar').attr('aria-valuenow', Math.round(per*100));"
"$('.progress-bar').html(Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!')"
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>";

void initPinsMode() {
  pinMode(led, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(adc_v_pin, INPUT);
  pinMode(adc_i_pin, INPUT);
}

void initLCD() {
  lcd.init();
  lcd.backlight();
}

void connectToWIFI() {
  lcd.setCursor(0, 0);
  lcd.print("Connecting to ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");
  int i = 1;

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    lcd.print(".");
    ++i;

    //try connect to second wifi
    if (i > 20) {
      i = 0;
      Serial.print("Connecting to ");
      Serial.println(ssid2);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connecting to ");
      lcd.setCursor(0, 1);
      lcd.print(ssid2);
      WiFi.begin(ssid2, password2);
    }
  }

  Serial.println("Connected to ");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected.");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
}

void initServerController() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });

  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/lcd", HTTP_GET, []() {
    lcd.clear();
    for (int i = 0; i < server.args(); i++) {
      lcd.setCursor(0, i);
      lcd.print(server.arg(i));
    }
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/motorLeft", HTTP_GET, []() {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/motorRight", HTTP_GET, []() {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/motorStop", HTTP_GET, []() {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, HIGH);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/readSensors", HTTP_GET, []() {
    float sum;
    for (int i = 0; i < numberOfTests; ++i) {
      sum += voltages[i];
      voltages[i] = 0;
    }
    float average = (sum / (numberOfTests * 1.000000)) / 22.3333333333;
    numberOfTests = 0;
    char buf[100];
    gcvt(average, 10, buf);
    server.send(200, "text/html", buf);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload & upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // flashing firmware to ESP
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
}

void doADCTest() {
  float adc_v = analogRead(adc_v_pin);
  voltages[numberOfTests] = adc_v;
  numberOfTests++;
}

void espLedHeartbeat() {
  //loop to blink without delay - Heartbeat of the ESP32
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    ledState = not(ledState);

    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

void setup(void) {
  //init serial print
  Serial.begin(115200);

  //set all pins mode
  initPinsMode();

  //init LCD
  initLCD();

  //connect to WIFI
  connectToWIFI();

  //set all server request handles 
  initServerController();

  //start web server
  server.begin();
}

void loop(void) {
  //handle all client requests 
  server.handleClient();

  //save results of ADC
  doADCTest();

  //just blink led like a heartbeat
  espLedHeartbeat();

  //delay
  delay(delayBetweenTests);
}
