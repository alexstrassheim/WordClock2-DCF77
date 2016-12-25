#include <Arduino.h>
#include <WClock.h>


void WClock::begin(void) {
    DEBUG_PRINTLN("Begin WordClock Matrix");

    // FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    // FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );
}


void WClock::resetAndBlack() {
    for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}


void WClock::drawHeart() {
    DEBUG_PRINTLN("HEART");
}

void WClock::drawMatrix() {
    DEBUG_PRINTLN("MATRIX");
}

uint16_t WClock::XY( uint8_t y, uint8_t x) {
  uint16_t i;

  if( y & 0x01) {
      // Odd rows run forwards
      i = (((kMatrixWidth * 10)+3) - (y * kMatrixWidth)) - kMatrixWidth + (1 + x);
  } else {
      // Even rows run backwards
      i = (((kMatrixWidth * 10)+3) - (y * kMatrixWidth)) - x;
  }
  return i;
}


void WClock::setLed(int row, int col, boolean on) {
    if(on)
        leds[XY(row,col)] = CRGB::Blue;
    else
        leds[XY(row,col)] = CRGB::Black;
    FastLED.show();
}

void WClock::updateDisplay(boolean previousframe[11][11], boolean frame[11][11]) {
    for (int r = 0; r < 11; ++r) {
        for (int c = 0; c < 11; ++c) {
            if (prevframe[r][c] && !frame[r][c]) {
                setLed(r, c, false);
            } else if (!prevframe[r][c] && frame[r][c]) {
                setLed(r, c, true);
            }
        }
    }
}

void WClock::addWordToFrame(const int theword[3], boolean frame[11][11]) {
    for (int i = 0; i < theword[2]; i++){
        frame[theword[0]][theword[1] + i] = true;
    }
}

void WClock::showTime(int hours, int minutes, int seconds) {

    int h = hours;
    int h2 = h;
    int m = minutes;
    int s = seconds;


    for (int r = 0; r < 11; r++){
        for (int c = 0; c < 11 ; c++){
            frame[r][c] = false;
        }
    }

    addWordToFrame(w_es, frame);
    addWordToFrame(w_ist, frame);

    if(m == 0) {
        addWordToFrame(w_uhr, frame);
    } else {
        if(m >= 5 && m < 10) {
            addWordToFrame(w_minutes[0], frame);
            addWordToFrame(w_nach, frame);
        }
        else if( m >= 10 && m < 15) {
            addWordToFrame(w_minutes[1], frame);
            addWordToFrame(w_nach, frame);
        }
        else if( m >= 15 && m < 20) {
            addWordToFrame(w_minutes[4], frame);
            addWordToFrame(w_nach, frame);
        }
        else if( m >= 20 && m < 25) {
            addWordToFrame(w_minutes[1], frame);
            addWordToFrame(w_vor, frame);
            addWordToFrame(w_minutes[5], frame);
        }
        else if( m >= 25 && m < 30) {
            addWordToFrame(w_minutes[0], frame);
            addWordToFrame(w_vor, frame);
            addWordToFrame(w_minutes[5], frame);
        }
        else if(m >= 30 && m < 35) {
            addWordToFrame(w_minutes[5], frame);
        }
        else if( m >= 35 && m < 40) {
            addWordToFrame(w_minutes[0], frame);
            addWordToFrame(w_nach, frame);
            addWordToFrame(w_minutes[5], frame);
        }
       else if( m >= 40 && m < 45) {
            addWordToFrame(w_minutes[1], frame);
            addWordToFrame(w_nach, frame);
            addWordToFrame(w_minutes[5], frame);
        }
       else if( m >= 45 && m < 50) {
            addWordToFrame(w_minutes[4], frame);
            addWordToFrame(w_vor, frame);
        }
       else if( m >= 50 && m < 55) {
            addWordToFrame(w_minutes[1], frame);
            addWordToFrame(w_vor, frame);
        }
       else if( m >= 55 && m < 60) {
            addWordToFrame(w_minutes[0], frame);
            addWordToFrame(w_vor, frame);
        }



    }

    if(!(m == 0 && (h == 0 || h == 12))) {
        if(h2 == 0) {
            addWordToFrame(w_hours[11], frame);
        }
        else if (h2 <= 12) {
            addWordToFrame(w_hours[h2 - 1], frame);
        }
        else {
            addWordToFrame(w_hours[h2 - 13], frame);
        }
        if(h2 == 11 || h2 == 23) {
            addWordToFrame(w_hours[10], frame);
        }
    }

    // addWordToFrame(w_drei, frame);
    // addWordToFrame(w_nach, frame);
    // addWordToFrame(w_zwanzig, frame);
    // addWordToFrame(w_sechs, frame);
    // addWordToFrame(w_zehn, frame);
    // addWordToFrame(w_uhr, frame);

    // addWordToFrame(w_funk, frame);

    int singleMinutes = m % 5;
    switch (singleMinutes) {
        case 1:
            addWordToFrame(w_minutes[6],frame);
            break;
        case 2:
            addWordToFrame(w_minutes[6],frame);
            addWordToFrame(w_minutes[7],frame);
            break;
        case 3:
            addWordToFrame(w_minutes[6],frame);
            addWordToFrame(w_minutes[7],frame);
            addWordToFrame(w_minutes[8],frame);
            break;
        case 4:
            addWordToFrame(w_minutes[6],frame);
            addWordToFrame(w_minutes[7],frame);
            addWordToFrame(w_minutes[8],frame);
            addWordToFrame(w_minutes[9],frame);
            break;
    }

    updateDisplay(prevframe, frame);

    for (int r = 0; r < 11; r++){
        for (int c = 0; c < 11 ; c++){
           prevframe[r][c]  = frame[r][c];
        }
    }

}
