# Projeto ESP8266 - Captive Portal com Página de Login Falsa
Este projeto utiliza o **ESP8266** para criar um portal cativo com uma página de login falsa, simulando um ponto de acesso WiFi público. Ele serve para fins educacionais, demonstrando como ataques de phishing em redes WiFi podem ser realizados e os riscos de confiar em WiFis abertos. A página de login coleta as credenciais inseridas e as armazena na EEPROM do ESP8266.

## Funcionalidades
- Criação de um ponto de acesso WiFi (SSID configurável).
- Captura de credenciais (login e senha) por meio de uma página de login falsa.
- Armazenamento das credenciais capturadas na EEPROM do ESP8266.
- Exibição das credenciais coletadas através de uma interface web.
- Download das credenciais capturadas para um arquivo local.
- Troca do SSID via interface web.
- Limpeza das credenciais armazenadas.

## Componentes Utilizados
- **ESP8266**
- **DNSServer** (para redirecionar todo o tráfego para o ESP8266)
- **EEPROM** (para armazenar as credenciais capturadas)
- **WebServer** (para servir as páginas de login, exibição de dados e download)

## Configurações Iniciais
- O nome do SSID padrão é configurado como `"Free WiFi"`, mas pode ser alterado através da interface web.
- O IP do ponto de acesso é configurado como `172.0.0.1`.
- O DNS redireciona todas as solicitações de páginas para o portal cativo.

## Imagens de como o projeto se comporta, estão na pasta src.

## Instalação
1. Clone este repositório em sua máquina.
   ```bash
   git clone https://github.com/celoezra/Captive-Portal-Google-Esp8266.git

