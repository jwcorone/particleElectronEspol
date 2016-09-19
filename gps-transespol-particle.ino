// This #include statement was automatically added by the Particle IDE.
#include "AssetTracker/AssetTracker.h"

unsigned long myChannelNumber = 147034;
const char * myWriteAPIKey = "YXWEEEBCJM9AZUKB";
char data[320];

#define publish_delay 16000
unsigned int lastPublish = 0;
//esta variable es para que si la bateria este baja envie al doble de tiempo 
//y se ahorre un poco mas
unsigned int downBattery = 1;

// Threshold to trigger a publish
// 9000 is VERY sensitive, 12000 will still detect small bumps
int accelThreshold = 9500;

// Creating an AssetTracker named 't' for us to reference
AssetTracker t = AssetTracker();
int x;
int y;
int z;
const int field3=1;
float la, la2;
float lo;
// Una variable FuelGauge para escanear el status de la bateria
FuelGauge fuel;

void setup() {
    
    t.begin();
    t.gpsOn();
    x=0;
    // Opens up a Serial port so you can listen over USB
    Serial.begin(9600);
    
    // These three functions are useful for remote diagnostics. Read more below.
    /*Particle.function("tmode", transmitMode);*/
    Particle.function("batt", batteryStatus);
    Particle.function("aThresh",accelThresholder);
    Particle.function("gps", gpsPublish);
    /*ThingSpeak.begin(client);
    Particle.subscribe("hook-response/get_thingSpeakWrite_", myHandler, MY_DEVICES);*/
}

void loop() {
    t.updateGPS();
    float longi;
    float lati;
    unsigned long now = millis();
    if ((now - lastPublish) < (downBattery * publish_delay)) {
        return;
    }
    //analisis de porcentaje de bateria para 
    //enviar gps y pasar de 16 a 32 segundos el envio y ahorrar bateria
    if(fuel.getSoC()<=10) {  
        downBattery = 2;
    } else {
        downBattery = 1;
    }
    //captura del GPS para enviar a publicar a webhook
    if(t.gpsFix()) {
        //t.gpsFix();
        longi = t.readLon();
        lati = t.readLat();
        //-2.146276, -79.964139dtostrf(floatVal, 4, 4, charVal); 
        ++x;
        Particle.publish("thingSpeakWrite_A0", "{\"k\":\"YXWEEEBCJM9AZUKB\", \"1\":\""+String(lati)+"\", \"2\":\""+String(longi)+"\", \"3\":\"1\", \"4\":\""+String(t.readX())+"\", \"a\":\""+String(lati)+"\", \"o\":\""+String(longi)+"\"}", 60, PRIVATE);
    }
    lastPublish = now;
}
// Remotely change the trigger threshold!
int accelThresholder(String command){
    accelThreshold = atoi(command);
    return 1;
}

// Actively ask for a GPS reading if you're impatient. Only publishes if there's
// a GPS fix, otherwise returns '0'
int gpsPublish(String command){
    if(t.gpsFix()){ 
        Particle.publish("G", t.readLatLon(), 60, PRIVATE);
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    }
    else { return 0; }
}
// Lets you remotely check the battery status by calling the function "batt"
// Triggers a publish with the info (so subscribe or watch the dashboard)
// and also returns a '1' if there's >10% battery left and a '0' if below
int batteryStatus(String command){
    // Publish the battery voltage and percentage of battery remaining
    // if you want to be really efficient, just report one of these
    // the String::format("%f.2") part gives us a string to publish,
    // but with only 2 decimal points to save space
    Particle.publish("B", 
          "v:" + String::format("%.2f",fuel.getVCell()) + 
          ",c:" + String::format("%.2f",fuel.getSoC()),
          60, PRIVATE
    );
    // if there's more than 10% of the battery left, then return 1
    if(fuel.getSoC()>10){ return 1;} 
    // if you're running out of battery, return 0
    else { return 0;}
}

