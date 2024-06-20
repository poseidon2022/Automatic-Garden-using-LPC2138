#include<NXP/iolpc2138.h>
#include <stdio.h>
#define bit(x) (1 << x)

void init();
void delay(int);
_Bool turn_on_relay(unsigned int);
void lcd_init();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(const char *s);
unsigned short sensor_reading_converted();

void main() {
    init();
    lcd_init();
    delay(100000);

    while (1) {
        unsigned int temp = sensor_reading_converted(); // ADC 8-bit resolution value

        // Correct percentage conversion
        unsigned int percentage_moisture = (temp * 100) / 255;

        // Display the moisture percentage on the LCD
        char message[50];
        sprintf(message, "SoilMoisture:%d%%", percentage_moisture);
        cmd(0x01);
        show(message);

        // Control the relay based on moisture level
        if (turn_on_relay(percentage_moisture)) {
            IO0SET = bit(8); // Set the eighth pin
        } else {
            IO0CLR = bit(8); // Clear the eighth pin
        }

        delay(100000); // Delay to avoid flooding the LCD with updates
    }
}

_Bool turn_on_relay(unsigned int perc) {
    return perc < 50;
}

void init() {
    // Set the direction of the I/O pins to output
    IO1DIR = 0xFFFFFFFF;  // Set all pins of PORT1 to output
    IO0DIR &= 0xFFFFFF00;      // Set only pin 8 of PORT0 to output
    IO0DIR |= (1<<8);
}

unsigned short sensor_reading_converted() {

    unsigned short result = IO0PIN & 0xFF; // Extract the 8-bit result
    return result;
}

void delay(int val) {
    for (int i = 0; i < val; i++) {}
}

void lcd_init() {
    cmd(0x38);   // 2 lines, 5x7 matrix
    cmd(0x0E);   // Display on, cursor blinking
    cmd(0x06);   // Cursor auto increment
    cmd(0x0C);   // Display on, cursor off
    cmd(0x80);   // Cursor at home position
}

void cmd(unsigned char a) {
    IO1PIN = (a << 24);  // Send command to data lines
    IO1CLR = bit(16);    // rs = 0 (Command mode)
    IO1CLR = bit(17);    // rw = 0 (Write mode)
    IO1SET = bit(18);    // en = 1 (Enable pulse)
    delay(5000);         // Delay 50 microseconds (adjust as needed)
    IO1CLR = bit(18);    // en = 0 (Disable enable pulse)
}

void dat(unsigned char b) {
    IO1PIN = (b << 24);  // Send data to data lines
    IO1SET = bit(16);    // rs = 1 (Data mode)
    IO1CLR = bit(17);    // rw = 0 (Write mode)
    IO1SET = bit(18);    // en = 1 (Enable pulse)
    delay(5000);         // Delay 50 microseconds (adjust as needed)
    IO1CLR = bit(18);    // en = 0 (Disable enable pulse)
}

void show(const char *s) {
    while (*s) {
        dat(*s++);
    }
}
