/******************************************************************************
 * File: main.c
 * Module: HMI_ECU (Human Machine Interface Electronic Control Unit)
 * Description: Smart Door Lock System - HMI Application
 * 
 * System Features:
 *   - Initial password setup (5-digit)
 *   - Main menu: Open Door / Change Password / Set Auto-Lock Timeout
 *   - Password verification with 3 attempts
 *   - Potentiometer-based timeout adjustment (5-30 seconds)
 *   - Communication with Control ECU via UART5
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "lcd.h"
#include "keypad.h"
#include "potentiometer.h"
#include "uart.h"
#include "systick.h"

/******************************************************************************
 *                              Definitions                                    *
 ******************************************************************************/

/* UART Communication Commands */
#define CMD_SETUP_PASSWORD      0x01
#define CMD_VERIFY_PASSWORD     0x02
#define CMD_CHANGE_PASSWORD     0x03
#define CMD_SET_TIMEOUT         0x04
#define CMD_OPEN_DOOR           0x05
#define CMD_ERASE_EEPROM        0x06
#define CMD_CHECK_PASSWORD      0x07
#define CMD_TRIGGER_LOCKOUT     0x08

/* UART Response Codes */
#define RESP_PASSWORD_MATCH     0x10
#define RESP_PASSWORD_MISMATCH  0x11
#define RESP_TIMEOUT_SAVED      0x12
#define RESP_DOOR_UNLOCKING     0x13
#define RESP_DOOR_LOCKING       0x14
#define RESP_DOOR_LOCKED        0x15
#define RESP_SYSTEM_LOCKED      0x16
#define RESP_EEPROM_ERASED      0x17
#define RESP_PASSWORD_EXISTS    0x18
#define RESP_NO_PASSWORD        0x19
#define RESP_COUNTDOWN_START    0x1A

/* Password Configuration */
#define PASSWORD_LENGTH         5
#define MAX_ATTEMPTS            3

/* Menu Keys */
#define KEY_OPEN_DOOR           'A'
#define KEY_CHANGE_PASSWORD     'B'
#define KEY_SET_TIMEOUT         'C'
#define KEY_ERASE_EEPROM        'D'
#define KEY_SAVE                '*'

#define UART_RESPONSE_TIMEOUT_MS   (5000U)
#define LOCKOUT_DURATION_MS        (10000U)
#define TIMEOUT_MIN_SECONDS        (5U)
#define TIMEOUT_MAX_SECONDS        (30U)
#define RESP_TIMEOUT   (0xFFU)
/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

static void GetPassword(char *password);
void SendPassword(const char *password);
bool SetupPassword(void);
void DisplayMainMenu(void);
void HandleOpenDoor(void);
void HandleChangePassword(void);
void HandleSetTimeout(void);
void HandleEraseEEPROM(void);
bool CheckPasswordExists(void);
static uint8_t WaitForResponse(void);

/******************************************************************************
 *                          Main Application                                   *
 ******************************************************************************/

int main(void)
{
    char key;
    bool passwordSet = false;
    
    /* Initialize all peripherals */
    SysTick_Init(16000, SYSTICK_NOINT); /* 1ms tick */
    UART5_Init();
    Keypad_Init();
    POT_Init();
    LCD_Init();
    
    /* Display welcome message */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Smart Door Lock");
    LCD_SetCursor(1, 0);
    LCD_WriteString("System Ready");
    DelayMs(2000);
    
    /* Check if password already exists in EEPROM */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Checking...");
    DelayMs(500);
    
    passwordSet = CheckPasswordExists();
    
    /* Step 1: Initial Password Setup (only if no password exists) */
    if (!passwordSet) {
        while (!passwordSet) {
            passwordSet = SetupPassword();
        }
    } else {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Password Found");
        DelayMs(1500);
    }
    
    /* Main loop */
    while (1) {
        /* Step 2: Display Main Menu */
        DisplayMainMenu();
        
        /* Wait for user input */
        key = 0;
        while (key == 0) {
            key = Keypad_GetKey();
            DelayMs(10); /* Debounce delay */
        }
        
        /* Handle menu selection */
        switch (key) {
            case KEY_OPEN_DOOR:
                HandleOpenDoor();
                break;
                
            case KEY_CHANGE_PASSWORD:
                HandleChangePassword();
                break;
                
            case KEY_SET_TIMEOUT:
                HandleSetTimeout();
                break;
                
            case KEY_ERASE_EEPROM:
                HandleEraseEEPROM();
                break;
                
            default:
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Invalid Choice");
                DelayMs(1000);
                break;
        }
    }
}

