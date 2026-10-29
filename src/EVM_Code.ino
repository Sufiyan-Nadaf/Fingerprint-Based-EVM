// ── Libraries ──────────────────────────────────────────────
#include <LiquidCrystal.h>         // Standard parallel LCD library
#include <Adafruit_Fingerprint.h>  // R307 fingerprint sensor
#include <SoftwareSerial.h>        // Software UART for R307
#include <EEPROM.h>                // Internal ATmega328P EEPROM


LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// R307 Fingerprint Sensor on D2(RX) / D3(TX) 
SoftwareSerial fpSerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);

//  PIN DEFINITIONS


// Candidate vote buttons 
#define BTN_A        4   // Vote for Candidate A
#define BTN_B        5   // Vote for Candidate B
#define BTN_C        6   // Vote for Candidate C
#define BTN_D        A5  // Vote for Candidate D 

// Control inputs
#define MODE_SWITCH  A2  // DPST: LOW=Register, HIGH=Vote
#define RESULT_BTN   14  // A0 = digital pin 14 — short=winner, hold 3s=admin
#define RESET_BTN    15  // A1 = digital pin 15 — election reset

// Feedback outputs
#define GREEN_LED    A3  // Lights on success (analog pin as digital out)
#define RED_LED      A4  // Lights on error / rejection
#define BUZZER       13  // D13 — tone() 

//  EEPROM ADDRESS CONSTANTS
#define ADDR_VOTE_A      0   // uint8_t: Candidate A vote count
#define ADDR_VOTE_B      1   // uint8_t: Candidate B vote count
#define ADDR_VOTE_C      2   // uint8_t: Candidate C vote count
#define ADDR_VOTE_D      3   // uint8_t: Candidate D vote count
#define ADDR_ADMIN_ID    4   // uint8_t: admin fingerprint ID (0=not set)
#define ADDR_NEXT_ID     5   // uint8_t: next enrollment slot (1–127)
#define ADDR_VOTED_BASE  10  // byte per voter: EEPROM[10+id]=1 means voted

// R307 internal flash holds up to 127 fingerprint templates
#define MAX_VOTERS       127

// Reject fingerprint matches with confidence below this value.
#define MIN_CONFIDENCE   50

// How long (ms) RESULT_BTN must be held to trigger admin mode
#define ADMIN_HOLD_MS    3000

//  GLOBAL STATE
uint8_t voteA = 0, voteB = 0, voteC = 0, voteD = 0;
uint8_t nextID  = 1;  
uint8_t adminID = 0;  

//  SETUP
void setup() {

  finger.begin(0x00000000);
  // Button inputs — internal pull-up, active LOW
  pinMode(BTN_A,            INPUT_PULLUP);
  pinMode(BTN_B,            INPUT_PULLUP);
  pinMode(BTN_C,            INPUT_PULLUP);
  pinMode(BTN_D,            INPUT_PULLUP);
  pinMode(MODE_SWITCH,      INPUT_PULLUP);
  pinMode(RESULT_BTN,       INPUT_PULLUP);
  pinMode(RESET_BTN,        INPUT_PULLUP);

  // Output devices
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED,   OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED,   LOW);

  // Start 16×2 parallel LCD (16 cols, 2 rows)
  lcd.begin(16, 2);

  // Debug serial — open Serial Monitor at 9600 baud
  Serial.begin(9600);

  // Start R307 fingerprint sensor at 57600 baud
  fpSerial.begin(57600);
  finger.begin(57600);

  // Verify R307 is connected — halt if not found
  if (!finger.verifyPassword()) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("R307 Not Found!");
    lcd.setCursor(0, 1); lcd.print("Check D2/D3 wire");
    Serial.println("ERROR: R307 not detected");
    while (true) { errorSignal(); delay(2000); }
  }

  // ── Load votes from EEPROM ──
  // Fresh EEPROM returns 0xFF — treat as 0
  voteA = EEPROM.read(ADDR_VOTE_A); if (voteA == 0xFF) voteA = 0;
  voteB = EEPROM.read(ADDR_VOTE_B); if (voteB == 0xFF) voteB = 0;
  voteC = EEPROM.read(ADDR_VOTE_C); if (voteC == 0xFF) voteC = 0;
  voteD = EEPROM.read(ADDR_VOTE_D); if (voteD == 0xFF) voteD = 0;

  // Load enrollment counter 
  nextID = EEPROM.read(ADDR_NEXT_ID);
  if (nextID < 1 || nextID > MAX_VOTERS) nextID = 1;

  // Load admin ID 
  adminID = EEPROM.read(ADDR_ADMIN_ID);
  if (adminID == 0xFF) adminID = 0;

  // Startup screen 
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Biometric Voting");
  lcd.setCursor(0, 1);
  lcd.print("V:"); lcd.print(nextID - 1);
  lcd.print(adminID ? " Adm:OK" : " NoAdmin");
  successSignal();
  delay(2500);
  lcd.clear();
}

