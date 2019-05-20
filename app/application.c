#include <application.h>

#include "m2.h"
#include "at.h"

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

bc_button_t button_left;
bc_button_t button_right;

// Lora instance
bc_cmwx1zzabz_t lora;

bool lora_send_confimed_message = false;

extern int keyPress;

void callback(void *m);
void m_cb_lora_send(void *m);
void m_cb_lora_mode(void *m);
void m_cb_lora_join(void *m);
void m_cb_lora_confirmation(void *m);
void m_cb_lora_port(void *m);
void m_cb_lora_band(void *m);
void m_cb_lora_nwk(void *m);
void m_cb_lora_class(void *m);
void m_cb_lora_received(void *m);
void m_cb_datarate(void *m);

bool at_send(void);
bool at_status(void);

int msTick = 10;

char m_lora_send_str[16] = "...";
char m_lora_mode_str[8] = "ABP";
char m_lora_join_str[16] = "";
char m_lora_band_str[16] = "";
char m_lora_nwk_str[16] = "";
char m_lora_class_str[16] = "A";
char m_lora_received_str[16] = "";
char m_lora_datarate_str[16] = "";

int lora_port = 2;
int lora_received = 0;
int app_state = 0;


// Items
MenuItem m_item_send = {{"Send"}, m_cb_lora_send, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING, (int)m_lora_send_str};
MenuItem m_item_lora_mode = {{"Mode"}, m_cb_lora_mode, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING, (int)m_lora_mode_str};
MenuItem m_item_join = {{"Join"}, m_cb_lora_join, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING, (int)m_lora_join_str};
MenuItem m_item_confirmed_chk = {{"Confirmation"}, m_cb_lora_confirmation, MENU_CALLBACK_IS_FUNCTION | MENU_ITEM_IS_CHECKBOX , 0};
MenuItem m_item_port = {{"Port"}, m_cb_lora_port, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_NUMBER , (int)&lora_port};
MenuItem m_item_band = {{"Band"}, m_cb_lora_band, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING , (int)&m_lora_band_str};
MenuItem m_item_datarate = {{"Datarate"}, m_cb_datarate, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING , (int)&m_lora_datarate_str};
MenuItem m_item_nwk = {{"Network"}, m_cb_lora_nwk, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING, (int)&m_lora_nwk_str};
MenuItem m_item_class = {{"Class"}, m_cb_lora_class, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_STRING, (int)&m_lora_class_str};
MenuItem m_item_received = {{"Received"}, m_cb_lora_received, MENU_CALLBACK_IS_FUNCTION | MENU_PARAMETER_IS_NUMBER, (int)&lora_received};
MenuItem m_item_rx_data = {{"RX"}, 0, MENU_PARAMETER_IS_STRING, (int)&m_lora_received_str};

Menu menu_main = {
    {"BigClown LoRa v1.0"},
    .items = {&m_item_send, &m_item_lora_mode, &m_item_join, &m_item_confirmed_chk, &m_item_port, &m_item_band, &m_item_datarate, &m_item_nwk, &m_item_class, &m_item_received, &m_item_rx_data, 0},
    .refresh = 200
};

MenuItem m_band_as923 = {{"AS923"}, 0, 0, 0}; // 0
MenuItem m_band_au915 = {{"AU915"}, 0, 0, 0}; // 1
MenuItem m_band_eu868 = {{"EU868"}, 0, 0, 0}; // 5
MenuItem m_band_kr920 = {{"KR920"}, 0, 0, 0}; // 6
MenuItem m_band_in865 = {{"IN865"}, 0, 0, 0}; // 7
MenuItem m_band_us915 = {{"US915"}, 0, 0, 0}; // 8

Menu menu_band = {
    {"Band"},
    .items = {&m_band_as923, &m_band_au915, &m_band_eu868, &m_band_kr920, &m_band_in865, &m_band_us915, 0},
    .refresh = 200
};