/******************************************************************************
 *                          Function Implementations                           *
 ******************************************************************************/

/*
 * GetPassword
 * Prompts user to enter a 5-digit password via keypad
 * Displays asterisks (*) for each digit entered
 */
static void GetPassword(char *password)
{ 
  if (password == NULL) {
        return; /* or handle error */
    } uint8_t i = 0;
    char key;
    while (i < PASSWORD_LENGTH) {
        key = Keypad_GetKey();
        if (key >= '0' && key <= '9') {
            password[i] = key;
            LCD_WriteChar('*');
            i++;
            DelayMs(200);
        }
        DelayMs(10);
    }  password[PASSWORD_LENGTH] = '\0';
}

/*
 * SendPassword
 * Sends a 5-digit password to Control ECU via UART
 */
void SendPassword(const char *password)
{
    uint8_t i;
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        UART5_SendChar(password[i]);
        DelayMs(10);
    }
}

/*
 * SetupPassword
 * Step 1: Initial password setup
 * User enters password twice for confirmation
 * Returns true if passwords match, false otherwise
 */
bool SetupPassword(void)
{
    char password1[PASSWORD_LENGTH + 1];
    char password2[PASSWORD_LENGTH + 1];
    uint8_t response;
    
    /* Get first password */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Enter Password:");
    LCD_SetCursor(1, 0);
    GetPassword(password1);
    
    DelayMs(500);
    
    /* Get confirmation password */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Confirm Pass:");
    LCD_SetCursor(1, 0);
    GetPassword(password2);
    
    /* Send setup command to Control ECU */
    UART5_SendChar(CMD_SETUP_PASSWORD);
    DelayMs(50);
    SendPassword(password1);
    DelayMs(50);
    SendPassword(password2);
    
    /* Wait for response */
    response = WaitForResponse();
    
    if (response == RESP_PASSWORD_MATCH) {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Password Saved!");
        DelayMs(2000);
        return true;
    } else {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Passwords Don't");
        LCD_SetCursor(1, 0);
        LCD_WriteString("Match! Try Again");
        DelayMs(2000);
        return false;
    }
}

/*
 * DisplayMainMenu
 * Step 2: Shows main menu options on LCD
 */
void DisplayMainMenu(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("A:Open B:Pass");
    LCD_SetCursor(1, 0);
    LCD_WriteString("C:Time D:Erase");
}

/*
 * HandleOpenDoor
 * Step 3: Open door sequence
 * Prompts for password and sends command to Control ECU
 */
