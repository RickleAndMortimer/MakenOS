#include <pic.h>
#include <stdint.h>

#define PS2_DATA_PORT 0x80 // Read/Write
#define PS2_STATUS_REG 0x64 // Read
#define PS2_COMMAND_REG 0x64 // Write

uint8_t readDataPort() {
    return my_inb(PS2_DATA_PORT);
}

void writeDataPort(uint8_t byte) {
    my_outb(PS2_DATA_PORT, byte);
}

/*
    Status Register 
    
    Bit 0	Output buffer status (0 = empty, 1 = full)
(must be set before attempting to read data from IO port 0x60)

    Bit 1	Input buffer status (0 = empty, 1 = full)
(must be clear before attempting to write data to IO port 0x60 or IO port 0x64)

    Bit 2	System Flag
Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST)

    Bit 3	Command/data (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)
    Bit 4	Unknown (chipset specific)
May be "keyboard lock" (more likely unused on modern systems)

    Bit 5	Unknown (chipset specific)
May be "receive time-out" or "second PS/2 port output buffer full"

    Bit 6	Time-out error (0 = no error, 1 = time-out error)
    Bit 7	Parity error (0 = no error, 1 = parity error)
*/

uint8_t readStatusRegister() {
    return my_inb(PS2_STATUS_REG);
}

void writeCommandRegister(uint8_t command_byte, uint8_t next_byte) {
    my_outb(PS2_COMMAND_REG, command_byte);
    if (next_byte) {
	while (readStatusRegister() & 1) {}
	writeDataPort(next_byte);
    }
}

uint8_t testPS2Controller() {
    // test controller
    writeCommandRegister(0xAA, 0);
    return readDataPort();
}

uint8_t testPS2Port(uint8_t port) {
    // test first port
    writeCommandRegister(0xAB - 2*(port-1), 0);
    return readDataPort();
}

void enablePS2Port(uint8_t port) {
    writeCommandRegister(0xAE - 7*(port-1), 0);
}

void disablePS2Port(uint8_t port) {
    // port 1 disable
    writeCommandRegister(0xAD - 7*(port-1), 0);
}

/*
    Byte 0: Controller Configuration Byte

    Bit 0	First PS/2 port interrupt (1 = enabled, 0 = disabled)
    Bit 1	Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)
    Bit 2	System Flag (1 = system passed POST, 0 = your OS shouldn't be running)
    Bit 3	Should be zero
    Bit 4	First PS/2 port clock (1 = disabled, 0 = enabled)
    Bit 5	Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported)
    Bit 6	First PS/2 port translation (1 = enabled, 0 = disabled)
    Bit 7	Must be zero

    Byte range: 0-0x1F
*/


uint8_t readPS2RAM(uint8_t byte) {
    writeCommandRegister(0x20+byte, 0);
    return readDataPort();
}

void writePS2RAM(uint8_t byte) {
    writeCommandRegister(0x60+byte, byte);
}

void writeConfigurationByte(uint8_t byte) {
    writeCommandRegister(0x60, byte);
}

// copies bits from the input port to the status register
// high = 1 for bits 7..4, 0 for bits 3...0
void copyToStatus(uint8_t high) {
    writeCommandRegister(0xC1 + high, 0);
}

/*
   PS2 Controller Output Port 

   Bit 0	System reset (output) WARNING always set to '1'. You need to pulse the reset line (e.g. using command 0xFE), and setting this bit to '0' can lock the computer up ("reset forever").

   Bit 1	A20 gate (output)
   Bit 2	Second PS/2 port clock (output, only if 2 PS/2 ports supported)
   Bit 3	Second PS/2 port data (output, only if 2 PS/2 ports supported)
   Bit 4	Output buffer full with byte from first PS/2 port (connected to IRQ1)
   Bit 5	Output buffer full with byte from second PS/2 port (connected to IRQ12, only if 2 PS/2 ports supported)
   Bit 6	First PS/2 port clock (output)
   Bit 7	First PS/2 port data (output)

   controller output port: 0
   first PS2 port: 1
   second PS2 port: 2
   second PS2 port (with sender): 3
*/

uint8_t readControllerOutputBuffer() {
    writeCommandRegister(0xD0, 0);
    return readDataPort();
}

void writeOutputBuffer(uint8_t byte, uint8_t port) {
    writeCommandRegister(0xD1 + port, byte);
}

// Lines are mapped by bit number 
// 0xF0-0xFF
void pulseOutputLines(uint8_t lines) {
    writeCommandRegister(0xF0 + lines, 0);
}

void initKeyboard() {
    // TODO: Initalise USB Controllers
    
    // TODO: Check ACPI for PS/2 Controller

    // Disable devices
    disablePS2Port(1);
    disablePS2Port(2);

    // Flush the output buffer
    readControllerOutputBuffer();

    // Set Controller Configuration Byte
    disableAllIRQs();
    uint8_t configuration_byte = readPS2RAM(0);
    uint8_t two_channel = ~(configuration_byte & 0b00010000);
    if (two_channel) {
    	disablePS2Port(2);
    }
    configuration_byte &= 0b11011100;
    writeConfigurationByte(configuration_byte);

    // Perform controller self-test
    testPS2Controller();

    // Determine if there are two channels
    if (two_channel) {
	enablePS2Port(2);
	configuration_byte = readPS2RAM(0);
	if (configuration_byte & 0b00010000) {
	    two_channel = 0; 
	}
	else {
	    disablePS2Port(2);
	}
    }

    // Perform Inteface tests and enable devices
    uint8_t port1_status = testPS2Port(1);
    switch (port1_status) {
	case 0x00:
	    enablePS2Port(1);
	    configuration_byte = readPS2RAM(0);
	    writeConfigurationByte(configuration_byte | 1);
	    break;
	case 0x01:
	    // clock line stuck low
	    break;
	case 0x02:
	    // clock line stuck high
	    break;
	case 0x03:
	    // data line stuck low
	    break;
	case 0x04:
	    // data line stuck high
	    break;
    }

    if (two_channel) {
    	uint8_t port2_status = testPS2Port(2);
	switch (port2_status) {
	    case 0x00:
	        enablePS2Port(2);
	    	configuration_byte = readPS2RAM(0);
	    	writeConfigurationByte(configuration_byte | 2);
	        break;
	    case 0x01:
	        // clock line stuck low
	        break;
	    case 0x02:
	        // clock line stuck high
	        break;
	    case 0x03:
	        // data line stuck low
	        break;
	    case 0x04:
	        // data line stuck high
	        break;
	}
    }
    // Reset Devices
   
    clearMaskIRQ(1);
}


