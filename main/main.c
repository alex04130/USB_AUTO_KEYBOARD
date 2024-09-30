#include <stdlib.h>
#include <stdio.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"

char *TAG = "main";

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const char *string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},                                   // 0: is supported language is English (0x0409)
    "ChipLoop Tech",                                        // 1: Manufacturer
    "ChipLoop USB AUTO KEYBOARD",                           // 2: Product
    "000001",                                               // 3: Serials, should use chip ID
    "ChipLoop USB AUTO KEYBOARD simulate keyboard HID",     // 4: HID
    "ChipLoop USB AUTO KEYBOARD massive storage usb drive", // 5: MSC
};

static tusb_desc_device_t hid_descriptor_config = {
    .bLength = sizeof(hid_descriptor_config),
    .bDescriptorType = TUSB_DESC_DEVICE, //    0x01
    .bcdUSB = 0x0200,                    //    USB2.0

    .bDeviceClass = TUSB_CLASS_HID,
    .bDeviceSubClass = HID_SUBCLASS_BOOT,
    .bDeviceProtocol = HID_ITF_PROTOCOL_KEYBOARD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE, //    64

    .idVendor = 0x303A,
    .idProduct = 0x4002,
    .bcdDevice = 0x100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x02};

const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD))};

static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
}

const char *hidREportString[] = {
    "HID REPORT INVALID",
    "HID REPORT INPUT",  ///< Input
    "HID REPORT OUTPUT", ///< Output
    "HID REPORT FEATURE" ///< Feature"
};
bool caplock;

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == HID_ITF_PROTOCOL_KEYBOARD)
        {
            // bufsize should be (at least) 1
            if (reqlen < 1)
                return reqlen;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
            {
                // Capslock On: disable blink, turn led on
                caplock = true;
                ESP_LOGI(TAG, "CAP Locks ON");
            }
            else
            {
                // Caplocks Off: back to normal blink
                caplock = false;
                ESP_LOGI(TAG, "CAP Locks OFF");
            }
        }
    }
    ESP_LOGI(TAG, "Get_report Request: %d, report_id: %d, hid report type: %s", instance, report_id, hidREportString[report_type]);
    for (int i = 0; i < reqlen; i++)
    {
        ESP_LOGI(TAG, "Report %d:%d\n", i, buffer[i]);
    }
    ESP_LOGI(TAG, "end REQUSET\n");

    return reqlen;
}

static const uint8_t key_ACSII[256] = {
    HID_KEY_SPACE,         // " "
    HID_KEY_1,             // !
    HID_KEY_APOSTROPHE,    // "
    HID_KEY_3,             // #
    HID_KEY_4,             // $
    HID_KEY_5,             // %
    HID_KEY_7,             // &
    HID_KEY_APOSTROPHE,    // '
    HID_KEY_9,             // (
    HID_KEY_0,             // )
    HID_KEY_8,             // *
    HID_KEY_EQUAL,         // +
    HID_KEY_COMMA,         // ,
    HID_KEY_MINUS,         // -
    HID_KEY_PERIOD,        // .
    HID_KEY_SLASH,         // /
    HID_KEY_0,             // 0
    HID_KEY_1,             // 1
    HID_KEY_2,             // 2
    HID_KEY_3,             // 3
    HID_KEY_4,             // 4
    HID_KEY_5,             // 5
    HID_KEY_6,             // 6
    HID_KEY_7,             // 7
    HID_KEY_8,             // 8
    HID_KEY_9,             // 9
    HID_KEY_SEMICOLON,     // :
    HID_KEY_SEMICOLON,     // ;
    HID_KEY_PERIOD,        // <
    HID_KEY_EQUAL,         // =
    HID_KEY_COMMA,         // >
    HID_KEY_SLASH,         // ?
    HID_KEY_2,             // @
    HID_KEY_A,             // A
    HID_KEY_B,             // B
    HID_KEY_C,             // C
    HID_KEY_D,             // D
    HID_KEY_E,             // E
    HID_KEY_F,             // F
    HID_KEY_G,             // G
    HID_KEY_H,             // H
    HID_KEY_I,             // I
    HID_KEY_J,             // J
    HID_KEY_K,             // K
    HID_KEY_L,             // L
    HID_KEY_M,             // M
    HID_KEY_N,             // N
    HID_KEY_O,             // O
    HID_KEY_P,             // P
    HID_KEY_Q,             // Q
    HID_KEY_R,             // R
    HID_KEY_S,             // S
    HID_KEY_T,             // T
    HID_KEY_U,             // U
    HID_KEY_V,             // V
    HID_KEY_W,             // W
    HID_KEY_X,             // X
    HID_KEY_Y,             // Y
    HID_KEY_Z,             // Z
    HID_KEY_BRACKET_LEFT,  // [
    HID_KEY_BACKSLASH,     // '\'
    HID_KEY_BRACKET_RIGHT, // ]
    HID_KEY_6,             // ^
    HID_KEY_MINUS,         // _
    HID_KEY_GRAVE,         // `
    HID_KEY_A,             // a
    HID_KEY_B,             // b
    HID_KEY_C,             // c
    HID_KEY_D,             // d
    HID_KEY_E,             // e
    HID_KEY_F,             // f
    HID_KEY_G,             // g
    HID_KEY_H,             // h
    HID_KEY_I,             // i
    HID_KEY_J,             // j
    HID_KEY_K,             // k
    HID_KEY_L,             // l
    HID_KEY_M,             // m
    HID_KEY_N,             // n
    HID_KEY_O,             // o
    HID_KEY_P,             // p
    HID_KEY_Q,             // q
    HID_KEY_R,             // r
    HID_KEY_S,             // s
    HID_KEY_T,             // t
    HID_KEY_U,             // u
    HID_KEY_V,             // v
    HID_KEY_W,             // w
    HID_KEY_X,             // x
    HID_KEY_Y,             // y
    HID_KEY_Z,             // z
    HID_KEY_BRACKET_LEFT,  // {
    HID_KEY_BACKSLASH,     // |
    HID_KEY_BRACKET_RIGHT, // }
    HID_KEY_GRAVE,         // ~
};
bool keyshiftASCII[256] = {
    0, // ' '
    1, // !
    1, // "
    1, // #
    1, // $
    1, // %
    1, // &
    0, // '
    1, // (
    1, // )
    1, // *
    1, // +
    0, // ,
    0, // -
    0, // .
    0, // /
    0, // 0
    0, // 1
    0, // 2
    0, // 3
    0, // 4
    0, // 5
    0, // 6
    0, // 7
    0, // 8
    0, // 9
    1, // :
    0, // ;
    1, // <
    0, // =
    1, // >
    1, // ?
    1, // @
    1, // A
    1, // B
    1, // C
    1, // D
    1, // E
    1, // F
    1, // G
    1, // H
    1, // I
    1, // J
    1, // K
    1, // L
    1, // M
    1, // N
    1, // O
    1, // P
    1, // Q
    1, // R
    1, // S
    1, // T
    1, // U
    1, // V
    1, // W
    1, // X
    1, // Y
    1, // Z
    0, // [
    0, // '\'
    0, // ]
    1, // ^
    1, // _
    0, // `
    0, // a
    0, // b
    0, // c
    0, // d
    0, // e
    0, // f
    0, // g
    0, // h
    0, // i
    0, // j
    0, // k
    0, // l
    0, // m
    0, // n
    0, // o
    0, // p
    0, // q
    0, // r
    0, // s
    0, // t
    0, // u
    0, // v
    0, // w
    0, // x
    0, // y
    0, // z
    1, // {
    1, // |
    1, // }
    1, // ~
};

