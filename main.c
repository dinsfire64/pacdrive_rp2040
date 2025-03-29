#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "tusb.h"
#include "pico/stdlib.h"

#define NUM_OF_LEDS 16
#define HID_WRITE_SIZE 4

// map of pins to pacdrive LED numbers
// use -1 if not mapped.
int pin_mapping[NUM_OF_LEDS] =
    {
        // starting with LED 1 -> 16
        2,
        3,
        4,
        5,

        6,
        7,
        8,
        9,

        10,
        11,
        12,
        13,

        18,
        19,
        20,
        21,
};

uint16_t current_led_state = 0;

int main(void)
{
    // setup/init the pins.
    for (int i = 0; i < NUM_OF_LEDS; i++)
    {
        if (pin_mapping[i])
        {
            gpio_init(pin_mapping[i]);
            gpio_set_dir(pin_mapping[i], GPIO_OUT);

            // start high just like the pacdrive.
            gpio_put(pin_mapping[i], 1);
        }
    }

    stdio_init_all();
    printf("\n\nStart " __DATE__ " "__TIME__
           "\n\n");

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

    if (bufsize == HID_WRITE_SIZE)
    {
        // values are in [2] and [3]
        // 16 downto 1, active high.
        // printf("%02x %02x %02x %02x\n", buffer[0], buffer[1], buffer[2], buffer[3]);

        uint16_t incoming_state = (buffer[2] << 8) | buffer[3];

        // only output on changes in state
        // some games will write duplicate information.
        if (incoming_state != current_led_state)
        {
            printf("%04x\n", incoming_state);

            for (int i = 0; i < NUM_OF_LEDS; i++)
            {
                // active high values.
                bool state = incoming_state & (1 << i);

                if (pin_mapping[i])
                {
                    gpio_put(pin_mapping[i], state);
                }
            }

            current_led_state = incoming_state;
        }
    }
    else
    {
        printf("UNKN: %02x %02x %02x: %d\n", itf, report_id, report_type, bufsize);
    }
}
