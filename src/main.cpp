#include <Arduino.h>
#include <WiFi.h>
#include <BleKeyboard.h>
#include <fabgl.h>
using namespace fabgl;

static bool connected = false;
static BleKeyboard bleKeyboard;
static PS2Controller ps2Controller;
static Keyboard* keyboard;

uint8_t convertToKeyCode(VirtualKey virtualKey);

void setup() 
{
    Serial.begin(115200);
    Serial.println("Starting");

    bleKeyboard.begin();
    ps2Controller.begin(PS2Preset::KeyboardPort0);
    keyboard = ps2Controller.keyboard();
    Serial.println("Started");
}

void loop() 
{
    if (connected) 
    {
        if (!bleKeyboard.isConnected())
        {
            connected = false;
            Serial.println("Disconnected");
        }
    }
    else
    {
        if (bleKeyboard.isConnected())
        {
            connected = true;
            Serial.println("Connected");
        }
    }
    
    if (!connected) 
    {
        return;
    }

    if (!keyboard->virtualKeyAvailable())
    {
        return;
    }

    bool keyDown;
    VirtualKey virtualKey = keyboard->getNextVirtualKey(&keyDown);
    uint8_t keyCode = convertToKeyCode( virtualKey);
    if (keyDown)
    {
        bleKeyboard.press(keyCode);
    }
    else
    {
        bleKeyboard.release(keyCode);
    }
}

uint8_t convertToKeyCode(VirtualKey virtualKey)
{
    switch (virtualKey)
    {
        case VK_RETURN:
        case VK_KP_ENTER:
            return KEY_RETURN;

        case VK_TAB:
            return KEY_TAB;

        case VK_HOME:
            return KEY_HOME;

        case VK_END:
            return KEY_END;

        case VK_LEFT:
        case VK_KP_LEFT:
            return KEY_LEFT_ARROW;

        case VK_RIGHT:
        case VK_KP_RIGHT:
            return KEY_RIGHT_ARROW;

        case VK_UP:
        case VK_KP_UP:
            return KEY_UP_ARROW;

        case VK_DOWN:
        case VK_KP_DOWN:
            return KEY_DOWN_ARROW;

        case VK_LSHIFT:
            return KEY_LEFT_SHIFT;

        case VK_RSHIFT:
            return KEY_RIGHT_SHIFT;

        case VK_LCTRL:
            return KEY_LEFT_CTRL;

        case VK_RCTRL:
            return KEY_RIGHT_CTRL;

        case VK_LALT:
            return KEY_LEFT_ALT;

        case VK_RALT:
            return KEY_RIGHT_ALT;

        case VK_LGUI:
            return KEY_LEFT_GUI;

        case VK_RGUI:
            return KEY_RIGHT_GUI;

        case VK_PAGEUP:
        case VK_KP_PAGEUP:
            return KEY_PAGE_UP;

        case VK_PAGEDOWN:
        case VK_KP_PAGEDOWN:
            return KEY_PAGE_DOWN;

        case VK_ESCAPE:
            return KEY_ESC;

        case VK_INSERT:
        case VK_KP_INSERT:
            return KEY_INSERT;

        case VK_DELETE:
        case VK_KP_DELETE:
            return KEY_DELETE;

        case VK_F1:
            return KEY_F1;

        case VK_F2:
            return KEY_F2;

        case VK_F3:
            return KEY_F3;

        case VK_F4:
            return KEY_F4;

        case VK_F5:
            return KEY_F5;

        case VK_F6:
            return KEY_F6;

        case VK_F7:
            return KEY_F7;

        case VK_F8:
            return KEY_F8;

        case VK_F9:
            return KEY_F9;

        case VK_F10:
            return KEY_F10;

        case VK_F11:
            return KEY_F11;

        case VK_F12:
            return KEY_F12;

        default:
            return (uint8_t)keyboard->virtualKeyToASCII(virtualKey);
    }
}