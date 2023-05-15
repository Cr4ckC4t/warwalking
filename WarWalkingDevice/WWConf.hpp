// SD module related
#define SPI_SD_CS 5 // GPIO 5 (SPI Chip Select) for SD card module
#define LOG_FILE_NAME "ww_scan.csv"

// Display related
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_I2C_ADDR 0x3c
#define DISPLAY_RESET_PIN -1 // Has no reset pin

// GPS module related
#define GPS_BAUDRATE 9600

// AP scans
#define SCAN_INTERVAL_S 5 // seconds
#define ASYNC_SCAN false
#define DETECT_HIDDEN true
