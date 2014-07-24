#include "led.h"

LedStrip::LedStrip(uint8_t _pin): pin(_pin), adaStrip(30, pin, NEO_GRB + NEO_KHZ800){}

void LedStrip::Initialize(){
  adaStrip.begin();
  // Initialize all pixels to 'off'
  adaStrip.show(); 
}

void LedStrip::SetBottle(uint8_t bottleNum, uint8_t displayMode, uint8_t color){
  uint32_t ledColor[5];
  uint32_t wait;
  uint8_t idx;

  // first select the desired color 
  switch (color){ 
    case '0': // rainbow
	
       ledColor[0] = adaStrip.Color(255, 255, 0); // yellow
       ledColor[1] = adaStrip.Color(100, 255, 0); // yellow-green
       ledColor[2] = adaStrip.Color(255, 100, 0); // red-yellow
       ledColor[3] = adaStrip.Color(255, 0, 100); // red-purple
       ledColor[4] = adaStrip.Color(255, 0, 255); // magenta
    break;
    case '1': // white
      for( int i = 0; i < 5; i++){ 
            ledColor[i] = adaStrip.Color(255, 255, 255); 
      }
    break;
    case '2': // bottle specific
      for( int i = 0; i < 6; i++){ 
            ledColor[i] = BottleColors[bottleNum]; 
      }
    break;
    case '3': // off default
    default: uint32_t ledColor = adaStrip.Color(0, 0, 0); 
  }

  //display mode
  switch(displayMode){ 
    case Ramp: 
	  if((bottleNum %  2) == 0){
		wait = 300;
		for(int i = 0; i < 5; i++){ 
			idx = bottleNum * 5 + i;
			adaStrip.setPixelColor(idx, ledColor[i]); 
			adaStrip.show(); 
			delay(wait - (i*50));
			adaStrip.setPixelColor(idx, 0, 0, 0); 
		}  
	  }
	  else{
	    wait = 100;
		for(int i = 4; i >= 0; i--){ 
			idx = bottleNum * 5 + 5 - i;
			adaStrip.setPixelColor(idx, ledColor[i]); 
			adaStrip.show(); 
			delay(wait + i * 50);
			adaStrip.setPixelColor(idx, 0, 0, 0); 
		}  
	  }
     break;
     case Solid: 
	   if((bottleNum %  2) == 0){
		   for(int i = 0; i < 5; i++){ 
			  idx = bottleNum * 5 + i;
			  adaStrip.setPixelColor((bottleNum + i), ledColor[i]);
			}
	   }
	   else{
		   for(int i = 4; i >= 0; i--){ 
			idx = bottleNum * 9 - i;
			adaStrip.setPixelColor((bottleNum + i), ledColor[i]);
		   }
	   }
     break;

     case Blink: 
	   if((bottleNum %  2) == 0){
		   for(int i = 0; i < 5; i++){ 
			  idx = bottleNum * 5 + i;
			  adaStrip.setPixelColor((bottleNum + i), ledColor[i]);
			}
	   }
	   else{
		   for(int i = 4; i >= 0; i--){ 
			idx = bottleNum * 9 - i;
			adaStrip.setPixelColor((bottleNum + i), ledColor[i]);
		   }
	   }
  }//end switch

  adaStrip.show();
}