void HandleOpenDoor(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t attempts = 0;
    uint8_t response;
    uint8_t countdown;
    char buffer[16];
    
    while (attempts < MAX_ATTEMPTS) {
        /* Prompt for password */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Enter Password:");
        LCD_SetCursor(1, 0);
        GetPassword(password);
        
        /* Send open door command */
        UART5_SendChar(CMD_OPEN_DOOR);
        DelayMs(50);
        SendPassword(password);
        
        /* Wait for response */
        response = WaitForResponse();
        
        if (response == RESP_PASSWORD_MATCH) {
            /* Correct password - wait for door operation */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_WriteString("Access Granted");
            DelayMs(1500);
            
            /* Wait for unlocking message */
            response = WaitForResponse();
            if (response == RESP_DOOR_UNLOCKING) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Door Unlocking..");
                DelayMs(2000);
            }
            
            /* Wait for countdown start signal */
            response = WaitForResponse();
            if (response == RESP_COUNTDOWN_START) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Door Open");
                
                /* Receive countdown values */
                bool countingDown = true;
                while (countingDown) {
                    if (UART5_IsDataAvailable()) {
                        countdown = UART5_ReceiveChar();
                        
                        if (countdown == RESP_DOOR_LOCKING) {
                            /* Countdown finished, door is locking */
                            countingDown = false;
                            response = RESP_DOOR_LOCKING;
                        } else if (countdown <= 30) {
                            /* Valid countdown value */
                            LCD_SetCursor(1, 0);
                            snprintf(buffer, sizeof(buffer), "Closing in:%2u s", (unsigned)countdown);
                            LCD_WriteString(buffer);
                        }
                    }
                    DelayMs(10);
                }
            }
            
            /* Display locking message */
            if (response == RESP_DOOR_LOCKING) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Door Locking...");
                DelayMs(2000);
            }
            
            /* Wait for locked message */
            response = WaitForResponse();
            if (response == RESP_DOOR_LOCKED) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Door Locked");
                DelayMs(1500);
            }
            
            return; /* Exit function */
            
        } else {
            /* Incorrect password */
            attempts++;
            
            if (attempts < MAX_ATTEMPTS) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Wrong Password!");
                LCD_SetCursor(1, 0);
                sprintf(buffer, "Attempt %d/%d", attempts, MAX_ATTEMPTS);
                LCD_WriteString(buffer);
                DelayMs(1500);
            }
        }
    }
    
    /* Max attempts reached - trigger lockout */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("System Locked!");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Please Wait...");
    
    /* Send lockout trigger to Control ECU to sound buzzer */
    UART5_SendChar(CMD_TRIGGER_LOCKOUT);
    
    /* Wait 10 seconds - lockout duration */
    DelayMs(LOCKOUT_DURATION_MS);
}

/*
 * HandleChangePassword
 * Step 4: Change password sequence
 * Verifies old password before allowing new password setup
 */
void HandleChangePassword(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t attempts = 0;
    uint8_t response;
    bool success;
    char buffer[16];
    
    while (attempts < MAX_ATTEMPTS) {
        /* Prompt for old password */
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Enter Old Pass:");
        LCD_SetCursor(1, 0);
        GetPassword(password);
        
        /* Send change password command */
        UART5_SendChar(CMD_CHANGE_PASSWORD);
        DelayMs(50);
        SendPassword(password);
        
        /* Wait for response */
        response = WaitForResponse();
        
        if (response == RESP_PASSWORD_MATCH) {
            /* Correct password - setup new password */
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_WriteString("Password Correct");
            DelayMs(1000);
            
            /* Repeat initial password setup until successful */
            success = false;
            while (!success) {
                success = SetupPassword();
            }
            
            return; /* Exit function */
            
        } else {
            /* Incorrect password */
            attempts++;
            
            if (attempts < MAX_ATTEMPTS) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_WriteString("Wrong Password!");
                LCD_SetCursor(1, 0);
                sprintf(buffer, "Attempt %d/%d", attempts, MAX_ATTEMPTS);
                LCD_WriteString(buffer);
                DelayMs(1500);
            }
        }
    }
    
    /* Max attempts reached - trigger lockout */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("System Locked!");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Please Wait...");
    
    /* Send lockout trigger to Control ECU to sound buzzer */
    UART5_SendChar(CMD_TRIGGER_LOCKOUT);
    
    /* Wait 10 seconds - lockout duration */
    DelayMs(LOCKOUT_DURATION_MS);
}

/*
 * HandleSetTimeout
 * Step 5: Set auto-lock timeout using potentiometer
 * Allows user to adjust timeout value (5-30 seconds)
 */
