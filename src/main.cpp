
/*******************************************************************************
 * PROJETO: Gerador de Funções 
 * 
 * FUNÇÃO: SENOIDAL, QUADRADA, TRIANGULO, DENTE DE SERRA
 * 
 * DESENVOLVEDOR: Osiris Silva
 * 
 * CARGO: Técnico em Eletrônica
 * 
 * DATA: 10/01/2026
 * 
 * HARDWARE: Tela TFT - ST7796 - Resolução: 480x320
 * - Modelo: ESP WROOM 32 DEV KIT
 * - Não conjugados
 *
 * NOTAS TÉCNICAS:
 *
 *******************************************************************************/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <driver/dac.h>

// Pinos de Controle
#define BTN_MODE_PIN 22             
#define BTN_INC_PIN  13 
#define BTN_DEC_PIN  12 
#define DAC_CHAN     DAC_CHANNEL_1 

TFT_eSPI tft = TFT_eSPI();

// --- VARIÁVEIS GLOBAIS ---
volatile int modoOnda = 0;       
volatile float frequencia = 50.0;
volatile float dutyCycle = 50.0;
float fase = 0;
String nomes[] = {"SENOIDAL", "QUADRADA", "TRIANGULO", "DENTE SERRA"};
uint16_t cores[] = {TFT_CYAN, TFT_GREEN, TFT_YELLOW, TFT_MAGENTA};

// Controle de Estados (Somente 0=Normal, sem menu complexo)
int appState = 0; 

// Parâmetros de ajuste (Passo fixo em 10Hz, por enquanto)
float currentFreqStep = 10.0;
const float FREQ_MIN = 5.0;
const float FREQ_MAX = 20000.0; 

// Controle de Debounce
unsigned long ultimoDebounceBtnMode = 0;
unsigned long ultimoDebounceBtnInc = 0;
unsigned long ultimoDebounceBtnDec = 0;

// Variáveis para o Gráfico
const int GRAPH_HEIGHT = 120; 
const int GRAPH_Y_START = 70; 
int oldY = 0; int xPos = 0;


// --- FUNÇÕES DE DESENHO DA UI ---

void desenharTelaPrincipal() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("GERADOR ESTAVEL", 240, 10, 2); 
    tft.drawFastHLine(0, GRAPH_Y_START - 2, 480, TFT_DARKGREY); 
    tft.drawFastHLine(0, GRAPH_Y_START + GRAPH_HEIGHT + 2, 480, TFT_DARKGREY); 
    xPos = 0; oldY = 0;
}

void atualizarDadosNaTela() {
    // Texto do modo (em cima, Y=35)
    tft.setTextPadding(400); 
    tft.setTextColor(cores[modoOnda], TFT_BLACK);
    tft.drawCentreString(nomes[modoOnda], 240, 35, 4);
    
    // Texto da frequência (Y=210, fonte 6 grande, abaixo do gráfico)
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    // Usando 1 casa decimal conforme seu ajuste anterior
    tft.drawCentreString(String(frequencia, 1) + " Hz   ", 240, 210, 6); 
    
    // Exibe o Duty Cycle somente se for onda quadrada (Modo 1)
    if (modoOnda == 1) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawCentreString("DC: " + String((int)dutyCycle) + "%", 240, 275, 2); 
    } else {
        tft.fillRect(150, 270, 180, 25, TFT_BLACK); // Limpa o espaço se não for quadrada
    }
}


// --- SETUP E LOOP PRINCIPAL ---
void setup() {
    Serial.begin(115200);
    dac_output_enable(DAC_CHAN);
    pinMode(BTN_MODE_PIN, INPUT_PULLUP);
    pinMode(BTN_INC_PIN, INPUT_PULLUP);
    pinMode(BTN_DEC_PIN, INPUT_PULLUP);
    tft.init();
    tft.setRotation(1);
    desenharTelaPrincipal(); 
    atualizarDadosNaTela();
}

void loop() {
    // --- 1. GERAÇÃO DA ONDA & PLOTAGEM DO GRAFICO (Roda sempre) ---
    float incremento = (frequencia * TWO_PI) / 10000.0; uint8_t out = 0;
    if (modoOnda == 0) out = (uint8_t)((sin(fase) + 1) * 127);
    else if (modoOnda == 1) { float transitionPoint = TWO_PI * (dutyCycle / 100.0); out = (fase < transitionPoint) ? 255 : 0; }
    else if (modoOnda == 2) out = (fase < TWO_PI / 2.0) ? (uint8_t)((fase/(TWO_PI/2.0))*255) : (uint8_t)(255-((fase-(TWO_PI/2.0))/(TWO_PI/2.0))*255);
    else if (modoOnda == 3) out = (uint8_t)((fase / TWO_PI) * 255);
    dac_output_voltage(DAC_CHAN, out);
    fase += incremento; if (fase >= TWO_PI) fase = 0;

    // --- 2. PLOTAGEM DO GRÁFICO ---
    int newY = map(out, 0, 255, GRAPH_Y_START + GRAPH_HEIGHT, GRAPH_Y_START);
    if (xPos > 0) { tft.drawLine(xPos - 1, oldY, xPos, newY, cores[modoOnda]); }
    oldY = newY; xPos++;
    if (xPos >= 480) { xPos = 0; tft.fillRect(0, GRAPH_Y_START, 480, GRAPH_HEIGHT + 2, TFT_BLACK); }

    // --- 3. LEITURA DE BOTÕES (SIMPLES E ESTÁVEL) ---
    static unsigned long timerUI = 0;
    if (millis() - timerUI > 50) {
        timerUI = millis(); bool atualizaTela = false; unsigned long agora = millis();

        // Botão MODO (Alterna modo de onda - clique rápido e simples)
        if (digitalRead(BTN_MODE_PIN) == LOW && (agora - ultimoDebounceBtnMode > 300)) {
            modoOnda = (modoOnda + 1) % 4; fase = 0; ultimoDebounceBtnMode = agora; atualizaTela = true;
        }

        // Botão Incrementar Frequência / Duty Cycle
        if (digitalRead(BTN_INC_PIN) == LOW && (agora - ultimoDebounceBtnInc > 150)) {
            if(modoOnda == 1) { if(dutyCycle < 100) dutyCycle += 1; } 
            else { frequencia += currentFreqStep; if (frequencia > FREQ_MAX) frequencia = FREQ_MAX; }
            ultimoDebounceBtnInc = agora; atualizaTela = true;
        }
        
        // Botão Decrementar Frequência / Duty Cycle
        if (digitalRead(BTN_DEC_PIN) == LOW && (agora - ultimoDebounceBtnDec > 150)) {
            if(modoOnda == 1) { if(dutyCycle > 0) dutyCycle -= 1; } 
            else { frequencia -= currentFreqStep; if (frequencia < FREQ_MIN) frequencia = FREQ_MIN; }
            ultimoDebounceBtnDec = agora; atualizaTela = true;
        }

        if (atualizaTela) { atualizarDadosNaTela(); }
    }
    yield(); 
}
