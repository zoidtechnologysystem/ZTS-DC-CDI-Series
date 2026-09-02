/*
 ===============================================================================================
 =================================== ZOID TECHNOLOGY SYSTEM ====================================
 ======================================= SHADOW MASTER ========================================
 ===============================================================================================
*/

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define ANCHOR_DEG_X10 150U
#define MAP_POINTS 30U
#define LIMIT_LAUNCH_CONTROL 4500UL
#define LIMIT_RPM_LOW 9000UL
#define LIMIT_RPM_HIGH 12500UL
static const uint16_t rpm_axis[MAP_POINTS] PROGMEM = {0, 400, 800, 1200, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 10500, 11000, 11500, 12000, 12500, 13000, 13500, 14000};
static const uint8_t ignition_maps[2][MAP_POINTS] PROGMEM = {
    // Kurva Pertalite
    {15, 15, 15, 15, 18, 22, 26, 29, 31, 32, 33, 33, 34, 34, 34, 34, 35, 36, 36, 35, 34, 32, 30, 28, 25, 20, 15, 10, 5, 0},
    // Kurva Pertamax
    {15, 15, 15, 15, 19, 24, 28, 31, 33, 34, 35, 36, 36, 37, 37, 37, 38, 38, 38, 37, 35, 33, 31, 28, 25, 20, 15, 10, 5, 0}
};
volatile struct {uint16_t ticks_per_rev; uint16_t capture_snap; uint8_t  new_pulse;} cdi_data = {0, 0, 0};
static uint8_t find_map_segment(uint32_t rpm) {
  uint8_t low = 0;
  uint8_t high = MAP_POINTS - 1U;
  uint8_t mid;
  if (rpm <= pgm_read_word(&rpm_axis[0])) return 0U;
  if (rpm >= pgm_read_word(&rpm_axis[high])) return MAP_POINTS - 2U;
  while ((uint8_t)(high - low) > 1U) {
    mid = low + ((high - low) / 2U);
    if (rpm < pgm_read_word(&rpm_axis[mid])) high = mid;
    else low = mid;
  }
  return low;
}
static uint8_t calculate_advance(uint32_t rpm, uint8_t map_index) {
  uint16_t rpm_low = pgm_read_word(&rpm_axis[0]);
  uint16_t rpm_high = pgm_read_word(&rpm_axis[MAP_POINTS - 1U]);
  if (rpm <= rpm_low)  return pgm_read_byte(&ignition_maps[map_index][0]);
  if (rpm >= rpm_high) return pgm_read_byte(&ignition_maps[map_index][MAP_POINTS - 1U]);
  uint8_t lower = find_map_segment(rpm);
  uint8_t upper = lower + 1U;
  uint16_t rpm_lower = pgm_read_word(&rpm_axis[lower]);
  uint16_t rpm_upper = pgm_read_word(&rpm_axis[upper]);
  uint8_t advance_lower = pgm_read_byte(&ignition_maps[map_index][lower]);
  uint8_t advance_upper = pgm_read_byte(&ignition_maps[map_index][upper]);
  uint32_t rpm_delta = (uint32_t)(rpm_upper - rpm_lower);
  uint32_t rpm_position = rpm - rpm_lower;
  int16_t advance_delta = (int16_t)advance_upper - (int16_t)advance_lower;
  int32_t result = (int32_t)advance_lower;
  result += (((int32_t)advance_delta * (int32_t)rpm_position) / (int32_t)rpm_delta);
  if (result < 0)   result = 0;
  if (result > 255) result = 255;
  return (uint8_t)result;
}
ISR(TIMER1_CAPT_vect) {
  static uint16_t last_capture_val = 0;
  uint16_t current_capture = ICR1;
  cdi_data.ticks_per_rev = current_capture - last_capture_val;
  cdi_data.capture_snap = current_capture;
  cdi_data.new_pulse = 1;
  last_capture_val = current_capture;
}
ISR(TIMER1_COMPA_vect) {
  PORTD |= (1 << PORTD5) | (1 << PORTD4);
  OCR1B = TCNT1 + 25;
  TIFR1 |= (1 << OCF1B);
  TIMSK1 |= (1 << OCIE1B);
  TIMSK1 &= ~(1 << OCIE1A);
}
ISR(TIMER1_COMPB_vect) {
  PORTD &= ~((1 << PORTD5) | (1 << PORTD4));
  TIMSK1 &= ~(1 << OCIE1B);
}
int main(void) {
  DDRD  |= (1 << DDD5) | (1 << DDD4);
  PORTD &= ~((1 << PORTD5) | (1 << PORTD4));
  DDRD  &= ~(1 << DDD6);
  PORTD |= (1 << PORTD6);
  DDRD  &= ~(1 << DDD2);
  PORTD |= (1 << PORTD2);
  DDRD  &= ~(1 << DDD3);
  PORTD |= (1 << PORTD3);
  DDRD  &= ~(1 << DDD7);
  PORTD |= (1 << PORTD7);
  TCCR1A = 0x00;
  TCCR1B = (1 << ICNC1) | (0 << ICES1) | (1 << CS11) | (1 << CS10);
  TIMSK1 = (1 << ICIE1);
  sei();
  uint16_t current_late = 0;
  uint16_t capture_anchor = 0;
  uint32_t current_rpm = 0;
  uint8_t  advance_deg = 0;
  uint8_t  map_index = 0;
  uint32_t active_limiter = LIMIT_RPM_HIGH;
  while (1) {
    if (cdi_data.new_pulse) {
      cli();
      cdi_data.new_pulse = 0;
      current_late = cdi_data.ticks_per_rev;
      capture_anchor = cdi_data.capture_snap;
      sei();
      if (!(PIND & (1 << PIND2))) map_index = 1;
      else map_index = 0;
      if (!(PIND & (1 << PIND7))) active_limiter = LIMIT_LAUNCH_CONTROL;
      else if (!(PIND & (1 << PIND3))) active_limiter = LIMIT_RPM_LOW;
      else active_limiter = LIMIT_RPM_HIGH;
      if (current_late > 0) {
        current_rpm = 15000000UL / (uint32_t)current_late;
        advance_deg = calculate_advance(current_rpm, map_index);
      }
      else {
        advance_deg = 15;
        current_rpm = 0;
      }
      if (current_rpm > 60) {
        if (current_rpm >= active_limiter) {
          cli();
          TIMSK1 &= ~(1 << OCIE1A);
          sei();
        }
        else if (current_rpm < 1400) {
          uint8_t random_factor = (uint8_t)(TCNT1 & 0x07);
          if (current_rpm > 700 && random_factor == 5) {
            cli();
            TIMSK1 &= ~(1 << OCIE1A);
            sei();
          }
          else {
            uint32_t idle_wait_ticks = ((uint32_t)current_late * 110U) / 3600UL;
            uint16_t idle_target = capture_anchor + (uint16_t)idle_wait_ticks;
            cli();
            OCR1A = idle_target;
            TIFR1 |= (1 << OCF1A);
            TIMSK1 |= (1 << OCIE1A);
            sei();
          }
        }
        else {
          uint16_t advance_deg_x10 = (uint16_t)advance_deg * 10U;
          uint32_t wait_ticks = 0;
          if (advance_deg_x10 > ANCHOR_DEG_X10) {
            uint16_t advance_diff = advance_deg_x10 - ANCHOR_DEG_X10;
            uint16_t travel_deg_x10 = 3600U - advance_diff;
            wait_ticks = ((uint32_t)current_late * (uint32_t)travel_deg_x10) / 3600UL;
          }
          else {
            uint16_t wait_deg_x10 = ANCHOR_DEG_X10 - advance_deg_x10;
            wait_ticks = ((uint32_t)current_late * (uint32_t)wait_deg_x10) / 3600UL;
          }
          uint16_t target_ocr1a = capture_anchor + (uint16_t)wait_ticks;
          cli();
          uint16_t current_tcnt1 = TCNT1;
          if ((int16_t)(target_ocr1a - current_tcnt1) <= 4) {
            PORTD |= (1 << PORTD5) | (1 << PORTD4);
            OCR1B = current_tcnt1 + 25;
            TIFR1 |= (1 << OCF1B);
            TIMSK1 |= (1 << OCIE1B);
            TIMSK1 &= ~(1 << OCIE1A);
          }
          else {
            OCR1A = target_ocr1a;
            TIFR1 |= (1 << OCF1A);
            TIMSK1 |= (1 << OCIE1A);
          }
          sei();
        }
      }
    }
  }
  return 0;
}