//  MAIN LOOP
void loop() {

  // ELECTION RESET / FULL EEPROM RESET
  // Short press (<5s) → Election reset (votes + flags only)
  // Hold 5 seconds    → Full EEPROM reset (clears nextID + adminID)
  //                     Use this after running emptyDatabase()
  if (digitalRead(RESET_BTN) == LOW) {
    delay(60);
    if (digitalRead(RESET_BTN) == LOW) {

      unsigned long pressTime = millis();

      // Show hint so user knows hold is being detected
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Hold 5s:FullRST");
      lcd.setCursor(0, 1); lcd.print("Release:ElecRST");

      while (digitalRead(RESET_BTN) == LOW) {
        if (millis() - pressTime >= 5000) {
          fullEepromReset();
          return;
        }
      }

      // Released before 5s → election reset only
      resetElection();
      return;
    }
  }

  // RESULT BUTTON 
  // Short press  : show winner only (no auth)
  // Hold 3 seconds : show full counts + winner
  if (digitalRead(RESULT_BTN) == LOW) {
    delay(60); // debounce
    if (digitalRead(RESULT_BTN) == LOW) {

      unsigned long pressTime = millis();

      while (digitalRead(RESULT_BTN) == LOW) {
        if (millis() - pressTime >= ADMIN_HOLD_MS) {
          // Held long enough — go to admin-verified result
          showResultsAdmin();
          return;
        }
      }

      // Button released before 3s — show winner only
      showResultsPublic();
      return;
    }
  }

  // ── MODE SWITCH (DPST) ────────────────────────────────
  // Switch ON (LOW) → Register Mode
  // Switch OFF (HIGH) → Voting Mode
  if (digitalRead(MODE_SWITCH) == LOW) {
    registrationMode();
  } else {
    votingMode();
  }
}

//  FEEDBACK SIGNALS
void successSignal() {
  digitalWrite(GREEN_LED, HIGH);
  tone(BUZZER, 1500, 700);
  delay(200);
  noTone(BUZZER);
  digitalWrite(GREEN_LED, LOW);
}

void errorSignal() {
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER, 200, 1000);
  delay(700);
  noTone(BUZZER);
  digitalWrite(RED_LED, LOW);
}

//  RESULT — PUBLIC (short press)
void showResultsPublic() {

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("-- RESULT --");

  // Find the winner from current vote counts
  uint8_t maxVotes = voteA;
  char    winner   = 'A';
  bool    tie      = false;

  if (voteB > maxVotes)       { maxVotes = voteB; winner = 'B'; tie = false; }
  else if (voteB == maxVotes)   tie = true;

  if (voteC > maxVotes)       { maxVotes = voteC; winner = 'C'; tie = false; }
  else if (voteC == maxVotes)   tie = true;

  if (voteD > maxVotes)       { maxVotes = voteD; winner = 'D'; tie = false; }
  else if (voteD == maxVotes)   tie = true;

  lcd.setCursor(0, 1);
  if (maxVotes == 0) {
    lcd.print("No votes yet");
  } else if (tie) {
    lcd.print("Result: TIE!");
  } else {
    lcd.print("Winner: ");
    lcd.print(winner);
    lcd.print(" (");
    lcd.print(maxVotes);
    lcd.print(" votes)");
  }

  // Stay on screen until RESULT_BTN pressed again
  waitForResultBtnRelease();

  lcd.clear();
}

