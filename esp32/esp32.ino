#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Firebase Functions
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// CONFIGURAÇÕES DE REDE E FIREBASE
const char* WIFI_SSID =  "";
const char* WIFI_PASSWORD = "";
#define FIREBASE_HOST ""
#define FIREBASE_API_KEY ""

// PINAGEM
#define DHTPIN 4          
#define DHTTYPE DHT22

#define PINO_LED_A 26     
#define PINO_LED_B 25
#define PINO_LED_C 14

#define PINO_BOTAO_A 32   
#define PINO_BOTAO_B 33
#define PINO_BOTAO_C 27

#define PINO_BUZZER 18    

// OBJETOS
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", -10800, 60000);
DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

// VARIÁVEIS GLOBAIS
String dispositivoID;
String userUID = "";
unsigned long ultimoStatusMillis = 0;
unsigned long ultimoFirebaseMillis = 0; 
unsigned long ultimoLcdMillis = 0; 
bool signupOK = false;

bool statusA = false;
bool statusB = false;
bool statusC = false;

bool buzzerAtivoA = false;
bool buzzerAtivoB = false;
bool buzzerAtivoC = false;

String ultimaHoraConfirmadaA = "";
String ultimaHoraConfirmadaB = "";
String ultimaHoraConfirmadaC = "";

// LCD Text
String textoProxRemedio = "Atualizando...  ";

// PROTÓTIPOS
void acionarCompartimento(String id);
void desligarCompartimento(String id);
void atualizarBuzzer();
String avaliarSinalWiFi(int rssi);
String getChipID();

