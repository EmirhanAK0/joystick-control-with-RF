
# 📡 RF Communication with PIC16F887 using NEC Protocol

This project demonstrates a basic RF communication setup using two PIC16F887 microcontrollers, based on the NEC protocol. The **transmitter** reads values from two potentiometers (via ADC) and sends encoded signals wirelessly. The **receiver** decodes these signals and displays visual feedback using an LED matrix driven by the MAX7219.

---

## 🧰 Hardware Requirements

- 2 × PIC16F887 microcontrollers  
- RF transmitter module (433 MHz or similar)  
- RF receiver module  
- 2 × Potentiometers (connected to AN0 and AN1 on transmitter)  
- MAX7219 and 8×8 LED dot matrix display (connected to receiver)  
- Breadboard, jumpers, power supply, etc.

---

## 🧠 Project Structure

- **`rf_transmitter.c`**  
  Sends a 32-bit NEC signal based on potentiometer readings from AN0 and AN1.

- **`rf_receiver.c`**  
  Receives NEC signals and shows different animations or patterns on a MAX7219-controlled LED matrix depending on signal contents.

---

## ⚙️ How It Works

### Transmitter
- Internal oscillator set to 8 MHz.
- ADC reads values from two potentiometers.
- Depending on ADC results, a predefined 32-bit NEC signal is sent via `PIN_B4`.

| Pot1 | Pot2 | Sent Code     |
|------|------|---------------|
| 0    | -    | `0x00FF00FF`  |
| 1    | -    | `0x00FF807F`  |
| 2    | -    | `0x00FF10EF`  |
| -    | 0    | `0x00FF40BF`  |
| -    | 1    | `0x00FF20DF`  |
| -    | 2    | `0x00FF08F7`  |

### Receiver
- Detects NEC protocol using external interrupts and Timer1.
- Extracts 32-bit command and updates LED matrix patterns accordingly using MAX7219.
- SPI communication with MAX7219 is bit-banged in software.

---

## 💡 Example LED Patterns

| pot1 | pot2 | Display Behavior     |
|------|------|----------------------|
| 0    | 0    | B → D animation      |
| 0    | 1    | B → C animation      |
| 1    | 1    | A → C animation      |
| 1    | 0    | D → A animation      |
| 2    | 1    | Show `C`             |
| 2    | 0    | Show `D`             |
| any  | 2    | Show single letter   |

---

## 🔧 Setup and Compilation

You can compile these `.c` files using the [CCS C Compiler](https://www.ccsinfo.com/). Make sure:
- The correct device is selected (`#include <16F887.h>`)
- Fuses match your configuration (oscillator, MCLR, etc.)
- Pins are correctly connected according to `PIN_B4`, `PIN_C3`, `PIN_C4`, and `PIN_C5`.

---

## 📁 Files

- [`rf_transmitter.c`](./rf_transmitter.c)
- [`rf_receiver.c`](./rf_receiver.c)
