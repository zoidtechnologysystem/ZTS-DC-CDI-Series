#define PULSE 58
int main(){
  // TIMING MAP (0 sampai 29)
  uint8_t map0 [30] = {0,5,5,15,22,30,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,37,36,35,34,34,34,34,34,34};
  uint32_t LATE, TIMING;
  PORTD = 0x00;
  DDRD  = 0x20;   // Pin D5 Output CDI Trigger
  TCCR1B = 0x03;  // Prescaler 64
    while (true){
    // Reading pulser pin D2
    while ((PIND >> 2 & 0x01) == 1);
    while ((PIND >> 2 & 0x01) == 0);
    LATE = TCNT1;
    TCNT1 = 0; // Reset Timer secepat mungkin
    if (LATE > 0) {
      // Rumus baru agar LATE kecil (RPM tinggi) menghasilkan index yang tepat
      // Ditambah pembatasan (constrain) agar tidak Out of Bounds
      TIMING = 750000 / LATE; 
      if (TIMING > 29) TIMING = 29; 
    } else {
      TIMING = 0;
    }
    // Kalkulasi delay derajat tunggu sebelum letupan SCR
    // Diputar perkaliannya dulu baru dibagi 360 (Fixed-point math)
    LATE = (LATE * (PULSE - map0[TIMING])) / 360;
    if(TIMING <= 29){
      while(TCNT1 < LATE);
      PORTD = 0x20;          // Ignition ON
      while(TCNT1 < LATE + 25);
      PORTD = 0x00;          // Ignition OFF
    }
  }
}