void HandleSetTimeout(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t timeout;
    uint8_t response;
    char buffer[16];
    char key = 0;
    
    /* Display timeout adjustment screen */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Adjust Timeout");
    
    /* Let user adjust timeout with potentiometer */
    while (key != KEY_SAVE) {
        /* Read potentiometer and map to 5-30 seconds */
        timeout = (uint8_t)POT_ReadMapped(TIMEOUT_MIN_SECONDS, TIMEOUT_MAX_SECONDS);
        
        /* Display current value */
        LCD_SetCursor(1, 0);
        sprintf(buffer, "Time: %2d sec   ", timeout);
        LCD_WriteString(buffer);
        LCD_SetCursor(1, 13);
        LCD_WriteString("# =");
        
        /* Check if user pressed save */
        key = Keypad_GetKey();
        DelayMs(100);
    }
    
    /* Prompt for password confirmation */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Enter Password:");
    LCD_SetCursor(1, 0);
    GetPassword(password);
    
    /* Send set timeout command */
    UART5_SendChar(CMD_SET_TIMEOUT);
    DelayMs(50);
    SendPassword(password);
    DelayMs(50);
    UART5_SendChar(timeout);
    
    /* Wait for response */
    response = WaitForResponse();
    
    if (response == RESP_TIMEOUT_SAVED) {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Timeout Saved!");
        sprintf(buffer, "%d seconds", timeout);
        LCD_SetCursor(1, 0);
        LCD_WriteString(buffer);
        DelayMs(2000);
    } else {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Wrong Password!");
        DelayMs(1500);
    }
}

/*
 * WaitForResponse
 * Waits for a response byte from Control ECU via UART
 * Timeout: ~5 seconds
 */
uint8_t WaitForResponse(void)
{
    uint16_t timeout = 0;
    
    while (!UART5_IsDataAvailable() && timeout < UART_RESPONSE_TIMEOUT_MS)  {
        DelayMs(1);
        timeout++;
    }
    
    if (UART5_IsDataAvailable()) {
        return UART5_ReceiveChar();
    }
    
    return RESP_TIMEOUT; /* Timeout */
}

/*
 * HandleEraseEEPROM
 * Handles EEPROM erase command
 * Prompts for password confirmation before erasing
 */
void HandleEraseEEPROM(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t response;
    
    /* Display warning */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Erase EEPROM?");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Enter Password:");
    DelayMs(2000);
    
    /* Prompt for password */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_WriteString("Enter Password:");
    LCD_SetCursor(1, 0);
    GetPassword(password);
    
    /* Send erase EEPROM command */
    UART5_SendChar(CMD_ERASE_EEPROM);
    DelayMs(50);
    SendPassword(password);
    
    /* Wait for response */
    response = WaitForResponse();
    
    if (response == RESP_EEPROM_ERASED) {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("EEPROM Erased!");
        LCD_SetCursor(1, 0);
        LCD_WriteString("Restarting...");
        DelayMs(2000);
        
        /* System will restart - setup new password */
        bool passwordSet = false;
        while (!passwordSet) {
            passwordSet = SetupPassword();
        }
    } else {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_WriteString("Wrong Password!");
        LCD_SetCursor(1, 0);
        LCD_WriteString("Erase Cancelled");
        DelayMs(1500);
    }
}

/*
 * CheckPasswordExists
 * Checks if a password already exists in Control ECU EEPROM
 * Returns true if password exists, false otherwise
 */
bool CheckPasswordExists(void)
{
    uint8_t response;
    uint16_t timeout = 0;
    
    /* Send check password command */
    UART5_SendChar(CMD_CHECK_PASSWORD);
    
    /* Wait for response */
    while (!UART5_IsDataAvailable() && timeout < 2000) {
        DelayMs(1);
        timeout++;
    }
    
    if (UART5_IsDataAvailable()) {
        response = UART5_ReceiveChar();
        return (response == RESP_PASSWORD_EXISTS);
    }
    
    return false; /* Timeout - assume no password */
}