void keyboardreportstring(char *info)
{
    for (int i = 0; i < strlen(info); i++)
    {
        if (info[i] >= ' ' && info[i] <= '~')
        {
            uint8_t keycode[6] = {key_ACSII[info[i] - ' ']}, modifier = 0;
            if (keyshiftASCII[info[i] - ' '])
            {
                modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
            }
            if (!tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, modifier, keycode))
            {
                ESP_LOGE(TAG, "String report FAILED when press info[%d]:'%c'", i, info[i]);
            }
            vTaskDelay(pdMS_TO_TICKS(20));
            keycode[0] = 0;
            if (!tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, modifier, 0))
            {
                ESP_LOGE(TAG, "String report FAILED when release info[%d]:'%c'", i, info[i]);
            }
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        else
        {
            ESP_LOGE(TAG, "String number [%d] not supported!", info[i]);
        }
    }
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "GPIO initialization");
    gpio_config_t cfg = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_7 | 1ULL << GPIO_NUM_5 | 1ULL << GPIO_NUM_18};
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_LOGI(TAG, "GPIO initialization DONE");
    ESP_LOGI(TAG, "USB HID initialization");
    const tinyusb_config_t tusb_hid_cfg = {
        .device_descriptor = &hid_descriptor_config,
        .string_descriptor = string_descriptor,
        .string_descriptor_count = sizeof(string_descriptor) / sizeof(string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_hid_cfg));
    ESP_LOGI(TAG, "USB HID initialization DONE");
    bool mounted = false;
    while (1)
    {
        if (mounted)
        {
            if (!tud_mounted())
            {
                ESP_LOGI(TAG, "USB dismounted!");
                mounted = false;
            }
        }
        else if (tud_mounted())
        {
            ESP_LOGI(TAG, "USB mounted!");
            mounted = true;
        }
        if (tud_mounted())
        {
            if (gpio_get_level(GPIO_NUM_7))
            {
                while (gpio_get_level(GPIO_NUM_7))
                {
                }
                keyboardreportstring("Left!");
            }
            if (gpio_get_level(GPIO_NUM_5))
            {
                while (gpio_get_level(GPIO_NUM_5))
                {
                }
                keyboardreportstring("Hello");
            }
            if (gpio_get_level(GPIO_NUM_18))
            {
                while (gpio_get_level(GPIO_NUM_18))
                {
                }
                keyboardreportstring("Right");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
