/******************************************************************************
 * File: main.c
 * Module: Control_ECU (Control Electronic Control Unit)
 * Description: Smart Door Lock System - Control Application
 * 
 * System Features:
 *   - Password storage and verification (EEPROM)
 *   - Motor control for door lock/unlock
 *   - Buzzer alarm for security
 *   - Auto-lock timeout configuration
 *   - Communication with HMI ECU via UART5
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "uart.h"
#include "eeprom.h"
#include "motor.h"
#include "buzzer.h"
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

/* EEPROM Memory Map */
#define EEPROM_PASSWORD_BLOCK   0
#define EEPROM_PASSWORD_OFFSET  0
#define EEPROM_TIMEOUT_BLOCK    0
#define EEPROM_TIMEOUT_OFFSET   2
#define EEPROM_VALID_FLAG_BLOCK 0
#define EEPROM_VALID_FLAG_OFFSET 3
#define PASSWORD_VALID_MARKER   0xAA55AA55

/* Default Settings */
#define DEFAULT_TIMEOUT         5   /* 5 seconds default */
#define LOCKOUT_DURATION        10  /* 10 seconds lockout after 3 failed attempts */

/******************************************************************************
 *                          Global Variables                                   *
 ******************************************************************************/

static char g_storedPassword[PASSWORD_LENGTH + 1];
static uint8_t g_autoLockTimeout = DEFAULT_TIMEOUT;

/******************************************************************************
 *                          Function Prototypes                                *
 ******************************************************************************/

void ReceivePassword(char *password);
bool VerifyPassword(const char *password);
void SavePassword(const char *password);
void LoadPassword(void);
void LoadTimeout(void);
void SaveTimeout(uint8_t timeout);
bool IsPasswordValid(void);
void MarkPasswordAsValid(void);
void HandleCheckPassword(void);
void HandleSetupPassword(void);
void HandleChangePassword(void);
void HandleSetTimeout(void);
void HandleOpenDoor(void);
void HandleEraseEEPROM(void);
void HandleTriggerLockout(void);
void PerformDoorOperation(void);
void TriggerLockout(void);
void SendCountdown(uint8_t seconds);

/******************************************************************************
 *                          Main Application                                   *
 ******************************************************************************/

int main(void)
{
    uint8_t command;
    
    /* Initialize all peripherals */
    SysTick_Init(16000, SYSTICK_NOINT);
    UART5_Init();  
    EEPROM_Init();
    Motor_Init();      /* Initialize motor first (PF0, PF4) */
    Buzzer_Init();     /* Initialize buzzer after motor (PF1) */
    
    /* Load stored password and timeout from EEPROM */
    LoadPassword();
    LoadTimeout();
    
    /* Main loop - wait for commands from HMI ECU */
    while (1) {
        /* Wait for command from HMI */
        if (UART5_IsDataAvailable()) {
            command = UART5_ReceiveChar();
            
            /* Process command */
            switch (command) {
                case CMD_CHECK_PASSWORD:
                    HandleCheckPassword();
                    break;
                    
                case CMD_SETUP_PASSWORD:
                    HandleSetupPassword();
                    break;
                    
                case CMD_CHANGE_PASSWORD:
                    HandleChangePassword();
                    break;
                    
                case CMD_SET_TIMEOUT:
                    HandleSetTimeout();
                    break;
                    
                case CMD_OPEN_DOOR:
                    HandleOpenDoor();
                    break;
                    
                case CMD_ERASE_EEPROM:
                    HandleEraseEEPROM();
                    break;
                    
                case CMD_TRIGGER_LOCKOUT:
                    HandleTriggerLockout();
                    break;
                    
                default:
                    /* Unknown command - ignore */
                    break;
            }
        }
        
        DelayMs(10);
    }
}

/******************************************************************************
 *                          Function Implementations                           *
 ******************************************************************************/

/*
 * ReceivePassword
 * Receives a 5-character password from UART
 */
void ReceivePassword(char *password)
{
    uint8_t i;
    uint16_t timeout;
    
    for (i = 0; i < PASSWORD_LENGTH; i++) {
        timeout = 0;
        
        /* Wait for character with timeout */
        while (!UART5_IsDataAvailable() && timeout < 1000) {
            DelayMs(1);
            timeout++;
        }
        
        if (UART5_IsDataAvailable()) {
            password[i] = UART5_ReceiveChar();
        } else {
            password[i] = '0'; /* Default on timeout */
        }
    }
    
    password[PASSWORD_LENGTH] = '\0';
}

/*
 * VerifyPassword
 * Compares received password with stored password
 * Returns true if match, false otherwise
 */
bool VerifyPassword(const char *password)
{
    return (strcmp(password, g_storedPassword) == 0);
}

/*
 * SavePassword
 * Saves password to EEPROM
 */
