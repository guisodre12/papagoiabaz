//Papagoiabaz robotica e resenha
//guisodre - 19 de marco
//papai do ceu abencoa esse codigo, esse robo e essa equipe

#include <Bluepad32.h>

// ===== MOTORES =====
#define A1 13
#define A2 14
#define B1 26
#define B2 25

// ===== LED =====
#define LED 2

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ===== MOVIMENTO =====
void parar() {
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
    digitalWrite(B1, LOW);
    digitalWrite(B2, LOW);
}

void frente() {
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
    digitalWrite(B1, HIGH);
    digitalWrite(B2, LOW);
}

void tras() {
    digitalWrite(A1, LOW);
    digitalWrite(A2, HIGH);
    digitalWrite(B1, LOW);
    digitalWrite(B2, HIGH);
}

void esquerda() {
    digitalWrite(A1, LOW);
    digitalWrite(A2, HIGH);
    digitalWrite(B1, HIGH);
    digitalWrite(B2, LOW);
}

void direita() {
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
    digitalWrite(B1, LOW);
    digitalWrite(B2, HIGH);
}

// ===== CALLBACKS =====
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("Controller conectado: %d\n", i);
            myControllers[i] = ctl;
            return;
        }
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("Controller desconectado: %d\n", i);
            myControllers[i] = nullptr;
            return;
        }
    }
}

// ===== CONTROLE DO CARRINHO =====
void processGamepad(ControllerPtr ctl) {
    int y = ctl->axisY();
    int x = ctl->axisX();

    if (abs(x) < 50) x = 0;
    if (abs(y) < 50) y = 0;

    if (y < -200) {
        frente();
    } 
    else if (y > 200) {
        tras();
    } 
    else if (x < -200) {
        esquerda();
    } 
    else if (x > 200) {
        direita();
    } 
    else {
        parar();
    }
}

// ===== LOOP DOS CONTROLES =====
bool hasControllerConnected() {
    for (auto ctl : myControllers) {
        if (ctl && ctl->isConnected()) return true;
    }
    return false;
}

bool hasInput() {
    for (auto ctl : myControllers) {
        if (ctl && ctl->isConnected() && ctl->hasData()) return true;
    }
    return false;
}

void processControllers() {
    for (auto ctl : myControllers) {
        if (ctl && ctl->isConnected() && ctl->hasData()) {
            if (ctl->isGamepad()) {
                processGamepad(ctl);
            }
        }
    }
}

// ===== SETUP =====
void setup() {
    Serial.begin(115200);

    pinMode(A1, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(B1, OUTPUT);
    pinMode(B2, OUTPUT);

    pinMode(LED, OUTPUT);

    parar();

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
}

// ===== LOOP =====
void loop() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;

    unsigned long now = millis();

    bool connected = hasControllerConnected();
    bool input = hasInput();

    // procurando controle pisca lento
    if (!connected) {
        if (now - lastBlink > 500) {
            ledState = !ledState;
            digitalWrite(LED, ledState);
            lastBlink = now;
        }
    }
    // conectado  pisca rapido
    else if (input) {
        if (now - lastBlink > 100) {
            ledState = !ledState;
            digitalWrite(LED, ledState);
            lastBlink = now;
        }
    }
    //conectado sem input  LED desligado
    else {
        digitalWrite(LED, LOW);
    }

    if (BP32.update()) {
        processControllers();
    }

    delay(10);
}
