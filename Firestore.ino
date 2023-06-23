// Include untuk menentukan jalur pin sensor PZEM ke ESP8266
#include <PZEM004Tv30.h>
PZEM004Tv30 pzem1(5, 4);
PZEM004Tv30 pzem2(14,12);
PZEM004Tv30 pzem3(15,13);

//Include untuk mengaktifkan aterau koneksi Modul ke jariangan WIFI
#include <ESP8266WiFi.h>
#include <Arduino.h>
#if defined(ESP32) || defined(PICO_RP2040)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

// Include untuk mengetahui waktu pengambilan data secara realtime
#include "time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

// Include untuk mengakitfkan dan koneksikan Firebase agar terhubung dengan alat
#include <Firebase_ESP_Client.h>
#define EN 12
// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* Deklasi WIFI SSID dan Password ke ESP8622 */
#define WIFI_SSID "SITIAISYAH"
#define WIFI_PASSWORD "bismillah99"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBB0APpvsyhUu2uBaJgmLEkaQfuh7EVPnE"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "johnlistrik-34d94"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ahmadmuzakir867@gmail.com"
#define USER_PASSWORD "Jajamuzakir123"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


unsigned long dataMillis = 0;
int count = 0;
int JumlahPerangkat;

 
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//ARDUINO_RASPBERRY_PI_PxICO_W
#if defined(IOT_LISTRIK_FIREBASE_ZAKIR)
WiFiMulti multi;
#endif

void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void setup()
{

    Serial.begin(9600);
    
// memulai koneksi WiFi pada perangkat
#if defined(IOT_LISTRIK_FIREBASE_ZAKIR)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
// memulai koneksi WiFi pada sebuah perangkat
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
// membaca jaringan internet
    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(IOT_LISTRIK_FIREBASE_ZAKIR)
        if (millis() - ms > 10000)
            break;
#endif
    }
    // Mendapatkan local IP dari SSID
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /*Tetapkan kunci api (wajib) */
    config.api_key = API_KEY;


    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // The WiFi credentials are required for LISTRIK_FIREBASE_ZAKIR
    // due to it does not have reconnect feature.
#if defined(IOT_LISTRIK_FIREBASE_ZAKIR)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif
    //get token
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    
#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);


    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(0);
}


//membuat dan mendapatkan name dengan char random
void getRandomStr(char* output, int len){
    char* eligible_chars = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890";
    for(int i = 0; i< len; i++){
        uint8_t random_index = random(0, strlen(eligible_chars));
        output[i] = eligible_chars[random_index];
    }
    Serial.print("output: "); Serial.println(output);
}

void getRandomStr1(char* output, int len){
    char* eligible_chars = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890";
    for(int i = 0; i< len; i++){
        uint8_t random_index = random(0, strlen(eligible_chars));
        output[i] = eligible_chars[random_index];
    }
    Serial.print("output: "); Serial.println(output);
}

