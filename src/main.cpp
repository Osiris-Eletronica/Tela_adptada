#include <Arduino.h>
#include <TFT_eSPI.h> 

TFT_eSPI tft = TFT_eSPI();

#define BTN_LEFT_PIN 34
#define BTN_RIGHT_PIN 35

int menuSelection = 0; 
const int numOptions = 3;
const char* menuItems[] = {"Opcao A (Contador)", "Opcao B (Status)", "Opcao C (Info)"};

void drawMenu() {
    tft.fillScreen(TFT_BLACK); // Apaga TUDO e redesenha (causa leve flicker)
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Menu Principal");

    // Desenha os 3 itens do menu e destaca o selecionado
    for (int i = 0; i < numOptions; i++) {
        int yPos = 50 + (i * 30); // Posicao Y para cada item

        if (i == menuSelection) {
            // Se for o selecionado, desenha com fundo amarelo
            tft.setTextColor(TFT_BLACK, TFT_YELLOW); 
        } else {
            // Caso contrario, desenha com fundo preto
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.setCursor(50, yPos); 
        tft.println(menuItems[i]);
    }
}

void setup() {
    tft.init();
    tft.setRotation(1); // Paisagem
    
    pinMode(BTN_LEFT_PIN, INPUT_PULLDOWN); 
    pinMode(BTN_RIGHT_PIN, INPUT_PULLDOWN);

    drawMenu(); // Desenha a primeira versao do menu
}

void loop() {
    bool selectionChanged = false;

    if (digitalRead(BTN_RIGHT_PIN) == HIGH) { // Botao Direito (mover para baixo)
        menuSelection++;
        if (menuSelection >= numOptions) menuSelection = 0; 
        selectionChanged = true;
        delay(250); // Aumentei o debounce para evitar ruido
    }

    if (digitalRead(BTN_LEFT_PIN) == HIGH) { // Botao Esquerdo (mover para cima)
        menuSelection--;
        if (menuSelection < 0) menuSelection = numOptions - 1; 
        selectionChanged = true;
        delay(250); // Aumentei o debounce para evitar ruido
    }

    if (selectionChanged) {
        drawMenu(); // Redesenha o menu inteiro com a nova selecao destacada
    }
    
    delay(10);
}