//  RESULT — ADMIN (hold 3s)
void showResultsAdmin() {

  // Block if admin was never enrolled
  if (adminID == 0) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("No Admin Set!");
    lcd.setCursor(0, 1); lcd.print("Enroll admin 1st");
    errorSignal();
    delay(2000);
    lcd.clear();
    return;
  }

  // Prompt admin to scan finger
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Admin Verify");
  lcd.setCursor(0, 1); lcd.print("Place Finger...");

  int id = getFingerprintID();

  // Reject if scan failed or ID doesn't match stored admin
  if (id == -1 || (uint8_t)id != adminID) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Access Denied!");
    lcd.setCursor(0, 1); lcd.print("Not Admin Finger");
    errorSignal();
    delay(2000);
    lcd.clear();
    return;
  }

  // Admin verified — show full result
  successSignal();

  // ── Screen 1: Vote counts for all candidates ──
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:"); lcd.print(voteA);
  lcd.print(" B:"); lcd.print(voteB);
  lcd.print(" C:"); lcd.print(voteC);
  lcd.print(" D:"); lcd.print(voteD);

  // ── Screen 2 (line 2): Winner ──
  uint8_t maxVotes = voteA;
  char    winner   = 'A';
  bool    tie      = false;

  if (voteB > maxVotes)       { maxVotes = voteB; winner = 'B'; tie = false; }
  else if (voteB == maxVotes)   tie = true;

  if (voteC > maxVotes)       { maxVotes = voteC; winner = 'C'; tie = false; }
  else if (voteC == maxVotes)   tie = true;

  if (voteD > maxVotes)       { maxVotes = voteD; winner = 'D'; tie = false; }
  else if (voteD == maxVotes)   tie = true;

  lcd.setCursor(0, 1);
  if (maxVotes == 0) {
    lcd.print("No votes cast");
  } else if (tie) {
    lcd.print("Result: TIE!");
  } else {
    lcd.print("Winner: ");
    lcd.print(winner);
    lcd.print(" (");
    lcd.print(maxVotes);
    lcd.print(")     ");
  }

  // Stay on screen until RESULT_BTN pressed again
  waitForResultBtnRelease();

  lcd.clear();
}

void waitForResultBtnRelease() {
  // Wait for press
  while (digitalRead(RESULT_BTN) != LOW);
  delay(60); // debounce
  // Wait for release
  while (digitalRead(RESULT_BTN) == LOW);
  delay(80);
}

//  VOTING MODE
void votingMode() {
  uint8_t img = finger.getImage();

  if (img == FINGERPRINT_NOFINGER) {
    // No finger present — show idle screen and return immediately
    lcd.setCursor(0, 0); lcd.print("Place Finger    ");
    lcd.setCursor(0, 1); lcd.print("to Vote...      ");
    delay(100); // small delay to avoid LCD flicker
    return;
  }

  // Finger detected 
  if (img != FINGERPRINT_OK) return; // sensor error, skip

  // Convert image to template
  if (finger.image2Tz() != FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Poor scan");
    lcd.setCursor(0, 1); lcd.print("Try again");
    delay(1000);
    lcd.clear();
    return;
  }

  // Search database for a match
  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    // No match found = unregistered voter
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Invalid User!");
    lcd.setCursor(0, 1); lcd.print("Not Registered");
    errorSignal();
    delay(2000);
    lcd.clear();
    return;
  }

  // Reject weak/partial matches
  if (finger.confidence < MIN_CONFIDENCE) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Invalid User!");
    lcd.setCursor(0, 1); lcd.print("Not Registered");
    errorSignal();
    delay(2000);
    lcd.clear();
    return;
  }

  int id = finger.fingerID;
  Serial.print("Match ID="); Serial.print(id);
  Serial.print(" Conf=");    Serial.println(finger.confidence);

  // ── REJECT: Duplicate vote ──
  if (hasVoted(id)) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Already Voted!");
    lcd.setCursor(0, 1); lcd.print("Duplicate Reject");
    errorSignal();
    delay(2500);
    lcd.clear();
    return;
  }

  // ACCEPT: Valid voter, not yet voted 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ID:"); lcd.print(id); lcd.print(" Verified!");
  successSignal();
  lcd.setCursor(0, 1); lcd.print("A  B  C  D = ?");

  unsigned long start = millis();
  bool voted = false;

  // 10-second window for the voter to press a button
  while (millis() - start < 10000) {
    int remaining = 10 - (int)((millis() - start) / 1000);
    lcd.setCursor(13, 1);
    lcd.print(remaining);
    lcd.print("s ");

    if (digitalRead(BTN_A) == LOW) { castVote(id, 'A'); voted = true; break; }
    if (digitalRead(BTN_B) == LOW) { castVote(id, 'B'); voted = true; break; }
    if (digitalRead(BTN_C) == LOW) { castVote(id, 'C'); voted = true; break; }
    if (digitalRead(BTN_D) == LOW) { castVote(id, 'D'); voted = true; break; }
  }

  if (!voted) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Vote Timeout!");
    lcd.setCursor(0, 1); lcd.print("Session Expired");
    errorSignal();
    delay(2000);
    lcd.clear();
  }
}

