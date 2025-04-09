# Controlador Digital de Potenciômetro X9C103 para ESP32

## Português (Brasil)

Este repositório contém o código-fonte para um controlador de potenciômetro digital X9C103 utilizando ESP32. O sistema permite ajustar a resistência do potenciômetro digitalmente através de botões físicos, com as seguintes funcionalidades:

### Funcionalidades
- Controle preciso da posição do potenciômetro (0-99 passos)
- Botões para aumentar e diminuir a resistência
- Operação contínua quando os botões são mantidos pressionados (similar a um controle remoto de TV)
- Armazenamento da última posição na memória SPIFFS do ESP32
- Recuperação automática da posição armazenada ao iniciar

### Componentes necessários
- ESP32
- Potenciômetro digital X9C103
- 3 botões para controle (aumentar, diminuir, salvar)
- Resistores pull-up (opcional, pois utiliza os resistores internos do ESP32)

### Pinagem
- INC_PIN: 22 (Pino de incremento do X9C103)
- UD_PIN: 21 (Pino de direção Up/Down do X9C103)
- CS_PIN: 23 (Pino de seleção de chip do X9C103)
- UP_BTN: 27 (Botão para aumentar a resistência)
- DOWN_BTN: 25 (Botão para diminuir a resistência)
- STORE_BTN: 26 (Botão para salvar a posição atual)

## English

This repository contains the source code for an X9C103 digital potentiometer controller using ESP32. The system allows adjusting the potentiometer resistance digitally through physical buttons, with the following features:

### Features
- Precise control of potentiometer position (0-99 steps)
- Buttons to increase and decrease resistance
- Continuous operation when buttons are held down (similar to a TV remote control)
- Storage of the last position in the ESP32's SPIFFS memory
- Automatic recovery of stored position on startup

### Required Components
- ESP32
- X9C103 digital potentiometer
- 3 buttons for control (increase, decrease, save)
- Pull-up resistors (optional, as it uses ESP32's internal resistors)

### Pin Mapping
- INC_PIN: 22 (X9C103 increment pin)
- UD_PIN: 21 (X9C103 Up/Down direction pin)
- CS_PIN: 23 (X9C103 chip select pin)
- UP_BTN: 27 (Button to increase resistance)
- DOWN_BTN: 25 (Button to decrease resistance)
- STORE_BTN: 26 (Button to save current position) 