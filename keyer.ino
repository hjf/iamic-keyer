//#define UNO
#include <avr/sleep.h>
#ifdef UNO
#define PADDLE_LEFT 2
#define PADDLE_RIGHT 3
#define POT_INPUT A0
#define POT_ENABLE A1
//#define OUTPUT_TIP 4
#define OUTPUT_TIP LED_BUILTIN

#else
#define PADDLE_LEFT 1
#define PADDLE_RIGHT 0
#define POT_INPUT 3
#define POT_ENABLE 2
#define OUTPUT_TIP 4

#endif

#define DIT (1)
#define DAH (3)

#ifdef UNO
#include "LowPower.h"
#endif
void setup() {
#ifdef UNO
  Serial.begin(9600);
#endif
  pinMode(PADDLE_LEFT, INPUT_PULLUP);
  pinMode(PADDLE_RIGHT, INPUT);

  pinMode(POT_ENABLE, OUTPUT);
  digitalWrite(POT_ENABLE, LOW);

  pinMode(OUTPUT_TIP, OUTPUT);
  digitalWrite(OUTPUT_TIP, LOW);
}

void wakeUp()
{
  // Just a handler for the pin interrupt.
}


void loop() {
  static uint32_t speed_measure;
  static int keyspeed;
  static char sleepcnt = 5;
  static uint32_t modeswitch_d = 0;
  static char modeswitch = 40;
  static bool last_rd;

  static bool paddle_left = false;
  static bool paddle_right = false;
  static uint32_t paddle_delay = 0;


  //detect paddle mode
  if (millis() > modeswitch_d) {
    modeswitch_d = millis() + 100;
    pinMode(PADDLE_RIGHT, INPUT);
    if (digitalRead(PADDLE_RIGHT)) {
      if (modeswitch > 0)
        modeswitch--;
      if (modeswitch == 1) {
#ifdef UNO
        Serial.println(F("Radio has iambic input"));
#endif
      }
    } else {
#ifdef UNO
      if (modeswitch != 40)
        Serial.println(F("Radio has straight input"));
#endif
      modeswitch = 40;
    }

  }

  if (modeswitch == 0) {
    bool rd = !digitalRead(PADDLE_LEFT);
    if (rd != last_rd) {
      last_rd = rd;
      sleepcnt = 5;
    }
    digitalWrite(OUTPUT_TIP, rd);

  } else {
    pinMode(PADDLE_RIGHT, INPUT_PULLUP);

    if (paddle_left || digitalRead(PADDLE_LEFT) == LOW) {
      paddle_left = 0;
      paddle_delay = millis() + keyspeed * DIT;
      digitalWrite(OUTPUT_TIP, HIGH);
      while (millis() < paddle_delay) {
        if (digitalRead(PADDLE_RIGHT) == LOW)
          paddle_right = true;
      }

      paddle_delay = millis() + keyspeed * DIT;
      digitalWrite(OUTPUT_TIP, LOW);
      while (millis() < paddle_delay) {
        if (digitalRead(PADDLE_RIGHT) == LOW)
          paddle_right = true;
      }
      sleepcnt = 5;
    }

    if (paddle_right || digitalRead(PADDLE_RIGHT) == LOW) {
      paddle_right = 0;
      paddle_delay = millis() + keyspeed * DAH;
      digitalWrite(OUTPUT_TIP, HIGH);
      while (millis() < paddle_delay) {
        if (digitalRead(PADDLE_LEFT) == LOW)
          paddle_left = true;
      }

      paddle_delay = millis() + keyspeed * DIT;
      digitalWrite(OUTPUT_TIP, LOW);
      while (millis() < paddle_delay) {
        if (digitalRead(PADDLE_LEFT) == LOW)
          paddle_left = true;
      }
      sleepcnt = 5;
    }
  }

  if (millis() > speed_measure) {
    speed_measure = millis() + 1000;

    if (modeswitch > 0) {
      pinMode(POT_INPUT, INPUT);
      digitalWrite(POT_ENABLE, HIGH);
      delay(1);

      keyspeed = map(analogRead(POT_INPUT), 10, 1023, 25, 200);
      digitalWrite(POT_ENABLE, LOW);
      pinMode(POT_INPUT, OUTPUT);
      digitalWrite(POT_INPUT, LOW);
    }
    sleepcnt--;

    if (sleepcnt == 0) {
#ifdef UNO
      Serial.println(F("No activity, sleeping"));
      Serial.flush ();
      // wait for transmit buffer to empty
      while ((UCSR0A & _BV (TXC0)) == 0);
#endif
      attachInterrupt(0, wakeUp, CHANGE);
      attachInterrupt(1, wakeUp, CHANGE);
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      cli();
      sleep_enable();
      //      sleep_bod_disable();
      sei();
      sleep_cpu();
      sleep_disable();
      sei();
#ifdef UNO


      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      detachInterrupt(0);
      detachInterrupt(1);

#else
#endif
#ifdef UNO
      Serial.println(F("Up"));
#endif
      sleepcnt = 5;
    }
  }
}
