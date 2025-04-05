// Set up the external interrupt pins INT2 and INT3
// for falling edge triggered. Use bare-metal.
void setupEINT()
{
  // Use bare-metal to configure pins 18 and 19 to be
  // falling edge triggered. Remember to enable
  // the INT2 and INT3 interrupts.
  // Hint: Check pages 110 and 111 in the ATmega2560 Datasheet.
  
  EICRA |= (1 << ISC20); // Set INT2 and INT3 to CHANGE edge
  EICRA &= ~(1 << ISC21);
  EICRA |= (1 << ISC30);
  EICRA &= ~(1 << ISC31);
  EIMSK |= (1 << INT2) | (1 << INT3); // Enable INT2 and INT3 interrupts  

}
