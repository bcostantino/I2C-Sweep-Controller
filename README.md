# I2C-Sweep-Controller
This is a modified version of the MSP430 I2C controller code found [here](https://dev.ti.com/tirex4-desktop/nodeContent?devices=MSP430G2553&node=AGuta3AAbGCV3.WXIWOnDw__IOGqZri__LATEST&search=I2C), written and debugged in Code Composer Studio. This code will sweep all 7-bit I2C peripheral addresses (0x01 - 0x7F), and return an array containing all valid peripheral addresses.

### Description
`main.c` contains register level code, written for the MSP430G2553. The code begins by initializing the I2C communication as a controller, and defining a starting address (0x00) for the sweep. Then, as the start of an iterative process, the controller generates a START condition on the I2C bus, and transmits the current peripheral address. If the NACK condition is set, then the peripheral did not acknowledge the request, thus the current address is invalid, and the peripheral address is incrimented. 

This process continues until the request is acknowledged by a peripheral device. In which case, the RX interrupt flag is set, and the code jumps to an ISR, which cleares the RXBUF register and adds the current peripheral address to a list of valid peripheral addresses.

### Resources
- https://www.ti.com/lit/ug/slau144j/slau144j.pdf