void SavePassword(const char *password)
{
    uint32_t data1, data2;
    
    /* Pack password into two 32-bit words */
    data1 = ((uint32_t)password[0] << 24) | 
            ((uint32_t)password[1] << 16) | 
            ((uint32_t)password[2] << 8) | 
            ((uint32_t)password[3]);
    
    data2 = ((uint32_t)password[4] << 24);
    
    /* Write to EEPROM */
    EEPROM_WriteWord(EEPROM_PASSWORD_BLOCK, EEPROM_PASSWORD_OFFSET, data1);
    DelayMs(10); /* Delay to ensure write completes */
    EEPROM_WriteWord(EEPROM_PASSWORD_BLOCK, EEPROM_PASSWORD_OFFSET + 1, data2);
    DelayMs(10); /* Delay to ensure write completes */
    
    /* Mark password as valid */
    MarkPasswordAsValid();
    DelayMs(10); /* Delay to ensure write completes */
    
    /* Update global variable */
    strcpy(g_storedPassword, password);
}

/*
 * LoadPassword
 * Loads password from EEPROM
 */
void LoadPassword(void)
{
    uint32_t data1, data2;
    
    /* Read from EEPROM */
    EEPROM_ReadWord(EEPROM_PASSWORD_BLOCK, EEPROM_PASSWORD_OFFSET, &data1);
    EEPROM_ReadWord(EEPROM_PASSWORD_BLOCK, EEPROM_PASSWORD_OFFSET + 1, &data2);
    
    /* Unpack password */
    g_storedPassword[0] = (char)((data1 >> 24) & 0xFF);
    g_storedPassword[1] = (char)((data1 >> 16) & 0xFF);
    g_storedPassword[2] = (char)((data1 >> 8) & 0xFF);
    g_storedPassword[3] = (char)(data1 & 0xFF);
    g_storedPassword[4] = (char)((data2 >> 24) & 0xFF);
    g_storedPassword[5] = '\0';
}

/*
 * LoadTimeout
 * Loads auto-lock timeout from EEPROM
 */
void LoadTimeout(void)
{
    uint32_t data;
    
    /* Read from EEPROM */
    EEPROM_ReadWord(EEPROM_TIMEOUT_BLOCK, EEPROM_TIMEOUT_OFFSET, &data);
    
    g_autoLockTimeout = (uint8_t)(data & 0xFF);
    
    /* Validate range (5-30 seconds) */
    if (g_autoLockTimeout < 5 || g_autoLockTimeout > 30) {
        g_autoLockTimeout = DEFAULT_TIMEOUT;
    }
}

/*
 * SaveTimeout
 * Saves auto-lock timeout to EEPROM
 */
void SaveTimeout(uint8_t timeout)
{
    uint32_t data = (uint32_t)timeout;
    
    /* Write to EEPROM */
    EEPROM_WriteWord(EEPROM_TIMEOUT_BLOCK, EEPROM_TIMEOUT_OFFSET, data);
    DelayMs(10); /* Delay to ensure write completes */
    
    /* Update global variable */
    g_autoLockTimeout = timeout;
}

/*
 * HandleSetupPassword
 * Handles initial password setup command
 * Receives two passwords and compares them
 */
void HandleSetupPassword(void)
{
    char password1[PASSWORD_LENGTH + 1];
    char password2[PASSWORD_LENGTH + 1];
    
    /* Receive both passwords */
    ReceivePassword(password1);
    DelayMs(50);
    ReceivePassword(password2);
    
    /* Compare passwords */
    if (strcmp(password1, password2) == 0) {
        /* Passwords match - save to EEPROM */
        SavePassword(password1);
        DelayMs(100); /* Ensure EEPROM write completes */
        UART5_SendChar(RESP_PASSWORD_MATCH);
    } else {
        /* Passwords don't match */
        UART5_SendChar(RESP_PASSWORD_MISMATCH);
    }
}

/*
 * HandleChangePassword
 * Handles change password command
 * Verifies old password before allowing change
 */
void HandleChangePassword(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t attempts = 0;
    
    /* Receive and verify old password */
    ReceivePassword(password);
    
    if (VerifyPassword(password)) {
        /* Old password correct */
        UART5_SendChar(RESP_PASSWORD_MATCH);
    } else {
        /* Old password incorrect */
        UART5_SendChar(RESP_PASSWORD_MISMATCH);
    }
}

/*
 * HandleSetTimeout
 * Handles set timeout command
 * Verifies password before saving new timeout
 */
void HandleSetTimeout(void)
{
    char password[PASSWORD_LENGTH + 1];
    uint8_t timeout;
    uint16_t waitTime;
    
    /* Receive password */
    ReceivePassword(password);
    
    /* Wait for timeout value */
    waitTime = 0;
    while (!UART5_IsDataAvailable() && waitTime < 1000) {
        DelayMs(1);
        waitTime++;
    }
    
    if (UART5_IsDataAvailable()) {
        timeout = UART5_ReceiveChar();
    } else {
        timeout = DEFAULT_TIMEOUT;
    }
    
    /* Verify password */
    if (VerifyPassword(password)) {
        /* Password correct - save timeout */
        SaveTimeout(timeout);
        DelayMs(50); /* Ensure EEPROM write completes */
        UART5_SendChar(RESP_TIMEOUT_SAVED);
    } else {
        /* Password incorrect */
        UART5_SendChar(RESP_PASSWORD_MISMATCH);
    }
}