String getChipID() {
  uint64_t chipid = ESP.getEfuseMac();
  char buf[13];
  sprintf(buf, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  return String(buf);
}

String avaliarSinalWiFi(int rssi) {
  if (rssi >= -50) return "Excelente"; 
  else if (rssi >= -60) return "Bom";       
  else if (rssi >= -70) return "Regular";   
  else return "Ruim";      
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Conectado");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado");
  delay(1000);

  dispositivoID = getChipID();
  Serial.print("ID DO DISPOSITIVO: ");
  Serial.println(dispositivoID);

  timeClient.begin();
  timeClient.update();

  firebaseConfig.database_url = FIREBASE_HOST;
  firebaseConfig.api_key = FIREBASE_API_KEY;

  if (Firebase.signUp(&firebaseConfig, &firebaseAuth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOK = true;
  } else {
    Serial.printf("Erro no SignUp: %s\n", firebaseConfig.signer.signupError.message.c_str());
  }

  firebaseConfig.token_status_callback = tokenStatusCallback;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  dht.begin();
  
  pinMode(PINO_LED_A, OUTPUT);
  pinMode(PINO_LED_B, OUTPUT);
  pinMode(PINO_LED_C, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT); 
  
  pinMode(PINO_BOTAO_A, INPUT_PULLUP);
  pinMode(PINO_BOTAO_B, INPUT_PULLUP);
  pinMode(PINO_BOTAO_C, INPUT_PULLUP);

  lcd.clear();
}

void loop() {
  timeClient.update();
  String horaAtual = timeClient.getFormattedTime().substring(0, 5);
  int diaAtual = timeClient.getDay();
  String nomesDias[] = {"Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado"};

  // LCD UPDATE
  if (millis() - ultimoLcdMillis > 1000) {
    ultimoLcdMillis = millis();
    lcd.setCursor(0, 0);
    lcd.print("Hora: " + timeClient.getFormattedTime() + "    ");

    lcd.setCursor(0, 1);
    if (statusA || statusB || statusC) {
      lcd.print("ALERTA: REMEDIO!");
    } else {
      lcd.print(textoProxRemedio);
    }
  }

  // BUTTON READ
  if (digitalRead(PINO_BOTAO_A) == LOW) {
    if (statusA) {
      Serial.println(">>> Botão A pressionado! Salvando histórico...");
      unsigned long timestampUTC = timeClient.getEpochTime() + 10800;
      String pathA = "/users/" + userUID + "/compartimentos/A/historico";
      if (Firebase.pushInt(firebaseData, pathA.c_str(), timestampUTC)) {
        Serial.println(">>> Confirmação A salva!");
      }
      ultimaHoraConfirmadaA = horaAtual; 
      desligarCompartimento("A");       
      buzzerAtivoA = false;             
      atualizarBuzzer();                
      delay(200); 
    }
  }

  if (digitalRead(PINO_BOTAO_B) == LOW) {
    if (statusB) {
      Serial.println(">>> Botão B pressionado! Salvando histórico...");
      unsigned long timestampUTC = timeClient.getEpochTime() + 10800;
      String pathB = "/users/" + userUID + "/compartimentos/B/historico";
      if (Firebase.pushInt(firebaseData, pathB.c_str(), timestampUTC)) {
        Serial.println(">>> Confirmação B salva!");
      }
      ultimaHoraConfirmadaB = horaAtual; 
      desligarCompartimento("B");       
      buzzerAtivoB = false;             
      atualizarBuzzer();                
      delay(200); 
    }
  }

  if (digitalRead(PINO_BOTAO_C) == LOW) {
    if (statusC) {
      Serial.println(">>> Botão C pressionado! Salvando histórico...");
      unsigned long timestampUTC = timeClient.getEpochTime() + 10800;
      String pathC = "/users/" + userUID + "/compartimentos/C/historico";
      if (Firebase.pushInt(firebaseData, pathC.c_str(), timestampUTC)) {
        Serial.println(">>> Confirmação C salva!");
      }
      ultimaHoraConfirmadaC = horaAtual; 
      desligarCompartimento("C");       
      buzzerAtivoC = false;             
      atualizarBuzzer();                
      delay(200); 
    }
  }

  // HEARTBEAT REDE (30 seg)
  if (millis() - ultimoStatusMillis > 30000) {
    ultimoStatusMillis = millis();
    String pathDispositivo = "/dispositivos/" + dispositivoID;
    unsigned long timestampUTC = timeClient.getEpochTime() + 10800;

    Firebase.setInt(firebaseData, (pathDispositivo + "/status").c_str(), timestampUTC);
    Firebase.setString(firebaseData, (pathDispositivo + "/wifi_ssid").c_str(), WiFi.SSID());
    Firebase.setInt(firebaseData, (pathDispositivo + "/wifi_rssi").c_str(), WiFi.RSSI());
    Firebase.setString(firebaseData, (pathDispositivo + "/wifi_qualidade").c_str(), avaliarSinalWiFi(WiFi.RSSI()));
    
    Serial.println("x----------------------------------x");
    Serial.println(">> Heartbeat e status de rede updated.");
    Serial.println("x----------------------------------x");
  }

  // VERIFICAÇÃO DOS ALARMES (5 seg)
  if (millis() - ultimoFirebaseMillis > 5000) {
    ultimoFirebaseMillis = millis();

    String pathVinculo = "/vinculos/" + dispositivoID;
   
    if (Firebase.getString(firebaseData, pathVinculo.c_str())) {
      String resultado = firebaseData.stringData();
      resultado.trim();
      resultado.replace("\"", "");

      if (resultado == "null" || resultado == "" || resultado.length() == 0) {
        if (userUID != "") { desligarCompartimento("A"); desligarCompartimento("B"); desligarCompartimento("C"); }
        userUID = ""; buzzerAtivoA = false; buzzerAtivoB = false; buzzerAtivoC = false;
        atualizarBuzzer(); 
        textoProxRemedio = "Aguard. Vinculo ";
        Serial.println("Aparelho sem vínculo ativo.");
        return;
      } else {
        userUID = resultado;
      }
    } else {
      if (userUID != "") { desligarCompartimento("A"); desligarCompartimento("B"); desligarCompartimento("C"); }
      userUID = ""; buzzerAtivoA = false; buzzerAtivoB = false; buzzerAtivoC = false;
      atualizarBuzzer(); 
      textoProxRemedio = "Erro de Vinculo ";
      Serial.println("Erro ao ler vínculo.");
      return;
    }

    Serial.println("------------------------------------");
    Serial.printf("DISPOSITIVO: %s | %s às %s\n", dispositivoID.c_str(), nomesDias[diaAtual].c_str(), horaAtual.c_str());
    Serial.println("");

    String compartimentos[] = {"A", "B", "C"};
    
    int menorTempoMinutos = 9999;
    int minutosAtuais = horaAtual.substring(0, 2).toInt() * 60 + horaAtual.substring(3, 5).toInt();

    for (int i = 0; i < 3; i++) {
      String compID = compartimentos[i];
      String path = "/users/" + userUID + "/compartimentos/" + compID;
      String pathDias = path + "/dias/" + String(diaAtual);
      String pathHora = path + "/horario";
      String pathAtivo = path + "/ativo"; 

      bool tocaHoje = false;
      String horarioProg = "";
      bool alarmeAtivo = true; 

      if (Firebase.getBool(firebaseData, pathAtivo.c_str(), &alarmeAtivo)) {
        if (!alarmeAtivo) {
          desligarCompartimento(compID);
          if (compID == "A") buzzerAtivoA = false;
          else if (compID == "B") buzzerAtivoB = false;
          else if (compID == "C") buzzerAtivoC = false;
          Serial.print("Comp "); Serial.print(compID); Serial.println(": DESATIVADO no app.");
          continue; 
        }
      }

      if (Firebase.getBool(firebaseData, pathDias.c_str(), &tocaHoje)) {
        if (Firebase.getString(firebaseData, pathHora.c_str(), &horarioProg)) {
          
          Serial.print("Comp ");
          Serial.print(compID);
          Serial.print(": [Prog: ");
          Serial.print(horarioProg);
          Serial.print(" | Hoje: ");
          Serial.print(tocaHoje ? "SIM" : "NAO");
          Serial.print(" | LED: ");
          Serial.println((compID == "A" ? statusA : (compID == "B" ? statusB : statusC)) ? "LIGADO" : "DESLIGADO");

          // CÁLCULO DE TEMPO (LCD)
          if (tocaHoje && alarmeAtivo && horarioProg.length() >= 5) {
             int minutosProg = horarioProg.substring(0, 2).toInt() * 60 + horarioProg.substring(3, 5).toInt();
             
             // SE NÃO TOCOU
             if (minutosProg > minutosAtuais) {
                int diferenca = minutosProg - minutosAtuais;
                if (diferenca < menorTempoMinutos) {
                   menorTempoMinutos = diferenca;
                }
             }
          }

          // LOGICA DE DISPARO
          if (tocaHoje && horaAtual == horarioProg) {
            if (compID == "A") {
              if (ultimaHoraConfirmadaA != horaAtual) { acionarCompartimento(compID); buzzerAtivoA = true; } 
              else { buzzerAtivoA = false; }
            } 
            else if (compID == "B") {
              if (ultimaHoraConfirmadaB != horaAtual) { acionarCompartimento(compID); buzzerAtivoB = true; } 
              else { buzzerAtivoB = false; }
            } 
            else if (compID == "C") {
              if (ultimaHoraConfirmadaC != horaAtual) { acionarCompartimento(compID); buzzerAtivoC = true; } 
              else { buzzerAtivoC = false; }
            }
          } else {
            if (compID == "A") { ultimaHoraConfirmadaA = ""; buzzerAtivoA = false; } 
            else if (compID == "B") { ultimaHoraConfirmadaB = ""; buzzerAtivoB = false; } 
            else if (compID == "C") { ultimaHoraConfirmadaC = ""; buzzerAtivoC = false; }
          }
        }
      }
    }
    
    // LCD TEXT
    if (menorTempoMinutos == 9999) {
       textoProxRemedio = "Sem mais p/ hoje";
    } else {
       int h = menorTempoMinutos / 60;
       int m = menorTempoMinutos % 60;
       if (h > 0) {
          textoProxRemedio = "Prox: " + String(h) + "h " + String(m) + "m";
       } else {
          textoProxRemedio = "Prox: " + String(m) + " min";
       }
    }
    
    // GAP FILL
    while (textoProxRemedio.length() < 16) {
       textoProxRemedio += " ";
    }

    Serial.println(""); 
    atualizarBuzzer(); 
  }
}

void acionarCompartimento(String id) {
  if (id == "A") { digitalWrite(PINO_LED_A, HIGH); statusA = true; }
  if (id == "B") { digitalWrite(PINO_LED_B, HIGH); statusB = true; }
  if (id == "C") { digitalWrite(PINO_LED_C, HIGH); statusC = true; }
}

void desligarCompartimento(String id) {
  if (id == "A") { digitalWrite(PINO_LED_A, LOW); statusA = false; }
  if (id == "B") { digitalWrite(PINO_LED_B, LOW); statusB = false; }
  if (id == "C") { digitalWrite(PINO_LED_C, LOW); statusC = false; }
}

void atualizarBuzzer() {
  if (buzzerAtivoA || buzzerAtivoB || buzzerAtivoC) {
    tone(PINO_BUZZER, 1500); 
  } else {
    noTone(PINO_BUZZER);
  }
}
