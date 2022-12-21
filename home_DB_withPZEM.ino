#include <PZEM004Tv30.h>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif

#define NUM_PZEMS 2

PZEM004Tv30 pzems[NUM_PZEMS];


#include<WiFi.h> //koneksi ke esp32
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include <DHT.h>//sensor suhu/humidity

#define WIFI_SSID "gabrielws" //nama wifi
#define WIFI_PASSWORD "sepeed246" //sandi wifi
#define API_KEY "AIzaSyAZcPjEzGuV5l9JbntBqr9aVieUlZ9g1J0" //sandi api firebase
#define DATABASE_URL "https://home-psi-project-default-rtdb.asia-southeast1.firebasedatabase.app/" //link url database

#define dht_pin 21 //sambung dht sensor ke pin 21 esp32
#define dht_type DHT11 //tipe dht yang digunakan
#define relay_pin 16 //sambung relay module ke pin 16 esp32

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int dataSuhu = 0;
float voltage = 0.0;

DHT dht_sensor(dht_pin, dht_type);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

 for(int i = 0; i < NUM_PZEMS; i++)
    {

#if defined(USE_SOFTWARE_SERIAL)
        // Initialize the PZEMs with Software Serial
        pzems[i] = PZEM004Tv30(pzemSWSerial, 0x10 + i);
#elif defined(ESP32)
        // Initialize the PZEMs with Hardware Serial2 on RX/TX pins 16 and 17
        pzems[i] = PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x10 + i);
#else
        // Initialize the PZEMs with Hardware Serial2 on the default pins

        /* Hardware Serial2 is only available on certain boards.
        *  For example the Arduino MEGA 2560
        */
        pzems[i] = PZEM004Tv30(PZEM_SERIAL, 0x10 + i);
#endif
    }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(300); 
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("SignUp OK");
    signupOK = true;    
  }else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht_sensor.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:

for(int i = 0; i < NUM_PZEMS; i++){
        // Print the Address of the PZEM
        Serial.print("PZEM ");
        Serial.print(i);
        Serial.print(" - Address:");
        Serial.println(pzems[i].getAddress(), HEX);
        Serial.println("===================");

        
  float humi  = dht_sensor.readHumidity();
  float tempC = dht_sensor.readTemperature(); //suhu celcius
  /////////////
   float voltage = pzems[i].voltage();
   float current = pzems[i].current();
   float power = pzems[i].power();
   float energy = pzems[i].energy();
   float frequency = pzems[i].frequency();
   float pf = pzems[i].pf();

 
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //------ngestore data ke RTDB
    
    if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_suhu_humi", humi)){
      Serial.println(); Serial.print(humi);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
    
      if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_suhu_tempC", tempC)){
      Serial.println(); Serial.print(tempC);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
      }
      if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_voltage", voltage)){
      Serial.println(); Serial.print(voltage);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
      }
      if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_Current", current)){
      Serial.println(); Serial.print(current);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
      }
      if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_Power", power)){
      Serial.println(); Serial.print(power);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
      }
      if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_Energy", energy)){
      Serial.println(); Serial.print(energy,3);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());
      }else{
         
      Serial.println("GAGAL: " + fbdo.errorReason());
     }
    }  
  }
 }
}
