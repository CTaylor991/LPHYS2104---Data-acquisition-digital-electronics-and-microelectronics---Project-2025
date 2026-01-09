// LPHYS2104 - Data acquisition, digital electronics and microelectronics - Charlie Taylor - Project 2025 

//-------------- Libraries --------------------------------

// I2C communication library (used by the SCD30)
#include <Wire.h>

// Adafruit SCD30 CO2 sensor library
#include "Adafruit_SCD30.h"

// ESP32 WiFi and WebServer libraries
#include <WiFi.h>
#include <WebServer.h>

//-------------- CO2, Temp and Humidity sensor --------------------------------

// Create the SCD30 sensor object
Adafruit_SCD30 scd30;

//-------------- LED Pins --------------------------------
const int LED_GREEN = 4;
const int LED_YELLOW = 17;
const int LED_RED = 18;

//---------------Potentiometer-----------------------------
const int pot = 32;

// ESP32 wifi and webserver setup
const char* ssid     = "ESP32_ACCESS_POINT_CHARLIE";
const char* password = "password";

//Create a web server listening on port 80 (standard HTTP)
WebServer server(80);

// Setup variables that hold the readings and what the webpage will read/display
float currentCO2 = 0.0;
float currentTemp = 0.0;
float currentHumidity = 0.0;
float potValue= 0.0;
float potVoltage = 0.0;
float potOut = 0.0;
float LEDBrightness = 0.0;
String co2status = ""; 


// Establish function (void) for the sensor readings, LED output and printed co2 status
// analogWrite is used rather than digitalWrite to enable the potentiometer to vary the brightness
void checkAirQuality(float co2_reading) {
  Serial.print("Status: ");

  if (co2_reading <= 450) {
    Serial.println("Healthy outside air level (Excellent)");
    co2status = "Excellent";
    analogWrite(LED_GREEN, potOut);
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_RED, 0);
  }
  else if (co2_reading <= 700) {
    Serial.println("Healthy indoor climate (Good)");
    co2status = "Good";
    analogWrite(LED_GREEN, potOut);
    analogWrite(LED_YELLOW, potOut);
    analogWrite(LED_RED, 0);
  }
  else if (co2_reading <= 900) {
    Serial.println("Acceptable level (Fair)");
    co2status = "Fair";
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_YELLOW, potOut);
    analogWrite(LED_RED, 0);
  }
  else if (co2_reading <= 1100) {
    Serial.println("Ventilation required (Poor)");
    co2status = "Poor";
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_YELLOW, potOut);
    analogWrite(LED_RED, potOut);
  }
  else if (co2_reading <= 1300) {
    Serial.println("Ventilation necessary (Bad)");
    co2status = "Bad";
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_RED, potOut);
  }
  else if (co2_reading <= 2000) {
    Serial.println("Negative health effects (Very Bad)");
    co2status = "Very Bad";
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_RED, potOut);
    delay(100);
    analogWrite(LED_RED, 0);
    delay(100);
    analogWrite(LED_RED, potOut);
    delay(100);
    analogWrite(LED_RED, 0);
  }
  else {
    Serial.println("HAZARDOUS PROLONGED EXPOSURE (DANGER)");
    co2status = "DANGER";
    //analogWrite(LED_GREEN, 0);
    //analogWrite(LED_YELLOW, 0);
    //analogWrite(LED_RED, 0);
    //delay(100);
    analogWrite(LED_GREEN, 255);
    delay(100);
    analogWrite(LED_YELLOW, 255);
    delay(100);
    analogWrite(LED_RED, 255);
    delay(100);
    analogWrite(LED_GREEN, 0);
    delay(100);
    analogWrite(LED_YELLOW, 0);
    delay(100);
    analogWrite(LED_RED, 0);
  }
}

// Webpage handler from tutorial with edits to display what we want
//Does display of the sensor numbers and the figures as well as pulling the data effectively runnning its own script
void handleRoot() {
  //Stores a large HTML page in flash memory (PROGMEM)
  const char html[] PROGMEM = R"rawliteral( 

<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <style>
    body { font-family: Helvetica; text-align: center; }
    h1 { color: #333; }
    .value { font-size: 2em; }
    canvas { max-width: 100%; }
  </style>
</head>
<body>
<h1>ESP32 Air Quality Monitor</h1>
<p>CO2 (ppm): <span id='co2' class='value'>---</span></p>
<p>CO2 Status: <span id='co2status' class='value'>---</span></p>
<p>Temperature (Celsius): <span id='temp' class='value'>---</span></p>
<p>Humidity (%): <span id='hum' class='value'>---</span></p>
<p>LED Brightness (%): <span id='ledBright' class='value'>---</span></p>

<canvas id='co2Chart'></canvas>
<canvas id='tempChart'></canvas>
<canvas id='humChart'></canvas>
<p>Page refreshes automatically.</p>
<p>Built by Charlie Taylor for LPHYS2104 - Enjoy.</p>

<script>
let seconds = 0;
let labels = [];
let co2Data = [];
const ctx = document.getElementById('co2Chart').getContext('2d');
const chart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: labels,
    datasets: [{
      label: 'CO2 (ppm)',
      data: co2Data,
      borderColor: 'red',
      fill: false
    }]
  },
  options: {
    animation: false,
    scales: {
      x: { title: { display: true, text: 'Time since start' } },
      y: { title: { display: true, text: 'CO2 (ppm)' } }
    }
  }
});
let tempLabels = [];
let tempData = [];
const tempCtx = document.getElementById('tempChart').getContext('2d');
const tempChart = new Chart(tempCtx, {
  type: 'line',
  data: {
    labels: tempLabels,
    datasets: [{
      label: 'Temperature (Celsius)',
      data: tempData,
      borderColor: 'green',
      fill: false
    }]
  },
  options: {
    animation: false,
    scales: {
      x: { title: { display: true, text: 'Time since start (s)' } },
      y: { title: { display: true, text: 'Temperature (°C)' } }
    }
  }
});
let humLabels = [];
let humData = [];
const humCtx = document.getElementById('humChart').getContext('2d');
const humChart = new Chart(humCtx, {
  type: 'line',
  data: {
    labels: humLabels,
    datasets: [{
      label: 'Humidity (%)',
      data: humData,
      borderColor: 'blue',
      fill: false
    }]
  },
  options: {
    animation: false,
    scales: {
      x: { title: { display: true, text: 'Time since start (s)' } },
      y: { title: { display: true, text: 'Humidity (%)' } }
    }
  }
});


