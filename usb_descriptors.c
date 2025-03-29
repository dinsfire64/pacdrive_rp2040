
#include "tusb.h"

#define ULTIMARC_VID 0xd209
#define ULTIMARC_PID 0x1500

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = ULTIMARC_VID,
        .idProduct = ULTIMARC_PID,
        .bcdDevice = 0x0001,

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
    {
        0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
        0x09, 0x00, // Usage (Undefined)
        0xA1, 0x01, // Collection (Application)
        0x05, 0x09, //   Usage Page (Button)
        0x19, 0x01, //   Usage Minimum (0x01)
        0x29, 0x20, //   Usage Maximum (0x20)
        0x15, 0x00, //   Logical Minimum (0)
        0x25, 0x01, //   Logical Maximum (1)
        0x95, 0x20, //   Report Count (32)
        0x75, 0x01, //   Report Size (1)
        0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x08, //   Usage Page (LEDs)
        0x09, 0x4B, //   Usage (Generic Indicator)
        0x95, 0x20, //   Report Count (32)
        0x75, 0x01, //   Report Size (1)
        0x91, 0x20, //   Output (Data,Array,Abs,No Wrap,Linear,No Preferred State,No Null Position,Non-volatile)
        0xC0,       // End Collection

        // 33 bytes

};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void)itf;
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define EPNUM_HID 0x81

#define TUD_HID_IN_DESC_LEN (9 + 9 + 7)

// HID Input & Output descriptor
// Interface number, string index, protocol, report descriptor len, EP OUT & IN address, size & polling interval
#define TUD_HID_IN_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin, _epsize, _ep_interval)                                                        \
    /* Interface */                                                                                                                                                    \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_HID, (uint8_t)((_boot_protocol) ? (uint8_t)HID_SUBCLASS_BOOT : 0), _boot_protocol, _stridx, /* HID descriptor */ \
        9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0111), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(_report_desc_len),                                 /* Endpoint In */    \
        7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_IN_DESC_LEN)

uint8_t const desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),

        // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
        TUD_HID_IN_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] =
    {
        (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
        "icedragon.io",             // 1: Manufacturer
        "pacdrive clone",           // 2: Product
        "1",                        // 3: Serials will use unique ID if possible
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t chr_count;

    switch (index)
    {
    case STRID_LANGID:
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
        break;

    default:
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char *str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
        if (chr_count > max_count)
            chr_count = max_count;

        // Convert ASCII string into UTF-16
        for (size_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
        break;
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}
