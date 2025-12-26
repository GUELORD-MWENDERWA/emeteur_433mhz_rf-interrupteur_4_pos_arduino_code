#include <RH_ASK.h>
#include <SPI.h> // Nécessaire pour la compilation

// ------------------------------------------------------------------
// CONFIGURATION
// ------------------------------------------------------------------
const int NUM_BUTTONS = 4;
const int boutonPins[NUM_BUTTONS] = {2, 3, 4, 5};  // INPUT_PULLUP
const unsigned long DEBOUNCE_DELAY = 50;           // ms

const char* EMETTEUR_ID = "TX1";
const size_t MSG_BUF_SIZE = 48;

// Retransmission / fiabilité
const unsigned int RESEND_INTERVAL_MS = 100;
const uint8_t MAX_RESEND_ATTEMPTS = 3;

// ------------------------------------------------------------------
RH_ASK driver;

// Anti-rebond
bool btnState[NUM_BUTTONS];
bool lastReading[NUM_BUTTONS];
unsigned long lastDebounce[NUM_BUTTONS];

// Machine d’état
enum ETAT {IDLE, PREPARE_MSG, SENDING};
ETAT etat = IDLE;

char message[MSG_BUF_SIZE];
unsigned long nextSendTime = 0;
uint8_t sendAttemptsLeft = 0;
uint16_t seqCounter = 0;

int boutonDetecte = -1;
bool actionDetectee = false;

// ------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  if (!driver.init()) {
    Serial.println("Erreur init RH_ASK");
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(boutonPins[i], INPUT_PULLUP);
    lastReading[i] = digitalRead(boutonPins[i]);
    btnState[i] = lastReading[i];
    lastDebounce[i] = 0;
  }

  Serial.println("Émetteur prêt !");
}

// ------------------------------------------------------------------
void loop() {
  unsigned long now = millis();

  // --------------------------------------------------------------
  // LECTURE BOUTONS + ANTI-REBOND
  // --------------------------------------------------------------
  for (int i = 0; i < NUM_BUTTONS; i++) {
    bool reading = digitalRead(boutonPins[i]);

    if (reading != lastReading[i]) {
      lastDebounce[i] = now;
      lastReading[i] = reading;
    }

    if ((now - lastDebounce[i]) > DEBOUNCE_DELAY) {
      if (reading != btnState[i]) {

        btnState[i] = reading;
        boutonDetecte = i;
        actionDetectee = true;
        etat = PREPARE_MSG;
      }
    }
  }

  // --------------------------------------------------------------
  // PRÉPARATION DU MESSAGE
  // --------------------------------------------------------------
  if (etat == PREPARE_MSG && actionDetectee && boutonDetecte >= 0) {

    const char* action = (btnState[boutonDetecte] == LOW) ? "ON" : "OFF";

    seqCounter++;

    snprintf(message, sizeof(message),
             "%s|B%d|%s|%u",
             EMETTEUR_ID,
             boutonDetecte + 1,
             action,
             (unsigned int)seqCounter);

    Serial.print("Préparation message: ");
    Serial.println(message);

    sendAttemptsLeft = MAX_RESEND_ATTEMPTS;
    nextSendTime = now;
    etat = SENDING;
    actionDetectee = false;
  }

  // --------------------------------------------------------------
  // ENVOI + RETRANSMISSIONS
  // --------------------------------------------------------------
  if (etat == SENDING) {

    if (now >= nextSendTime && sendAttemptsLeft > 0) {

      driver.send((uint8_t*)message, strlen(message));
      driver.waitPacketSent();

      Serial.print("Envoi (tentatives restantes ");
      Serial.print(sendAttemptsLeft - 1);
      Serial.print("): ");
      Serial.println(message);

      sendAttemptsLeft--;
      nextSendTime = now + RESEND_INTERVAL_MS;
    }

    if (sendAttemptsLeft == 0) {
      etat = IDLE;
      boutonDetecte = -1;
    }
  }
}
