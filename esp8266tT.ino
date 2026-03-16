// --- TRANSMITTER V_OOK_PACKET (Sends String Packet) ---

const int LED_PIN = 5; // D1

String message = "Optical Data Transmission";
char startMarker = '@';
char endMarker = '@';

int bit_duration = 500; // 500ms per bit

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED OFF
  Serial.begin(115200); // Optional, for monitoring
}

void loop() {
  Serial.println("--- Sending New Packet ---");
  
  // 1. Send the Start Marker '@'
  Serial.print("Sending Start Marker: @");
  send_char(startMarker);
  Serial.println(" ...Done.");

  // 2. Send the message, character by character
  for (int i = 0; i < message.length(); i++) {
    char c = message.charAt(i);
    Serial.print("Sending Char: "); Serial.print(c);
    send_char(c);
    Serial.println(" ...Done.");
  }

  // 3. Send the End Marker '@'
  Serial.print("Sending End Marker: @");
  send_char(endMarker);
  Serial.println(" ...Done.");

  // 4. Wait for 5 seconds (silence)
  Serial.println("--- End of Packet. Pausing... ---\n");
  digitalWrite(LED_PIN, LOW);
  delay(5000);
}

// This function sends one full character (8 bits)
void send_char(char c) {
  // 1. Send a Start Signal (for this character)
  digitalWrite(LED_PIN, HIGH);
  delay(1500); // 1.5s ON pulse
  
  // 2. Send a Separation Gap
  digitalWrite(LED_PIN, LOW);
  delay(500); // 0.5s OFF gap

  // 3. Send the 8 bits of the character
  for (int i = 7; i >= 0; i--) { // MSB first
    if (bitRead(c, i) == 1) {
      digitalWrite(LED_PIN, HIGH); // LEDs ON for '1'
    } else {
      digitalWrite(LED_PIN, LOW);  // LEDs OFF for '0'
    }
    delay(bit_duration); // Hold for 500ms
  }
}