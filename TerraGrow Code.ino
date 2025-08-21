// Your Blynk Auth Token
#define BLYNK_AUTH_TOKEN "MXsjVYN8Z-fW5Yv_gRJhlLyULlyDO_OR"
#define BLYNK_TEMPLATE_ID "TMPL6NRng2yzI"
#define BLYNK_TEMPLATE_NAME "ph dan kelembaban"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>  // Menambahkan pustaka DHT

// Akses koneksi Blynk
char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid = "C12CA1";
const char* password = "primawijaya";

// Pin definitions
#define DMSpin  13        // Pin output untuk DMS ph
#define indikator  2      // Pin output untuk indikator pembacaan sensor
#define adcPin 34         // Pin input sensor pH tanah
#define DMSpin_soil 12    // Pin output untuk DMS kelembaban
#define InputAnalog 35    // Pin input untuk sensor kelembaban tanah
#define RELAY_PIN 5       // Pin untuk relay
#define MID_DURATION 5000  // 0.5 detik (500 milidetik)

// DHT sensor
#define DHT_PIN 14        // Pin untuk DHT11
#define DHT_TYPE DHT11    // Jenis sensor DHT (DHT11)

DHT dht(DHT_PIN, DHT_TYPE);  // Inisialisasi sensor DHT

// Variables
int ADC_pH;
int ADC_Soil;
float pH = 0;
float moisture = 0;
int adcValue = 0;
float temperature = 0;
float humidity = 0;

// Tabel Persentase (%) dan Nilai ADC Sensor
const int adcSensor[] = {0, 102, 204, 306, 409, 511, 613, 716, 818, 920, 1023}; // Nilai ADC
const float moistureMeter[] = {100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0}; // Persentase kelembapan

// Fungsi Interpolasi Linear untuk menghitung kelembapan berdasarkan ADC
int interpolate(int adcValue) {
    int lowerIdx = 0;
    int upperIdx = 0;

    for (int i = 0; i < sizeof(adcSensor) / sizeof(adcSensor[0]); i++) {
        if (adcSensor[i] <= adcValue) {
            lowerIdx = i;
        }
        if (adcSensor[i] > adcValue) {
            upperIdx = i;
            break;
        }
    }

    if (adcValue < adcSensor[0]) {
        return (int)moistureMeter[0];  
    } else if (adcValue > adcSensor[sizeof(adcSensor) / sizeof(adcSensor[0]) - 1]) {
        return (int)moistureMeter[sizeof(moistureMeter) / sizeof(moistureMeter[0]) - 1];
    }

    float moisture = moistureMeter[lowerIdx] + (adcValue - adcSensor[lowerIdx]) * 
                     (moistureMeter[upperIdx] - moistureMeter[lowerIdx]) / 
                     (adcSensor[upperIdx] - adcSensor[lowerIdx]);
    
    return (int)moisture;  
}

// Fuzzy membership function for temperature
String fuzzyTemperature(float temperature) {
    if (temperature <= 18) {
        return "Cold"; // Cold
    } else if (temperature >= 9 && temperature <= 36) {
        return "Good"; // Good
    } else if (temperature >= 27) {
        return "Hot"; // Hot
    }
    return "Unknown";  // Return "Unknown" if no condition is met
}

// Fuzzy membership function for humidity
String fuzzyHumidity(float humidity) {
    if (humidity <= 50) {
        return "Dry"; // Dry
    } else if (humidity >= 40 && humidity <= 80) {
        return "Normal"; // Normal
    } else if (humidity >= 60) {
        return "Moist"; // Moist
    }
    return "Unknown";  // Return "Unknown" if no condition is met
}

// Fuzzy membership function for soil moisture
String fuzzySoilMoisture(float moisture) {
    if (moisture <= 60) {
        return "Low"; // Low
    } else if (moisture >= 50 && moisture <= 90) {
        return "Medium"; // Medium
    } else if (moisture >= 80) {
        return "High"; // High
    }
    return "Unknown";  // Return "Unknown" if no condition is met
}

// Defuzzification using Sugeno method (average)
String defuzzify(float temperature, float humidity, float moisture) {
    String temperatureLinguistic = fuzzyTemperature(temperature);
    String humidityLinguistic = fuzzyHumidity(humidity);
    String moistureLinguistic = fuzzySoilMoisture(moisture);

    // Display linguistic values


    // Apply fuzzy rules based on combinations of the conditions
    if (temperatureLinguistic == "Cold" && humidityLinguistic == "Dry" && moistureLinguistic == "Low") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Dry" && moistureLinguistic == "Medium") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Dry" && moistureLinguistic == "High") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Normal" && moistureLinguistic == "Low") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Normal" && moistureLinguistic == "Medium") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Normal" && moistureLinguistic == "High") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Moist" && moistureLinguistic == "Low") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Moist" && moistureLinguistic == "Medium") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Cold" && humidityLinguistic == "Moist" && moistureLinguistic == "High") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Dry" && moistureLinguistic == "Low") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Dry" && moistureLinguistic == "Medium") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Dry" && moistureLinguistic == "High") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Normal" && moistureLinguistic == "Low") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Normal" && moistureLinguistic == "Medium") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Normal" && moistureLinguistic == "High") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Moist" && moistureLinguistic == "Low") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Moist" && moistureLinguistic == "Medium") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Good" && humidityLinguistic == "Moist" && moistureLinguistic == "High") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Dry" && moistureLinguistic == "Low") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Dry" && moistureLinguistic == "Medium") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Dry" && moistureLinguistic == "High") {
        return "OFF";  // Relay OFF
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Normal" && moistureLinguistic == "Low") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Normal" && moistureLinguistic == "Medium") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Normal" && moistureLinguistic == "High") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Moist" && moistureLinguistic == "Low") {
        return "MID";  // Relay MID
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Moist" && moistureLinguistic == "Medium") {
        return "ON";  // Relay ON
    } else if (temperatureLinguistic == "Hot" && humidityLinguistic == "Moist" && moistureLinguistic == "High") {
        return "ON";  // Relay ON
    }
    
    return "OFF";  // Default Relay OFF
}

