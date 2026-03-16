// --- RECEIVER V_OOK_PACKET (Receives String Packet) ---

const int SENSOR_PIN = 35; // Make sure this is 35!

// --- TUNE THESE VALUES ---
const int ON_OFF_THRESHOLD = 200;      // Set halfway between OFF (0) and stable ON
const int START_PULSE_THRESHOLD = 200; // Set to a reliable 'ON' level

// --- Packet Markers ---
const char START_MARKER = '@';
const char END_MARKER = '@';
String messageBuffer = ""; // Holds the incoming message
bool receivingMessage = false; // Flag

// --- Sampling parameters ---
const unsigned long timeBetweenSamples = 500; // 500 µs
const int decision_point = 1000;              // 500ms window

// --- State ---
unsigned long lastSampleTime = 0;
double accumulator = 0.0;
int sample_count = 0;
byte receivedByte = 0; // We build the 8-bit character here

// --- State Machine Definition ---
enum State { WAITING_FOR_START, SKIPPING_GAP, READING_BITS };
State currentState = WAITING_FOR_START;
int bit_counter = 0;
const int MESSAGE_LENGTH = 8;
int windows_to_skip = 0; 

void setup() {
  Serial.begin(115200);
  Serial.println("OOK String Receiver Ready. Waiting for Start Marker '@'...");
  lastSampleTime = micros();
}

void loop() {
  // --- Sampling Timer ---
  if (micros() - lastSampleTime < timeBetweenSamples) {
    return;
  }
  lastSampleTime += timeBetweenSamples;

  // --- Accumulate Signal ---
  accumulator += (double)analogRead(SENSOR_PIN);
  sample_count++;

  // --- Make Decision ---
  if (sample_count >= decision_point) {
    
    double average_light_level = accumulator / decision_point;
    
    // --- State Machine Logic ---
    switch (currentState) {
      
      case WAITING_FOR_START:
        Serial.print("W..."); // Waiting
        
        if (average_light_level > START_PULSE_THRESHOLD) {
          Serial.println("\n--- START PULSE DETECTED! ---");
          Serial.println("Skipping rest of pulse & gap...");
          windows_to_skip = 3; 
          currentState = SKIPPING_GAP;
        }
        break;

      case SKIPPING_GAP:
        Serial.print("S-"); // Skipping
        
        windows_to_skip--; 
        
        if (windows_to_skip <= 0) {
          Serial.println("\n--- Ready to Read Bits ---");
          currentState = READING_BITS;
          bit_counter = 0;
          receivedByte = 0; // Clear the byte for the new char
        }
        break;
        
      case READING_BITS:
        // *** SIMPLE ON-OFF DECISION ***
        if (average_light_level > ON_OFF_THRESHOLD) {
          // It's a '1' bit
          Serial.print("1");
          bitSet(receivedByte, 7 - bit_counter); // Build the byte MSB first
        } else {
          // It's a '0' bit
          Serial.print("0");
          // No need to do anything, bit is already 0
        }
        
        bit_counter++;
        
        if (bit_counter >= MESSAGE_LENGTH) {
          // --- We have received all 8 bits ---
          char receivedChar = (char)receivedByte; // Convert the byte to a char
          Serial.print(" -> Char: ");
          Serial.println(receivedChar);

          // --- Packet Logic ---
          if (receivedChar == START_MARKER && !receivingMessage) {
            // This is the START of a new message
            receivingMessage = true;
            messageBuffer = ""; // Clear the buffer
            Serial.println("--- Start Marker Received ---");
          } 
          else if (receivedChar == END_MARKER && receivingMessage) {
            // This is the END of the message
            receivingMessage = false;
            Serial.println("\n--- End Marker Received ---");
            Serial.println("FINAL MESSAGE:");
            Serial.println(messageBuffer); // Print the complete, buffered message!
            Serial.println("---------------------------\n");
          }
          else if (receivingMessage) {
            // This is a normal character, add it to the buffer
            messageBuffer += receivedChar;
          }
          
          currentState = WAITING_FOR_START; // Go back to waiting for the next start pulse
        }
        break;
    }

    // Reset accumulators for the next window
    accumulator = 0.0;
    sample_count = 0;
  }
}