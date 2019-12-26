#include "MH_Z14.hpp"

MH_Z14 mh_z14;

void setup(){
    Serial.begin(9600);

    mh_z14.initialize();
    delay(1000);
}

void loop(){
  uint16_t _co2ppm = 0;

  if(mh_z14.get_CO2_ppm(_co2ppm)){
    String _co2_str = "CO2ppm = ";
    _co2_str += String(_co2ppm);
    Serial.println(_co2_str);
  }else{
    Serial.println("CO2 read failure...");
  }
  delay(1000); 
}