//  CAST VOTE
void castVote(int id, char candidate) {

  switch (candidate) {
    case 'A': voteA++; EEPROM.write(ADDR_VOTE_A, voteA); break;
    case 'B': voteB++; EEPROM.write(ADDR_VOTE_B, voteB); break;
    case 'C': voteC++; EEPROM.write(ADDR_VOTE_C, voteC); break;
    case 'D': voteD++; EEPROM.write(ADDR_VOTE_D, voteD); break;
  }

  // Write voted flag for this ID — prevents double voting
  EEPROM.write(ADDR_VOTED_BASE + id, 1);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Vote Cast!");
  lcd.setCursor(0, 1); lcd.print("Candidate ");
  lcd.print(candidate);
  successSignal();
  delay(2500);
  lcd.clear();
}


//  REGISTRATION MODE 
void registrationMode() {

  if (nextID > MAX_VOTERS) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("DB Full! Max 127");
    lcd.setCursor(0, 1); lcd.print("Reset to clear");
    errorSignal();
    delay(2000);
    lcd.clear();
    return;
  }

  bool enrollingAdmin = (adminID == 0); // true if no admin yet

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(enrollingAdmin ? "Enroll ADMIN" : "Enroll Voter");
  lcd.setCursor(0, 1);
  lcd.print("Slot: "); lcd.print(nextID);

  // Wait 1.5s but exit immediately if switch is flipped back
  unsigned long introStart = millis();
  while (millis() - introStart < 1500) {
    if (digitalRead(MODE_SWITCH) == HIGH) {
      lcd.clear(); return;
    }
    delay(50);
  }

  int id = enrollFingerprint();

  lcd.clear();
  if (id != -1) {
    // Persist updated slot counter
    EEPROM.write(ADDR_NEXT_ID, nextID);

    if (enrollingAdmin) {
      // Save this ID as the admin
      adminID = (uint8_t)id;
      EEPROM.write(ADDR_ADMIN_ID, adminID);
      lcd.setCursor(0, 0); lcd.print("Admin Set! ID:");
      lcd.print(id);
    } else {
      lcd.setCursor(0, 0); lcd.print("Enrolled! ID:");
      lcd.print(id);
    }

    lcd.setCursor(0, 1);
    lcd.print("Total: ");
    // Subtract 1 for admin when counting voters
    lcd.print(nextID - 1 - (adminID ? 1 : 0));
    successSignal();
  } else {
    lcd.setCursor(0, 0); lcd.print("Enroll Failed!");
    lcd.setCursor(0, 1); lcd.print("Try again");
    errorSignal();
  }

  delay(2500);
  lcd.clear();
}

//  ENROLL FINGERPRINT
int enrollFingerprint() {

  // ── Scan 1 ──
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Scan 1 of 2");
  lcd.setCursor(0, 1); lcd.print("Place Finger...");

  if (!waitForFinger(10000)) return -1;

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Poor image #1");
    delay(1500); return -1;
  }

  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Remove Finger");
  delay(1500);

  // Wait until finger is lifted — also exits if switch flipped
  unsigned long t = millis();
  while (millis() - t < 8000) {
    if (digitalRead(MODE_SWITCH) == HIGH) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Mode Changed");
      lcd.setCursor(0, 1); lcd.print("Enroll Cancelled");
      delay(1000);
      lcd.clear();
      return -1;
    }
    if (finger.getImage() == FINGERPRINT_NOFINGER) break;
    delay(50);
  }

  // ── Scan 2 ──
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Scan 2 of 2");
  lcd.setCursor(0, 1); lcd.print("Same Finger");

  if (!waitForFinger(10000)) return -1;

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Poor image #2");
    delay(1500); return -1;
  }

  // ── Merge both scans into a model ──
  uint8_t result = finger.createModel();
  if (result == FINGERPRINT_ENROLLMISMATCH) {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Scans Mismatch!");
    lcd.setCursor(0, 1); lcd.print("Use same finger");
    delay(1500); return -1;
  }
  if (result != FINGERPRINT_OK) {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Model Failed");
    delay(1500); return -1;
  }

  // ── Duplicate fingerprint check ──
  if (finger.fingerFastSearch() == FINGERPRINT_OK) {
    // Match found — finger already exists in database
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Already Exists!");
    lcd.setCursor(0, 1); lcd.print("ID:");
    lcd.print(finger.fingerID);
    lcd.print(" already reg.");
    errorSignal();
    delay(2500);
    return -1;
  }
  // FINGERPRINT_NOTFOUND is the expected result here — means it's new

  // ── Store model in R307 internal flash ──
  if (finger.storeModel(nextID) != FINGERPRINT_OK) {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Store Failed");
    delay(1500); return -1;
  }

  return nextID++; 
}

