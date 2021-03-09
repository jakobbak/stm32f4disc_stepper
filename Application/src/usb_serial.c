#include <stdint.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include "usbd_cdc_if.h"
#include "main.h"
#include "usb_serial.h"

#define USB_SERIAL_NO_DATA 0xFF

// Max buffer size: 64k (65535)
#define RX_BUFFER_SIZE (1024)
#define TX_BUFFER_SIZE (4096)

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile int tx_head = 0;
static volatile int tx_tail = 0;

extern USBD_HandleTypeDef hUsbDeviceFS;
bool usb_serial_tx_buffer_streaming_locked;
// Forward declare function defined at the bottom of the file
static void hack_USBD_CDC_Callbacks(void);


// we need this function to be defined somewhere for printf to work
int _write(int file, char *ptr, int len) {
    usb_serial_write_buffer((uint8_t*)ptr, len);
    return len;
}


void usb_serial_init() {
    memset(tx_buffer, 0, TX_BUFFER_SIZE);
    tx_head = 0;
    tx_tail = 0;
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
    rx_head = 0;
    rx_tail = 0;
    hack_USBD_CDC_Callbacks();
    usb_serial_tx_buffer_streaming_locked = false;
}


static void handle_incoming_byte(uint8_t data) {
    __DSB();
    int next_head = rx_head + 1;
    if (next_head == RX_BUFFER_SIZE) { next_head = 0; }
    
    // Write data to buffer unless it is full.
    if (next_head != rx_tail) {
        rx_buffer[rx_head] = data;
        __DMB();
        rx_head = next_head;
    }
    __DSB();
}


static void usb_serial_handle_rx_buffer_streaming(uint8_t* pbuf, uint16_t len) {
    for(int i=0; i < len; i++) {
        handle_incoming_byte(pbuf[i]);
    }
}


void usb_serial_handle_tx_buffer_streaming() {
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (hcdc->TxState != 0){
        return;
    }
    if(usb_serial_tx_buffer_streaming_locked) {
        return;
    }
    usb_serial_tx_buffer_streaming_locked = true;
    
    // Stop streaming if this is the end of the buffer.
    int tail = tx_tail;
    int head = tx_head;
    int next_tail;
    if (tail == head) {
        usb_serial_tx_buffer_streaming_locked = false;
        return;
    }
    int len;
    if(tail < head) {
        len = head - tail;
    } else {
        len = TX_BUFFER_SIZE - tail;
    }
    if(len > 64) len = 64;
    uint8_t status = CDC_Transmit_FS(&tx_buffer[tail], len);
    __DSB();
    if(status == USBD_OK) {
        next_tail = tail + len;
        if (next_tail >= TX_BUFFER_SIZE) {
            next_tail = next_tail % TX_BUFFER_SIZE;
        }
        tx_tail = next_tail;
    }
    usb_serial_tx_buffer_streaming_locked = false;
}


void usb_serial_write(uint8_t data) {
    // Calculate next head
    volatile int this_tail = tx_tail;
    volatile int this_head = tx_head;
    volatile int next_head = this_head + 1;
    if (next_head >= TX_BUFFER_SIZE) { next_head = 0; }
    
    // Wait until there is space in the buffer
    // while (next_head == tx_tail) {
    //     usb_serial_handle_tx_buffer_streaming();
    // }
    // Or not, don't wait until there is space in the buffer, as this will slow us down. better to loose data....?
    if (next_head == this_tail) {
        return;
    }
    // Store data and advance head
    tx_buffer[this_head] = data;
    tx_head = next_head;
}


void usb_serial_write_buffer(uint8_t* buf, int len) {
    int i = 0;
    bool initiate_transfer = false;
    if(tx_tail == tx_head) {
        initiate_transfer = true;
    }
    while (i < len) {
        usb_serial_write(buf[i++]);
    }
    if(initiate_transfer) {
        usb_serial_handle_tx_buffer_streaming();
    }
}


bool usb_serial_get_byte(uint8_t* data) {
    __DSB();
    if (rx_head == rx_tail) {
        return false;
    } else {
        *data = rx_buffer[rx_tail];
        __DMB();
        rx_tail++;
        if (rx_tail == RX_BUFFER_SIZE) { rx_tail = 0; }
        return true;
    }
    __DSB();
}


uint8_t hacked_USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)pdev->pClassData;
    PCD_HandleTypeDef *hpcd = pdev->pData;
    
    if(pdev->pClassData != NULL)
    {
        if((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
        {
            /* Update the packet total length */
            pdev->ep_in[epnum].total_length = 0U;
            
            /* Send ZLP */
            USBD_LL_Transmit (pdev, epnum, NULL, 0U);
        }
        else
        {
            hcdc->TxState = 0U;
            usb_serial_handle_tx_buffer_streaming();
        }
        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}


uint8_t hacked_USBD_CDC_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
    
    /* Get the received data length */
    hcdc->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
    
    /* USB data will be immediately processed, this allow next USB traffic being
    NAKed till the end of the application Xfer */
    if(pdev->pClassData != NULL)
    {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Receive(hcdc->RxBuffer, &hcdc->RxLength);
        usb_serial_handle_rx_buffer_streaming(hcdc->RxBuffer, hcdc->RxLength);
        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}


static void hack_USBD_CDC_Callbacks() {
    USBD_CDC.DataIn = hacked_USBD_CDC_DataIn;
    USBD_CDC.DataOut = hacked_USBD_CDC_DataOut;
}
