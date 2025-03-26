// Variables globales
double Temperatura = 0;
double LED_Movimiento = 0;
double Nivel_Luz = 0;
double LED_Cocina = 0;
double Ultras_Cm = 0;
double All_LED_s = 0;
String s_comandament;

// Configuración de sensores y actuadores
IRrecv ir_rx(A5);
decode_results ir_rx_results;
unsigned long ir_rx_results_value = 0;
String ir_rx_results_protocol = "";
int ir_rx_results_bits = 0;
Adafruit_NeoPixel neo_pixel(64, 2, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 oled_1(128, 64, &Wire, -1);
bool oled_1_autoshow = true;
AF_DCMotor motorshield_dc_1(1);
AF_DCMotor motorshield_dc_2(2);
AF_DCMotor motorshield_dc_3(3);
AF_DCMotor motorshield_dc_4(4);
String rtt_melody_14 = ":d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p";

// Función para registrar la temperatura
void LOG_Temp() {
    Serial.println("[SYSTEM] Temperatura ambient: " + String(Temperatura));
}

// Función para controlar la cocina
void Cocinar() {
    if (Temperatura >= 50 && Temperatura <= 64) {
        LED_Cocina = 1;
    } else if (Temperatura >= 65 && Temperatura <= 79) {
        LED_Cocina = 2;
        Movimiento_NO();
    } else if (Temperatura >= 80) {
        LED_Cocina = 3;
        delay(3000);
        Revisar_Temperatura();
    }
}

// Función para gestionar los LEDs
void Corona_LED() {
    if (LED_Movimiento == 1) {
        neo_pixel.setBrightness(255);
        for (int i = 0; i <= 29; i++) {
            neo_pixel.setPixelColor(All_LED_s, neo_pixel.Color(255, 255, 255));
            neo_pixel.show();
        }
    } else {
        neo_pixel.clear();
    }
}

// Función para registrar el nivel de luz
void LOG_Luz() {
    Serial.println("[SYSTEM] Nivell de llum: " + String(Nivel_Luz));
}

// Decodificador de señales IR
String fnc_ir_rx_decode_txt() {
    char buff[16] = {0};
    if (ir_rx.decode(&ir_rx_results)) {
        sprintf(buff, "%08lX", (unsigned long)ir_rx_results.value);
        ir_rx_results_value = (unsigned long)ir_rx_results.value;
        ir_rx_results_bits = ir_rx_results.bits;
        switch (ir_rx_results.decode_type) {
            case RC5: ir_rx_results_protocol = "RC5"; break;
            case RC6: ir_rx_results_protocol = "RC6"; break;
            case NEC: ir_rx_results_protocol = "NEC"; break;
            case SONY: ir_rx_results_protocol = "SONY"; break;
            case PANASONIC: ir_rx_results_protocol = "PANASONIC"; break;
            case JVC: ir_rx_results_protocol = "JVC"; break;
            case SAMSUNG: ir_rx_results_protocol = "SAMSUNG"; break;
            case WHYNTER: ir_rx_results_protocol = "WHYNTER"; break;
            case AIWA_RC_T501: ir_rx_results_protocol = "AIWA"; break;
            case LG: ir_rx_results_protocol = "LG"; break;
            case SANYO: ir_rx_results_protocol = "SANYO"; break;
            case MITSUBISHI: ir_rx_results_protocol = "MITSUBISHI"; break;
            case DENON: ir_rx_results_protocol = "DENON"; break;
        }
        ir_rx.resume();
    }
    return String(buff);
}

// Inicialización del sistema
void Inicializacion() {
    Temperatura = 0;
    Nivel_Luz = 0;
    LED_Cocina = 0;
    LED_Movimiento = 0;
    All_LED_s = 0;
    neo_pixel.begin();
    neo_pixel.clear();
    oled_1.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled_1.clearDisplay();
    if (oled_1_autoshow) oled_1.display();
}

// Revisión de la temperatura
void Revisar_Temperatura() {
    Temperatura = fnc_ntc(analogRead(A1));
    if (Temperatura < 50) {
        Movimiento();
    } else {
        Movimiento_NO();
        delay(1000);
        Cocinar();
    }
}

// Cálculo de temperatura a partir del NTC
double fnc_ntc(int _rawval) {
    double Vs = 5.0;
    double R1 = 10000.0;
    double Beta = 3950.0;
    double To = 298.15;
    double Ro = 10000.0;
    double Vout = (_rawval * Vs) / 1023.0;
    double Rt = R1 * Vout / (Vs - Vout);
    double T = 1 / (1 / To + log(Rt / Ro) / Beta);
    return T - 273.15;
}

// Función de parada total
void STOP_ALL() {
    Movimiento_NO();
    Inicializacion();
}

// Función para detener el movimiento
void Movimiento_NO() {
    LED_Movimiento = 0;
    motorshield_dc_1.run(RELEASE);
    motorshield_dc_2.run(RELEASE);
    motorshield_dc_3.run(RELEASE);
    motorshield_dc_4.run(RELEASE);
}

void setup() {
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(A0, INPUT);
    pinMode(A2, OUTPUT);
    pinMode(A1, INPUT);
    Serial.begin(9600);
    Serial.flush();
    while (Serial.available() > 0) Serial.read();
    ir_rx.enableIRIn();
    Inicializacion();
}

void loop() {
    s_comandament = fnc_ir_rx_decode_txt();
    if (s_comandament == "00FF6897") Inicializacion();
    else if (s_comandament == "00FF9867") Revisar_Nivel_de_Luz();
    else if (s_comandament == "00FFB04F") Revisar_Temperatura();
    else if (s_comandament == "00FF30CF") Cocinar();
    else if (s_comandament == "00FF18E7") Movimiento();
    else if (s_comandament == "00FF7A85") Movimiento_NO();
    else if (s_comandament == "00FF4AB5") STOP_ALL();
}