void loop()
{
   
    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - dataMillis > 10000 || dataMillis == 0))
    {
        dataMillis = millis();

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;
        
        char iv_str[17] = {0}; //The last '\0' helps terminate the string
        getRandomStr(iv_str, 16);
        // We will create the nested document in the parent path "a0/b0/c0
        // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
        // and d? is the document id in the document collection id c0 which we will create.
        String documentPath = "DataBase3Jalur/d" + String (iv_str);
        // If the document path contains space e.g. "a b c/d e f"
        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"



        //deklasi power dari pzem
        
        float power1 = pzem1.power();
        float power2 = pzem2.power();
        float power3 = pzem3.power();
        float powerUtama = power1 + power2 + power3;
            if( !isnan(power1) ){
              //Mendapatkan data power     
                Serial.print("Power 1: "); Serial.print(power1); Serial.println("W");
              // Mengirimkan data power ke firebase             
                 content.set("fields/power1/doubleValue", power1);
            } else {
             //kondisi kita tidak mendapatkan power          
                Serial.println("Error reading power 1");
                content.set("fields/power1/doubleValue", power1 = 0);
          }

            if( !isnan(power2) ){
            //Mendapatkan data power   
                Serial.print("Power2: "); Serial.print(power2); Serial.println("W");
                // Mengirimkan data power ke firebase  
                content.set("fields/power2/doubleValue", power2);
            } else {
//                 kondisi kita tidak mendapatkan power 
                Serial.println("Error reading power 2");
                content.set("fields/power2/doubleValue", power2 = 0);
          }
           if( !isnan(power3) ){
            //Mendapatkan data power   
                Serial.print("Power3: "); Serial.print(power3); Serial.println("W");
                // Mengirimkan data power ke firebase  
                content.set("fields/power3/doubleValue", power3);
            } else {
                 //kondisi kita tidak mendapatkan power 
                Serial.println("Error reading power 3");
                content.set("fields/power3/doubleValue", power3 = 0);
          }
         
            if( !isnan(powerUtama) ){
                Serial.print("Power Utama: "); Serial.print(powerUtama); Serial.println("W");
                content.set("fields/powerUtama/doubleValue",powerUtama);
            } else {
                Serial.println("Error reading powe Utama");
          }


          // To Mengetahui jumlah perangkat yang tercolok     
          if(power1 == 0 && power2 == 0 && power3 == 0){
            Serial.println("Perangkat tidak terhubung");
            JumlahPerangkat = 0;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }
          else if(power1 >=1 && power2==0 && power3 == 0){
            Serial.println("Perangkat terhubung 1");
            JumlahPerangkat = 1;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }          
          else if (power1 ==0 && power2 >=1 && power3 == 0){
            Serial.println("Perangkat terhubung 1");
            JumlahPerangkat = 1;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }
          else if(power1 == 0 && power2 == 0 && power3 >=1){
            Serial.println("Perangkat terhubung 1");
            JumlahPerangkat = 1;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }
          else if(power1 >=1 & power2 >=1 & power3 == 0){
            Serial.println("Perangkat terhubung 2");
            JumlahPerangkat = 2;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            } 
          else if(power1 ==0 && power2 >=1 && power3 >=1){
            Serial.println("Perangkat terhubung 2");
            JumlahPerangkat = 2;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }
           else if(power1 >=1 & power2 ==0 & power3 >=1){
            Serial.println("Perangkat terhubung 2");
            JumlahPerangkat = 2;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            } 
          else if(power1 >=1 && power2 >=1 && power3 >=1){
            Serial.println("Perangkat terhubung 3");
            JumlahPerangkat = 3;
            content.set("fields/JumlahPerangkat/doubleValue",JumlahPerangkat);
            }
           else {
            Serial.println("Perangkat terhubung ERROR");
            }


     // deklarasi untuk mendapatkan volt dari pzem
     float Tegangan1 = pzem1.voltage();
     float Tegangan2 = pzem2.voltage();
     float Tegangan3 = pzem3.voltage();
     float TeganganTotal = Tegangan1 + Tegangan2 + Tegangan3 ;
            if( !isnan(Tegangan1) ){
              
            //Mendapatkan data voltage  dan menampilkan di serial monitor         
                Serial.print("Tegangan 1 : "); Serial.print(Tegangan1, 1); Serial.println("V");

            //mengirimkan data voltage ke firebase          
                content.set("fields/Tegangan1/doubleValue", Tegangan1);
            } else {
            //kondisi ketika tidak mendapatkan volt
                Serial.println("Error reading Voltage 1");
            }
           if( !isnan(Tegangan2) ){
            
            //Mendapatkan data voltage  dan menampilkan di serial monitor  
                Serial.print("Tegangan 2 : "); Serial.print(Tegangan2, 1); Serial.println("V");
                //mengirimkan data voltage ke firebase 
                content.set("fields/Tegangan2/doubleValue", Tegangan2);
            } else {

                Serial.println("Error reading Voltage 2");
            }
           if( !isnan(Tegangan3) ){
            
            //Mendapatkan data voltage  dan menampilkan di serial monitor  
                Serial.print("Tegangan 3 : "); Serial.print(Tegangan3, 1); Serial.println("V");

                //mengirimkan data voltage ke firebase 
                content.set("fields/Tegangan3/doubleValue", Tegangan3);
            } else {

              //kondisi ketika tidak mendapatkan volt
                Serial.println("Error reading Voltage 3");
            }

            
      // Deklarasi arus dari pzem
     float Arus1 = pzem1.current();
     float Arus2 = pzem2.current();
     float Arus3 = pzem3.current();
     float ArusTotal = Arus1 + Arus2 + Arus3;
            if( !isnan(Arus1) ){

              //mendapatkan data arus dan menampilkan data
                Serial.print("Arus 1: "); Serial.print(Arus1, 1); Serial.println("A");
                
              // Mengirimkan data arus ke firebase           
                content.set("fields/Arus1/doubleValue", Arus1);
            } else {
              
              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading Current 1");
                content.set("fields/Arus1/doubleValue", Arus1 = 0);
            }
            if( !isnan(Arus2) ){
              
              //mendapatkan data arus dan menampilkan data
                Serial.print("Arus 2: "); Serial.print(Arus2, 1); Serial.println("A");

                // Mengirimkan data arus ke firebase 
                content.set("fields/Arus2/doubleValue", Arus2);
            } else {

              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading Current 2");
                content.set("fields/Arus2/doubleValue", Arus2 = 0);
            }
            if( !isnan(Arus3) ){

              //mendapatkan data arus dan menampilkan data
                Serial.print("Arus 3: "); Serial.print(Arus3, 1); Serial.println("A");

                // Mengirimkan data arus ke firebase 
                content.set("fields/Arus3/doubleValue", Arus3);
            } else {

              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading Current 3");
                content.set("fields/Arus3/doubleValue", Arus3 = 0);
            }
            if( !isnan(ArusTotal) ){
                Serial.print("Arus Total: "); Serial.print(ArusTotal, 1); Serial.println("A");
                content.set("fields/Arustotal/doubleValue", ArusTotal);
            } else {
                Serial.println("Error reading Curremt Total");
            }
     
            //Deklarasi energi dari pzem
        float energy1 = pzem1.energy();
        float energy2 = pzem2.energy();
        float energy3 = pzem3.energy();
        //hitungan atau dekralasi energy total
        float energytotal = energy1 + energy2 +energy3;
        
            if( !isnan(energy1) ){
              
                //mendapatkan data energi dan menampilkan data
                Serial.print("Energy 1: "); Serial.print(energy1,3); Serial.println("kWh");
                
                // Mengirimkan data arus ke firebase 
                 content.set("fields/energy1/doubleValue", energy1);
            } else {
              
              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading energy 1");
            }
            if( !isnan(energy2) ){
                //mendapatkan data energi dan menampilkan data
                Serial.print("Energy 2: "); Serial.print(energy2,3); Serial.println("kWh");

                // Mengirimkan data arus ke firebase 
                 content.set("fields/energy2/doubleValue", energy2);
            } else {

              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading energy 3");
            }
            if( !isnan(energy3) ){
                //mendapatkan data energi dan menampilkan data
                Serial.print("Energy 3: "); Serial.print(energy3,3); Serial.println("kWh");

                // Mengirimkan data arus ke firebase 
                 content.set("fields/energy3/doubleValue", energy3);
            } else {

              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading energy 3");
            }
            if( !isnan(energytotal) ){
                //mendapatkan data energi dan menampilkan data
                Serial.print("Energy total: "); Serial.print(energytotal,3); Serial.println("kWh");

                // Mengirimkan data arus ke firebase 
                 content.set("fields/energytotal/doubleValue", energytotal);
            } else {

              // kondisi ketika data tidak didapatkan
                Serial.println("Error reading energy Total");
            }

       //  memanggil waktu secara update
      timeClient.update();

      // deklarasi untuk mengatur waktu
      time_t epochTime = timeClient.getEpochTime();

     // deklarasi untuk mendapatk waktu 
      String formattedTime = timeClient.getFormattedTime();

      // Proses mereset energy sesuai dengan waktu ditentukan
        if( formattedTime >="17:00:00"&&formattedTime <="17:01:00"){
        pzem1.resetEnergy();
        pzem2.resetEnergy();
        pzem3.resetEnergy();    
        }
        
        //mendapatkan dan menampilkan data sesuai req waktu
       if(formattedTime >="16:59:00"&&formattedTime <"17:00 :00"){
       Serial.print("Total Energy Day: "); Serial.print(energytotal,3); Serial.println("kWh");
       //mengirimakn data total ke firebase
       content.set("fields/TotalEnergyDay/doubleValue", energytotal);
       
       }  
       //Mendapatkan waktu dan mendeklarasikan
      Serial.print("Formatted Time: ");
      Serial.println(formattedTime);  
    
    
      String weekDay = weekDays[timeClient.getDay()];
       
      int Harga_listrik = energytotal*1352;
      Serial.print("Harga listrik: RP. ");
      Serial.println(Harga_listrik);
      content.set("fields/HargaListrik/doubleValue",+Harga_listrik);
    
      //Get a time structure
      struct tm *ptm = gmtime ((time_t *)&epochTime); 
    
      int monthDay = ptm->tm_mday;
      int currentMonth = ptm->tm_mon+1;
      String currentMonthName = months[currentMonth-1];
      int currentYear = ptm->tm_year+1900;
      String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
     Serial.println("");
     //mengirimkan data waktu dan tanggal ke firebase
      content.set("fields/TimeStamp/timestampValue", currentDate+"T"+formattedTime+"Z");
            count++;
           //Menampilkan loadinng create document
            Serial.print("Create a document... ");
            //Untuk mengetahui ketika sudah terhubung dengan firebase 
            //membuat dokumen di firebase dengan data yang sudah didapatkan
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.println("Terhubung dengan Firebase");
            else
                Serial.println(fbdo.errorReason());
    }
  delay(9000);
}