function updateData() {
  fetch('/data').then(r => r.text()).then(d => {
    let v = d.split(',');
    document.getElementById('co2').innerHTML = v[0];
    document.getElementById('temp').innerHTML = v[1];
    document.getElementById('hum').innerHTML = v[2];
    document.getElementById('ledBright').innerHTML = v[3];
    document.getElementById('co2status').innerHTML = v[4]; 
    // The above fetches all the different required sensor inputs (and the CO2 status) that will be displayed.
 


    seconds += 2;       // increment by X seconds each update
    labels.push(seconds); // use seconds as the label
    co2Data.push(parseFloat(v[0]));
    if (labels.length > 600) { labels.shift(); co2Data.shift(); }
    chart.update();
    tempLabels.push(seconds);
    tempData.push(parseFloat(v[1])); // v[1] is temperature
    if (tempLabels.length > 600) { tempLabels.shift(); tempData.shift(); }
    tempChart.update();
    humLabels.push(seconds);
    humData.push(parseFloat(v[2])); // v[2] is humidity
    if (humLabels.length > 600) { humLabels.shift(); humData.shift(); }
    humChart.update();

  });
}

setInterval(updateData, 2000);
updateData();
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

//called when the browser requests data
void handleData() {
  //combine the data into a comma seperated string
  String data = String(currentCO2, 1) + "," +
                String(currentTemp, 1) + "," +
                String(currentHumidity, 1) + "," +
                String(LEDBrightness, 1) + "," +
                co2status;       

  //send the data to the server
  server.send(200, "text/plain", data);
}

//-----SETUP (only runs once)-----------------------------

void setup() {
  //Establish Serial monitor and its baud rate
  Serial.begin(9600);
  //Delay to enable the serial to establish
  delay(1000);

  //---------LEDs------------------
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  //---------Sensor----------------
  //Establish I2C communication
  Wire.begin();
  //Communicate with sensor
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 sensor!");
    while (1) delay(10);
  }
  Serial.println("SCD30 sensor found!");

  //---------WiFi----------------
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("ESP32 Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  //---------Webserver----------------
  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("Web server started");
}

//-----------LOOP (runs continuously)-------------------

void loop() {

  // Handle incoming web requests
  server.handleClient();

  // Check if new sensor data is available
  if (scd30.dataReady()) {
    //Read the sensor data into the objects
    if (scd30.read()) {

      // Update sensor values and print them
      currentCO2 = scd30.CO2;
      currentTemp = scd30.temperature;
      currentHumidity = scd30.relative_humidity; 
      //Serial.println("--- New Sensor Data ---");
      Serial.print("IP address: ");
      Serial.println(WiFi.softAPIP());
      Serial.print("CO2: "); Serial.println(currentCO2);
      Serial.print("Temp: "); Serial.println(currentTemp);
      Serial.print("Humidity: "); Serial.println(currentHumidity);
      Serial.println();

      // Update LEDs
      checkAirQuality(currentCO2);
    }
  }
  // Read the analog value (0-4095) from the pin
  potValue = analogRead(pot);
  //convert 12 bit input to Voltage value
  potVoltage = potValue * (3.3 / 4095.0);
  //convert to 8 bit PWM output
  potOut = potValue * (255/4095.0);
  //Conver PWM to percentage
  LEDBrightness = (potOut / 255) * 100;
 

  Serial.print("Analog Value: ");
  Serial.println(potValue); // Print the raw value (0-4095)

  Serial.print("Voltage: ");
  Serial.print(potVoltage);
  Serial.println(" V");
  Serial.print("PotOut: ");
  Serial.println(potOut);
  Serial.print("Brightness: ");
  Serial.print(LEDBrightness);
  Serial.println("%");



  // Short delay to avoid over printing (designed to match the 2 second cycle of the sensors)
  delay(2000);
}
