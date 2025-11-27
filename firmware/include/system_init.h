#pragma once

// System initialization
// Called early from main.cpp to set up hardware and FreeRTOS infrastructure.

/**
 * Initialize the system:
 * - Serial logging
 * - Board hardware (GPIO, SPI bus setup)
 * - FreeRTOS queues for inter-task communication
 * - Start UI and NETSEC tasks
 */
void system_init(void);

/**
 * ARCHI-internal: initialize board-level hardware.
 * Called from system_init().
 */
void archi_init_board(void);

/**
 * ARCHI-internal: initialize LVGL and attach display/touch drivers.
 * Called from system_init().
 */
void archi_init_lvgl(void);