/*
 * HandleOpenDoor
 * Handles open door command
 * Verifies password and performs door operation
 */
void HandleOpenDoor(void)
{
    char password[PASSWORD_LENGTH + 1];
    
    /* Receive password */
    ReceivePassword(password);
    
    /* Verify password */
    if (VerifyPassword(password)) {
        /* Password correct */
        UART5_SendChar(RESP_PASSWORD_MATCH);
        DelayMs(100);
        
        /* Perform door unlock/lock sequence */
        PerformDoorOperation();
    } else {
        /* Password incorrect */
        UART5_SendChar(RESP_PASSWORD_MISMATCH);
    }
}

/*
 * PerformDoorOperation
 * Executes the door unlock/lock sequence
 * 1. Unlock (rotate motor CW for 2 seconds)
 * 2. Wait (door remains open with countdown)
 * 3. Lock (rotate motor CCW for 2 seconds)
 */
void PerformDoorOperation(void)
{
    uint8_t i;
    
    /* Step 1: Unlock door */
    UART5_SendChar(RESP_DOOR_UNLOCKING);
    Motor_RotateCW();
    DelayMs(2000); /* 2 seconds to unlock */
    Motor_Stop();
    
    /* Step 2: Wait while door is open with countdown */
    UART5_SendChar(RESP_COUNTDOWN_START); /* Signal countdown is starting */
    DelayMs(50);
    
    for (i = g_autoLockTimeout; i > 0; i--) {
        SendCountdown(i);
        DelayMs(1000);
    }
    
    /* Step 3: Lock door */
    DelayMs(100); /* Small delay before sending lock command */
    UART5_SendChar(RESP_DOOR_LOCKING);
    Motor_RotateCCW();
    DelayMs(2000); /* 2 seconds to lock */
    Motor_Stop();
    
    /* Door locked */
    DelayMs(50);
    UART5_SendChar(RESP_DOOR_LOCKED);
}

/*
 * TriggerLockout
 * Triggers security lockout after 3 failed attempts
 * Sounds buzzer for LOCKOUT_DURATION seconds
 */
void TriggerLockout(void)
{
    uint8_t i;
    
    /* Send lockout notification */
    UART5_SendChar(RESP_SYSTEM_LOCKED);
    
    /* Sound buzzer - beep pattern for lockout duration */
    for (i = 0; i < LOCKOUT_DURATION; i++) {
        Buzzer_Beep(800);  /* 800ms beep */
        DelayMs(200);      /* 200ms silence */
    }
}

/*
 * HandleEraseEEPROM
 * Handles EEPROM erase command
 * Verifies password before erasing entire EEPROM
 */
void HandleEraseEEPROM(void)
{
    char password[PASSWORD_LENGTH + 1];
    
    /* Receive password */
    ReceivePassword(password);
    
    /* Verify password */
    if (VerifyPassword(password)) {
        /* Password correct - erase EEPROM */
        EEPROM_MassErase();
        
        /* Reset password and timeout to defaults */
        strcpy(g_storedPassword, "00000");
        g_autoLockTimeout = DEFAULT_TIMEOUT;
        
        /* Send success response */
        DelayMs(50);
        UART5_SendChar(RESP_EEPROM_ERASED);
    } else {
        /* Password incorrect */
        UART5_SendChar(RESP_PASSWORD_MISMATCH);
    }
}

/*
 * IsPasswordValid
 * Checks if a valid password exists in EEPROM
 */
bool IsPasswordValid(void)
{
    uint32_t validFlag;
    
    EEPROM_ReadWord(EEPROM_VALID_FLAG_BLOCK, EEPROM_VALID_FLAG_OFFSET, &validFlag);
    
    return (validFlag == PASSWORD_VALID_MARKER);
}

/*
 * MarkPasswordAsValid
 * Marks password as valid in EEPROM
 */
void MarkPasswordAsValid(void)
{
    EEPROM_WriteWord(EEPROM_VALID_FLAG_BLOCK, EEPROM_VALID_FLAG_OFFSET, PASSWORD_VALID_MARKER);
}

/*
 * HandleTriggerLockout
 * Handles lockout trigger command from HMI
 * Activates buzzer alarm
 */
void HandleTriggerLockout(void)
{
    TriggerLockout();
}

/*
 * HandleCheckPassword
 * Checks if password exists and responds to HMI
 */
void HandleCheckPassword(void)
{
    if (IsPasswordValid()) {
        UART5_SendChar(RESP_PASSWORD_EXISTS);
    } else {
        UART5_SendChar(RESP_NO_PASSWORD);
    }
}

/*
 * SendCountdown
 * Sends countdown value to HMI
 */
void SendCountdown(uint8_t seconds)
{
    UART5_SendChar(seconds);
}