MenuItem m_datarate_0 = {{"SF12/125kHz"}, 0, 0, 0}; // 0
MenuItem m_datarate_1 = {{"SF11/125kHz"}, 0, 0, 0}; // 1
MenuItem m_datarate_2 = {{"SF10/125kHz"}, 0, 0, 0}; // 5
MenuItem m_datarate_3 = {{"SF9/125kHz"}, 0, 0, 0}; // 6
MenuItem m_datarate_4 = {{"SF8/125kHz"}, 0, 0, 0}; // 7
MenuItem m_datarate_5 = {{"SF7/125kHz"}, 0, 0, 0}; // 8
MenuItem m_datarate_6 = {{"SF7/250kHz"}, 0, 0, 0}; // 8
MenuItem m_datarate_7 = {{"FSK 50kbit"}, 0, 0, 0}; // 8


Menu menu_datarate = {
    {"Datarate"},
    .items = {&m_datarate_0, &m_datarate_1, &m_datarate_2, &m_datarate_3, &m_datarate_4, &m_datarate_5, &m_datarate_6, &m_datarate_7, 0},
    .refresh = 200
};

void lcdBufferString(char *str, int x, int y)
{
    bc_module_lcd_draw_string(x, y, str, 1);
}

void lcdBufferNumber(int number, int x, int y)
{
    char str[16];
    snprintf(str, sizeof(str), "%d", number);
    bc_module_lcd_draw_string(x, y, str, 1);
}

// Callback
void callback(void *m)
{
    Menu* menu = ((Menu*)m);
    MenuItem *selectedItem = menu->items[menu->selectedIndex];
    (void) selectedItem;
}

// Callback
void m_cb_lora_send(void *m)
{
    Menu* menu = ((Menu*)m);
    MenuItem *selectedItem = menu->items[menu->selectedIndex];
    (void) selectedItem;
    at_send();
}

// Callback
void m_cb_lora_mode(void *m)
{
    Menu* menu = ((Menu*)m);
    MenuItem *selectedItem = menu->items[menu->selectedIndex];
    (void) selectedItem;

    if (bc_cmwx1zzabz_get_mode(&lora) == BC_CMWX1ZZABZ_CONFIG_MODE_ABP)
    {
        bc_cmwx1zzabz_set_mode(&lora, BC_CMWX1ZZABZ_CONFIG_MODE_OTAA);
        strcpy(m_lora_mode_str, "OTAA");
    }
    else
    {
        bc_cmwx1zzabz_set_mode(&lora, BC_CMWX1ZZABZ_CONFIG_MODE_ABP);
        strcpy(m_lora_mode_str, "ABP");
    }
}

void m_cb_lora_join(void *m)
{
    (void) m;
    bc_cmwx1zzabz_join(&lora);
}

void m_cb_lora_confirmation(void *m)
{
    Menu* menu = ((Menu*)m);
    MenuItem *selectedItem = menu->items[menu->selectedIndex];

    lora_send_confimed_message = (selectedItem->flags & MENU_ITEM_IS_CHECKED) ? true : false;
}

void m_cb_lora_port(void *m)
{
    (void) m;

    lora_port++;

    if (lora_port == 5)
    {
        lora_port = 0;
    }

    bc_cmwx1zzabz_set_port(&lora, lora_port);
}

void m_cb_lora_band(void *m)
{
    // Band menu
    app_state = 1;
    menu2_init(&menu_band);
}

void m_cb_lora_nwk(void *m)
{
    uint8_t public = bc_cmwx1zzabz_get_nwk_public(&lora);
    if (public)
    {
        bc_cmwx1zzabz_set_nwk_public(&lora, 0);
    }
    else
    {
        bc_cmwx1zzabz_set_nwk_public(&lora, 1);
    }
    // GUI parameter is updated in ready event handler
}

