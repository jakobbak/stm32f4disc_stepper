#ifndef USB_SERIAL_H
#define USB_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

void usb_serial_init();
void usb_serial_handle_tx_buffer_streaming();

bool usb_serial_get_byte(uint8_t* data);
void usb_serial_write_buffer(uint8_t* buf, int len);


#endif // USB_SERIAL_H
