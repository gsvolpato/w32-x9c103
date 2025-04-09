# Controlador Digital de Potenciômetro X9C103 para ESP32

## Branches

### main
Controle básico de um potenciômetro digital X9C103 usando botões físicos.

### activeBaxandall
Circuito ativo de controle de tom Baxandall usando dois potenciômetros digitais X9C103 para controles de graves e agudos, encoders rotativos, pedais, transistores e display OLED.

## Português (Brasil)

Este repositório contém o código-fonte para um controlador de potenciômetro digital X9C103 utilizando ESP32. O sistema permite ajustar a resistência do potenciômetro digitalmente através de botões físicos, com as seguintes funcionalidades:

### Funcionalidades (Branch main)
- Controle preciso da posição do potenciômetro (0-99 passos)
- Botões para aumentar e diminuir a resistência
- Operação contínua quando os botões são mantidos pressionados (similar a um controle remoto de TV)
- Armazenamento da última posição na memória SPIFFS do ESP32
- Recuperação automática da posição armazenada ao iniciar

### Funcionalidades Adicionais (Branch activeBaxandall)
- Controles de graves e agudos usando dois potenciômetros digitais X9C103
- Interface com encoders rotativos para ajuste preciso
- Interruptores de pé para ativar/desativar o efeito e boost
- Controle de três transistores para circuito de comutação
- LED indicador de status
- Reset dos controles para posição central ao pressionar o botão do encoder
- Display OLED SH1106 1.3" com interface gráfica intuitiva
- Visualização em tempo real das configurações de graves e agudos
- Indicadores de status de bypass e boost no display

### Componentes necessários (Branch main)
- ESP32
- Potenciômetro digital X9C103
- 3 botões para controle (aumentar, diminuir, salvar)
- Resistores pull-up (opcional, pois utiliza os resistores internos do ESP32)

### Componentes necessários (Branch activeBaxandall)
- ESP32
- 2 Potenciômetros digitais X9C103 (graves e agudos)
- 2 Encoders rotativos com botão integrado
- 2 Interruptores de pé (bypass e boost)
- 3 Transistores para comutação de sinal
- LED de status
- Display OLED SH1106 1.3" (128x64 pixels, I2C)
- Resistores e componentes adicionais para o circuito Baxandall

### Pinagem (Branch main)
- INC_PIN: 22 (Pino de incremento do X9C103)
- UD_PIN: 21 (Pino de direção Up/Down do X9C103)
- CS_PIN: 23 (Pino de seleção de chip do X9C103)
- UP_BTN: 27 (Botão para aumentar a resistência)
- DOWN_BTN: 25 (Botão para diminuir a resistência)
- STORE_BTN: 26 (Botão para salvar a posição atual)

### Pinagem (Branch activeBaxandall)
- Graves:
  - BASS_INC_PIN: 22
  - BASS_UD_PIN: 21
  - BASS_CS_PIN: 23
- Agudos:
  - TREBLE_INC_PIN: 19
  - TREBLE_UD_PIN: 18
  - TREBLE_CS_PIN: 5
- Encoders:
  - BASS_ENC_A: 32, BASS_ENC_B: 33, BASS_ENC_SW: 25
  - TREBLE_ENC_A: 26, TREBLE_ENC_B: 27, TREBLE_ENC_SW: 14
- Pedais:
  - BYPASS_FS_PIN: 12
  - BOOST_FS_PIN: 13
- Transistores:
  - BASS_TRANSISTOR_PIN: 4
  - TREBLE_TRANSISTOR_PIN: 2
  - BOOST_TRANSISTOR_PIN: 15
- LED:
  - STATUS_LED_PIN: 16
- Display OLED:
  - Usa comunicação I2C padrão (SDA e SCL do ESP32)
  - Endereço I2C: 0x3C

## English

This repository contains the source code for an X9C103 digital potentiometer controller using ESP32. The system allows adjusting the potentiometer resistance digitally through physical buttons, with the following features:

### Branches

### main
Basic control of an X9C103 digital potentiometer using physical buttons.

### activeBaxandall
Active Baxandall tone control circuit using two X9C103 digital potentiometers for bass and treble controls, rotary encoders, footswitches, transistors, and OLED display.

### Features (Main Branch)
- Precise control of potentiometer position (0-99 steps)
- Buttons to increase and decrease resistance
- Continuous operation when buttons are held down (similar to a TV remote control)
- Storage of the last position in the ESP32's SPIFFS memory
- Automatic recovery of stored position on startup

### Additional Features (activeBaxandall Branch)
- Bass and treble controls using two X9C103 digital potentiometers
- Interface with rotary encoders for precise adjustment
- Footswitches to enable/disable the effect and boost
- Control of three transistors for switching circuit
- Status LED indicator
- Reset controls to center position by pressing the encoder button
- SH1106 1.3" OLED display with intuitive graphical interface
- Real-time visualization of bass and treble settings
- Bypass and boost status indicators on the display

### Required Components (Main Branch)
- ESP32
- X9C103 digital potentiometer
- 3 buttons for control (increase, decrease, save)
- Pull-up resistors (optional, as it uses ESP32's internal resistors)

### Required Components (activeBaxandall Branch)
- ESP32
- 2 X9C103 digital potentiometers (bass and treble)
- 2 Rotary encoders with integrated button
- 2 Footswitches (bypass and boost)
- 3 Transistors for signal switching
- Status LED
- SH1106 1.3" OLED display (128x64 pixels, I2C)
- Additional resistors and components for the Baxandall circuit

### Pin Mapping (Main Branch)
- INC_PIN: 22 (X9C103 increment pin)
- UD_PIN: 21 (X9C103 Up/Down direction pin)
- CS_PIN: 23 (X9C103 chip select pin)
- UP_BTN: 27 (Button to increase resistance)
- DOWN_BTN: 25 (Button to decrease resistance)
- STORE_BTN: 26 (Button to save current position)

### Pin Mapping (activeBaxandall Branch)
- Bass:
  - BASS_INC_PIN: 22
  - BASS_UD_PIN: 21
  - BASS_CS_PIN: 23
- Treble:
  - TREBLE_INC_PIN: 19
  - TREBLE_UD_PIN: 18
  - TREBLE_CS_PIN: 5
- Encoders:
  - BASS_ENC_A: 32, BASS_ENC_B: 33, BASS_ENC_SW: 25
  - TREBLE_ENC_A: 26, TREBLE_ENC_B: 27, TREBLE_ENC_SW: 14
- Footswitches:
  - BYPASS_FS_PIN: 12
  - BOOST_FS_PIN: 13
- Transistors:
  - BASS_TRANSISTOR_PIN: 4
  - TREBLE_TRANSISTOR_PIN: 2
  - BOOST_TRANSISTOR_PIN: 15
- LED:
  - STATUS_LED_PIN: 16
- OLED Display:
  - Uses standard I2C communication (ESP32's SDA and SCL pins)
  - I2C Address: 0x3C 