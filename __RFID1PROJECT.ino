#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

// Pin Definitions
#define RST_PIN 9       // Reset pin for RFID module
#define SS_PIN 10       // Slave select pin for RFID module
#define ADMIN_CARD "F36C7114" // Replace with your administrator card UID

// RFID and Bluetooth setup
MFRC522 mfrc522(SS_PIN, RST_PIN);
SoftwareSerial bluetooth(2, 3); // RX, TX for Bluetooth module

int totalSeats = 30; // Total available seats
bool adminDetected = false;
bool assignedSeats[30] = {false}; // Track assigned seats

// Define classroom number range
int minClassroomNumber = 101; // Minimum classroom number
int maxClassroomNumber = 110;  // Maximum classroom number

void setup() {
  Serial.begin(9600);     // Initialize serial communication for debugging
  bluetooth.begin(9600);  // Initialize Bluetooth communication
  SPI.begin();            // Initialize SPI bus
  mfrc522.PCD_Init();     // Initialize RFID reader
  randomSeed(analogRead(0)); // Initialize random seed
  Serial.println("Place your card...");
}

void loop() {
  // Check for new card presence
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Get card UID as a string
  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  cardUID.toUpperCase(); // Convert to uppercase for consistency

  Serial.print("Card UID: ");
  Serial.println(cardUID);

  // Check if it's the administrator card
  if (!adminDetected && cardUID == ADMIN_CARD) {
    Serial.println("Administrator detected. Terminating card check loop.");
    adminDetected = true;
    Serial.println("Now scanning for student cards...");
  }

  // Continue checking student cards after admin card is detected
  if (adminDetected) {
    mfrc522.PICC_HaltA(); // Halt communication with the current card

    while (true) {
      // Check for new card presence
      if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        continue;
      }

      // Get student card UID
      String studentCardUID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        studentCardUID += String(mfrc522.uid.uidByte[i], HEX);
      }
      studentCardUID.toUpperCase();

      Serial.print("Student Card UID: ");
      Serial.println(studentCardUID);

      // Assign a random seat number
      int seatNumber;
      do {
        seatNumber = random(1, totalSeats + 1);
      } while (assignedSeats[seatNumber - 1]); // Ensure the seat is not already assigned

      assignedSeats[seatNumber - 1] = true; // Mark the seat as assigned
      Serial.print("Assigned Seat: ");
      Serial.println(seatNumber);

      // Assign a random classroom number
      int classroomNumber = random(minClassroomNumber, maxClassroomNumber + 1);
      Serial.print("Assigned Classroom: ");
      Serial.println(classroomNumber);

      // Send data to Bluetooth module
      bluetooth.print("Student Card UID: ");
      bluetooth.print(studentCardUID);
      bluetooth.print(", Seat Number: ");
      bluetooth.print(seatNumber);
      bluetooth.print(", Classroom Number: ");
      bluetooth.println(classroomNumber);

      // Halt communication with the current card
      mfrc522.PICC_HaltA();
    }
  }

  // Halt communication with the current card
  mfrc522.PICC_HaltA();
}
