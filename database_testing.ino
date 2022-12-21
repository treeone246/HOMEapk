#include <PZEM004Tv30.h>


#include<WiFi.h> //koneksi ke esp32
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include <DHT.h>//sensor suhu/humidity

#define WIFI_SSID "gabrielws" //nama wifi
#define WIFI_PASSWORD "sepeed246" //sandi wifi
#define API_KEY "AIzaSyAZcPjEzGuV5l9JbntBqr9aVieUlZ9g1J0" //sandi api firebase
#define DATABASE_URL "https://home-psi-project-default-rtdb.asia-southeast1.firebasedatabase.app/" //link url database


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int dataSuhu = 0;
float voltage = 0.0;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  

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

  
}

void loop() {
  // put your main code here, to run repeatedly:
float humi = 20;
 
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //------ngestore data ke RTDB
    
    if(Firebase.RTDB.setInt(&fbdo, "Sensor/data_suhu_humi", humi)){
      Serial.println(); Serial.print(humi);
      Serial.print(" - sukses masuk ke dalam: " + fbdo.dataPath());  
      }else{
         
      Serial.println("GAGAL: " + fbdo.errorReason());
     }
  }
 }
