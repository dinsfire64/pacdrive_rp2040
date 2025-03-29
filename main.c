#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tusb.h"
#include "pico/stdlib.h"

#define PIN_ONBOARD_LED 25

int main(void)
{
    gpio_init(PIN_ONBOARD_LED);
    gpio_set_dir(PIN_ONBOARD_LED, GPIO_OUT);
    gpio_put(PIN_ONBOARD_LED, 0);

    stdio_init_all();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    while (1)
    {
        tud_task(); // tinyusb device task
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // This example doesn't use multiple report and report ID
    (void)itf;
    (void)report_id;
    (void)report_type;

    if (bufsize == 4)
    {
        // values are in [2] and [3]
        // 16 downto 1, active high.
        printf("%02x %02x %02x %02x\n", buffer[0], buffer[1], buffer[2], buffer[3]);

        // tie LED #1 to the onboard LED.
        gpio_put(PIN_ONBOARD_LED, buffer[3] & 0x01);
    }
    else
    {
        printf("UNKN: %02x %02x %02x: %d\n", itf, report_id, report_type, bufsize);
    }
}