void setup() {
    Serial.begin(115200);           
    analogReadResolution(10);       
    pinMode(DMSpin, OUTPUT);
    pinMode(DMSpin_soil, OUTPUT);
    pinMode(indikator, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);     
    digitalWrite(DMSpin, HIGH);     
    digitalWrite(DMSpin_soil, HIGH);

    // Koneksi WiFi dan Blynk
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nBerhasil Koneksi ke WiFi");

    Blynk.begin(auth, ssid, password, "iot.serangkota.go.id", 8080);

    dht.begin();
}

void readMoistureSensor() {
    digitalWrite(DMSpin_soil, LOW);
    digitalWrite(indikator, HIGH);
    delay(2000);

    ADC_Soil = analogRead(InputAnalog);
    moisture = interpolate(ADC_Soil);

    // Menampilkan data kelembaban tanah di Serial Monitor
    Serial.println("=== Pembacaan Sensor Kelembaban Tanah ===");
    Serial.print("ADC Soil Value: ");
    Serial.println(ADC_Soil);
    Serial.print("Moisture Value: ");
    Serial.println(moisture);
    String moistureLinguistic = fuzzySoilMoisture(moisture);
    Serial.print("Soil Moisture Status: ");
    Serial.println(moistureLinguistic);

    Blynk.virtualWrite(V2, ADC_Soil);
    Blynk.virtualWrite(V3, moisture);      
    Blynk.virtualWrite(V8, "Soil Moisture: " + moistureLinguistic);

    digitalWrite(DMSpin_soil, HIGH);
    digitalWrite(indikator, LOW);
}

void readPhSensor() {
    digitalWrite(DMSpin, LOW);
    digitalWrite(indikator, HIGH);
    delay(5000);

    ADC_pH = analogRead(adcPin);
    pH = (-0.0233 * ADC_pH) + 12.698;

    // Menampilkan data pH tanah di Serial Monitor
    Serial.println("=== Pembacaan Sensor pH Tanah ===");
    Serial.print("ADC pH Value: ");
    Serial.println(ADC_pH);
    Serial.print("pH Value: ");
    Serial.println(pH, 1);

    Blynk.virtualWrite(V0, ADC_pH);
    Blynk.virtualWrite(V1, pH);

    digitalWrite(DMSpin, HIGH);
    digitalWrite(indikator, LOW);
}

void readDHT11() {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // Menampilkan hasil pembacaan pada Serial Monitor
    Serial.println("=== Pembacaan Sensor DHT11 ===");

    Serial.print("Humidity: ");
    Serial.print(humidity, 2);  // Menampilkan kelembaban dengan 2 digit desimal
    Serial.println("%");

    Serial.print("Temperature: ");
    Serial.print(temperature, 2);  // Menampilkan suhu dengan 2 digit desimal
    Serial.println("°C");

    String temperatureLinguistic = fuzzyTemperature(temperature);
    String humidityLinguistic = fuzzyHumidity(humidity);

    Serial.print("Temperature Status: ");
    Serial.println(temperatureLinguistic);
    Serial.print("Humidity Status: ");
    Serial.println(humidityLinguistic);
 
    
    Blynk.virtualWrite(V4, humidity);
    Blynk.virtualWrite(V5, temperature);
}


BLYNK_WRITE(V6) {
    int pinValue = param.asInt();  
    if (pinValue == 1) {
        digitalWrite(RELAY_PIN, HIGH);  
        Serial.println("Relay is ON");
        Blynk.virtualWrite(V6, HIGH);   
    } else {
        digitalWrite(RELAY_PIN, LOW);   
        Serial.println("Relay is OFF");
        Blynk.virtualWrite(V6, LOW);    
    }
}

void loop() {
    Blynk.run();
    // Kirim status relayOutput ke LCD Virtual Pin V7


    // Kirim status variabel linguistik ke LCD Virtual Pin V8

    // Pembacaan sensor kelembaban tanah
    readMoistureSensor();
    delay(2000);  

    // Pembacaan sensor pH tanah
    readPhSensor();
    delay(2000);  

    // Pembacaan sensor suhu dan kelembaban DHT11
    readDHT11();
    delay(2000);  

    
    String relayState = defuzzify(temperature, humidity, moisture);

    // Print the defuzzification result to the Serial Monitor
    Serial.print("Hasil Keputusan Fuzzy Untuk Relay: ");
    Serial.println(relayState);
    Blynk.virtualWrite(V7, relayState);

    // Control relay based on fuzzy logic output
    if (relayState == "ON") {
        digitalWrite(RELAY_PIN, HIGH);  // Relay ON
    } else if (relayState == "MID") {
        digitalWrite(RELAY_PIN, HIGH);  // Relay MID
    } else {
        digitalWrite(RELAY_PIN, LOW);   // Relay OFF
    }

    delay(MID_DURATION);  // Delay before next loop iteration 
    }
 
