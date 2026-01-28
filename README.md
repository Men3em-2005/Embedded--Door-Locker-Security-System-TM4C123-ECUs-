# Smart Door Locker Security System
 **Project Drive:** [Link](https://drive.google.com/drive/folders/1SGSzhNUnkoNBd79eBtCa1bOMNxV04Atm?usp=sharing)

---

## Overview
This project implements a **secure smart door locking system** using two TM4C123 (Tiva-C) microcontroller-based ECUs:

1. **HMI_ECU (Human-Machine Interface):**
   - LCD menu interface  
   - Keypad input for passwords  
   - Potentiometer input for auto-lock timeout  
   - UART communication with Control_ECU  

2. **Control_ECU (Control Unit):**
   - Password verification and EEPROM storage  
   - Motor control for door locking/unlocking  
   - Buzzer control for alarm and lockout  
   - Auto-lock timeout configuration  

---

## Features
- Initial password setup (5-digit)  
- Open door after password verification  
- Change password with old password verification  
- Set auto-lock timeout (5–30 seconds) via potentiometer  
- Lockout for 10 seconds after 3 failed attempts  
- EEPROM erase with password confirmation  
- UART5-based inter-ECU communication  

---

## Software Architecture
- **MCAL (Microcontroller Abstraction Layer):** GPIO, UART, ADC, Timers, EEPROM  
- **HAL (Hardware Abstraction Layer):** LCD, Keypad, RGB LED, Motor, Buzzer  
- **Application Layer:** Password setup, menu navigation, door control, lockout handling  

**Standards & Best Practices:**
- MISRA-C & CERT-C guidelines  
- Static functions for encapsulation  
- Named constants instead of magic numbers  
- NULL pointer checks  
- Safe `snprintf` for buffers  

---

## Hardware
- **Microcontroller:** TM4C123GH6PM (Tiva-C)  
- **Peripherals:** LCD, Keypad, DC Motor, Buzzer, Potentiometer  
- **Communication:** UART5 between HMI_ECU & Control_ECU  

---

## Resources
| ECU         | Flash Used | Stack Used | Notes |
|-------------|-----------|-----------|-------|
| HMI_ECU     | 13.25 KB  | 356 B     | UI & menu logic |
| Control_ECU | 4.52 KB   | 136 B     | Motor, EEPROM, buzzer |

---

## Usage
1. Power the system and HMI_ECU will check for existing password.  
2. If no password exists, setup a new 5-digit password.  
3. Navigate the menu on LCD:  
   - `A`: Open Door  
   - `B`: Change Password  
   - `C`: Set Auto-Lock Timeout  
   - `D`: Erase EEPROM  
4. Follow prompts and enter password when required.  

---

## Notes
- Maximum password attempts: 3  
- Lockout duration: 10 seconds  
- Auto-lock timeout: adjustable 5–30 seconds  
- Communication timeout: 5 seconds  