void m_cb_lora_class(void *m)
{
    bc_cmwx1zzabz_config_class_t class = bc_cmwx1zzabz_get_class(&lora);

    if (class == BC_CMWX1ZZABZ_CONFIG_CLASS_A)
    {
        bc_cmwx1zzabz_set_class(&lora, BC_CMWX1ZZABZ_CONFIG_CLASS_C);

    }
    else
    {
        bc_cmwx1zzabz_set_class(&lora, BC_CMWX1ZZABZ_CONFIG_CLASS_A);
    }
}

void m_cb_lora_received(void *m)
{
    // Clear counter of received messages
    lora_received = 0;
}

void m_cb_datarate(void *m)
{
    app_state = 2;
    menu2_init(&menu_datarate);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;

    bc_scheduler_plan_now(0);

    if (self == &button_left && event == BC_BUTTON_EVENT_CLICK)
    {
        keyPress = BTN_UP;
    }
    if (self == &button_left && event == BC_BUTTON_EVENT_HOLD)
    {
        keyPress = BTN_LEFT;
    }
    else if (self == &button_right && event == BC_BUTTON_EVENT_CLICK)
    {
        keyPress = BTN_DOWN;
    }
    else if (self == &button_right && event == BC_BUTTON_EVENT_HOLD)
    {
        keyPress = BTN_ENTER;
    }

}

void lora_ready_params_udpate()
{
    bc_cmwx1zzabz_config_band_t band = bc_cmwx1zzabz_get_band(&lora);
    if (band > 1)
    {
        // skip 3 unused bands
        band -= 3;
    }
    strncpy(m_lora_band_str, menu_band.items[band]->text[0], sizeof(m_lora_band_str));

    uint8_t public = bc_cmwx1zzabz_get_nwk_public(&lora);
    strncpy(m_lora_nwk_str, public ? "public" : "private", sizeof(m_lora_nwk_str));

    bc_cmwx1zzabz_config_class_t class = bc_cmwx1zzabz_get_class(&lora);
    strncpy(m_lora_class_str, (class == BC_CMWX1ZZABZ_CONFIG_CLASS_A) ? "A" : "C", sizeof(m_lora_class_str));

    int datarate = bc_cmwx1zzabz_get_datarate(&lora);
    strcpy(m_lora_datarate_str, menu_datarate.items[datarate]->text[0]);

}



void lora_callback(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_event_t event, void *event_param)
{
    if (event == BC_CMWX1ZZABZ_EVENT_ERROR)
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
        strcpy(m_lora_send_str, "ERR");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START)
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);
        strcpy(m_lora_send_str, "SENDING..");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
        strcpy(m_lora_send_str, "SENT");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_MESSAGE_CONFIRMED)
    {
        strcpy(m_lora_send_str, "ACK");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_MESSAGE_NOT_CONFIRMED)
    {
        strcpy(m_lora_send_str, "NACK");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_READY)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        static bool ready_flag = false;

        if (!ready_flag)
        {
            strncpy(m_lora_send_str, "READY", sizeof(m_lora_send_str));
            ready_flag = true;
        }

        lora_ready_params_udpate();
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_JOIN_SUCCESS)
    {
        bc_atci_printf("$JOIN_OK");
        strcpy(m_lora_join_str, "JOINED");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_JOIN_ERROR)
    {
        bc_atci_printf("$JOIN_ERROR");
        strcpy(m_lora_join_str, "ERROR");
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_MESSAGE_RECEIVED)
    {
        lora_received++;
        char hex[3];
        uint8_t buffer[60];
        uint8_t len = bc_cmwx1zzabz_get_received_message_data(&lora, buffer, sizeof(buffer));
        m_lora_received_str[0] = '\0';
        for (int i = 0; i < len; i++)
        {
            snprintf(hex, sizeof(hex), "%02X", buffer[i]);
            strncat(m_lora_received_str, hex, sizeof(m_lora_received_str) - 1);
        }
    }
}