bool waitForFinger(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    // If MODE_SWITCH flipped to voting (HIGH), abort immediately
    if (digitalRead(MODE_SWITCH) == HIGH) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Mode Changed");
      lcd.setCursor(0, 1); lcd.print("Enroll Cancelled");
      delay(1000);
      lcd.clear();
      return false;
    }
    if (finger.getImage() == FINGERPRINT_OK) return true;
    delay(80);
  }
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Timeout!");
  lcd.setCursor(0, 1); lcd.print("No finger placed");
  delay(1000);
  return false;
}

//  MATCH FINGERPRINT
int getFingerprintID() {
  unsigned long start = millis();

  while (millis() - start < 5000) {
    uint8_t p = finger.getImage();

    if (p == FINGERPRINT_NOFINGER) { delay(80); continue; }
    if (p != FINGERPRINT_OK) return -1;

    if (finger.image2Tz() != FINGERPRINT_OK) return -1;

    // fingerFastSearch returns FINGERPRINT_NOTFOUND if no match
    // → that becomes -1 → "Invalid User" shown to voter
    if (finger.fingerFastSearch() != FINGERPRINT_OK) return -1;

    if (finger.confidence < MIN_CONFIDENCE) {
      Serial.print("Weak match, conf=");
      Serial.println(finger.confidence);
      return -1;
    }

    Serial.print("Match ID="); Serial.print(finger.fingerID);
    Serial.print(" Conf=");    Serial.println(finger.confidence);
    return finger.fingerID;
  }

  return -1; 
}

// already voted?
bool hasVoted(int id) {
  if (id < 1 || id > MAX_VOTERS) return true; 
  return EEPROM.read(ADDR_VOTED_BASE + id) == 1;
}

//  ELECTION RESET 
void resetElection() {

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Election Reset");
  lcd.setCursor(0, 1); lcd.print("Please wait...");
  delay(2000);

  voteA = voteB = voteC = voteD = 0;
  EEPROM.write(ADDR_VOTE_A, 0);
  EEPROM.write(ADDR_VOTE_B, 0);
  EEPROM.write(ADDR_VOTE_C, 0);
  EEPROM.write(ADDR_VOTE_D, 0);

  for (int i = 0; i <= MAX_VOTERS; i++) {
    EEPROM.write(ADDR_VOTED_BASE + i, 0);
  }

  // fingerprints NOT cleared 
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Votes Cleared!");
  lcd.setCursor(0, 1); lcd.print("Voters kept:");
  lcd.print(nextID - 1);
  successSignal();
  delay(2500);
  lcd.clear();
}


//  FULL EEPROM SYNC RESET 
void fullEepromReset() {

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("EEPROM Sync RST");
  lcd.setCursor(0, 1); lcd.print("Please wait...");
  delay(2000);

  // Clear vote counts in RAM and EEPROM
  voteA = voteB = voteC = voteD = 0;
  EEPROM.write(ADDR_VOTE_A, 0);
  EEPROM.write(ADDR_VOTE_B, 0);
  EEPROM.write(ADDR_VOTE_C, 0);
  EEPROM.write(ADDR_VOTE_D, 0);

  // Clear all voted flags
  for (int i = 0; i <= MAX_VOTERS; i++) {
    EEPROM.write(ADDR_VOTED_BASE + i, 0);
  }

  // Reset enrollment counter — sensor DB is empty so start from 1
  nextID = 1;
  EEPROM.write(ADDR_NEXT_ID, nextID);

  // Clear admin ID — first enrollment will become admin again
  adminID = 0;
  EEPROM.write(ADDR_ADMIN_ID, 0);

  // Wipe all fingerprint templates from R307 internal flash
  finger.emptyDatabase();

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("EEPROM Cleared!");
  lcd.setCursor(0, 1); lcd.print("Enroll Admin 1st");
  successSignal();
  delay(2500);
  lcd.clear();
}