bool at_send(void)
{
    static uint8_t buffer[30];

    for (int i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = i;
    }

    if (lora_send_confimed_message)
    {
        bc_cmwx1zzabz_send_message_confirmed(&lora, buffer, sizeof(buffer));
    }
    else
    {
        bc_cmwx1zzabz_send_message(&lora, buffer, sizeof(buffer));
    }


    return true;
}

bool at_status(void)
{
    bc_atci_printf("$STATUS: OK");
    return true;
}

void application_init(void)
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Init LCD
    bc_module_lcd_init();
    bc_module_lcd_set_font(&bc_font_ubuntu_13);
    bc_module_lcd_update();

    // Initialize lora module
    bc_cmwx1zzabz_init(&lora, BC_UART_UART1);
    bc_cmwx1zzabz_set_event_handler(&lora, lora_callback, NULL);
    bc_cmwx1zzabz_set_mode(&lora, BC_CMWX1ZZABZ_CONFIG_MODE_ABP);
    bc_cmwx1zzabz_set_class(&lora, BC_CMWX1ZZABZ_CONFIG_CLASS_A);

    // Initialize AT command interface
    at_init(&led, &lora);
    static const bc_atci_command_t commands[] = {
            AT_LORA_COMMANDS,
            {"$SEND", at_send, NULL, NULL, NULL, "Immediately send packet"},
            {"$STATUS", at_status, NULL, NULL, NULL, "Show status"},
            AT_LED_COMMANDS,
            BC_ATCI_COMMAND_CLAC,
            BC_ATCI_COMMAND_HELP
    };
    bc_atci_init(commands, BC_ATCI_COMMANDS_LENGTH(commands));

    const bc_button_driver_t* lcdButtonDriver =  bc_module_lcd_get_button_driver();
    bc_button_init_virtual(&button_left, 0, lcdButtonDriver, 0);
    bc_button_init_virtual(&button_right, 1, lcdButtonDriver, 0);

    bc_button_set_event_handler(&button_left, button_event_handler, (int*)0);
    bc_button_set_event_handler(&button_right, button_event_handler, (int*)1);

    bc_button_set_hold_time(&button_left, 300);
    bc_button_set_hold_time(&button_right, 300);

    bc_button_set_debounce_time(&button_left, 30);
    bc_button_set_debounce_time(&button_left, 30);

    menu2_init(&menu_main);
    menu2_init(&menu_band);
    menu2_init(&menu_datarate);

}

void application_task(void)
{
    if (!bc_module_lcd_is_ready())
    {
        bc_scheduler_plan_current_relative(20);
        return;
    }

    msTick = bc_tick_get() / 500;

    bc_system_pll_enable();

    switch (app_state)
    {
        case 0:
        {
            menu2(&menu_main);
            break;
        }

        case 1:
        {
            int ret = menu2(&menu_band);
            if (ret == -2)
            {
                // Go back
                app_state = 0;
                break;
            }
            int band_lora_index;
            if (ret >= 0)
            {
                band_lora_index = ret;
                if (band_lora_index > 1)
                {
                    // skip 3 unused bands
                    band_lora_index += 3;
                }
                strcpy(m_lora_band_str, menu_band.items[ret]->text[0]);
                bc_cmwx1zzabz_set_band(&lora, band_lora_index);
                app_state = 0;
            }
            break;
        }

        // Datarate
        case 2:
        {
            int datarate = menu2(&menu_datarate);
            if (datarate == -2)
            {
                // Go back
                app_state = 0;
                break;
            }
            if (datarate >= 0)
            {
                strcpy(m_lora_datarate_str, menu_datarate.items[datarate]->text[0]);
                bc_cmwx1zzabz_set_datarate(&lora, datarate);
                app_state = 0;
            }
            break;
        }
    }

    bc_system_pll_disable();

    bc_scheduler_plan_current_relative(200);

}
