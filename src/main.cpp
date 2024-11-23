#include <Arduino.h>
#include <WiFi.h>
#include <BleKeyboard.h>
#include <unordered_set>
#include <fabgl.h>

using namespace fabgl;

static bool connected = false;
static BleKeyboard bleKeyboard("ESP32 Keyboard", "Espressif", 100);
static PS2Controller ps2Controller;
static Keyboard* keyboard;

// BLE Keyboard report
static KeyReport report = { 0 }; 

// Pressed keys
std::unordered_set<uint8_t> pressedKeys;

// Used to convert "alt virtual key" back to source virtual key
static std::map<VirtualKey, VirtualKey> _virtualKeyMap;

uint8_t convertToKeyCode(VirtualKey virtualKey);

void setup() 
{
    Serial.begin(115200);
    Serial.println("Starting");

    bleKeyboard.begin();
    ps2Controller.begin(PS2Preset::KeyboardPort0);
    keyboard = ps2Controller.keyboard();
    Serial.println("Started");

	const KeyboardLayout* layout = keyboard->getLayout();
	for (AltVirtualKeyDef keyDef: layout->alternateVK)
	{
		_virtualKeyMap[keyDef.virtualKey] = keyDef.reqVirtualKey;
	}
}

static void updateModifiers(VirtualKey virtualKey, bool keyDown)
{
    switch (virtualKey)
    {
        case VK_LCTRL:
            if (keyDown)
            {
                report.modifiers |= 0x01;
            }
            else
            {
                report.modifiers &= ~0x01; 
            }
            break;
        
        case VK_LSHIFT:
            if (keyDown)
            {
                report.modifiers |= 0x02;
            }
            else
            {
                report.modifiers &= ~0x02; 
            }
            break;
        
        case VK_LALT:
            if (keyDown)
            {
                report.modifiers |= 0x04;
            }
            else
            {
                report.modifiers &= ~0x04; 
            }
            break;
        
        case VK_LGUI:
            if (keyDown)
            {
                report.modifiers |= 0x08;
            }
            else
            {
                report.modifiers &= ~0x08; 
            }
            break;
        
        case VK_RCTRL:
            if (keyDown)
            {
                report.modifiers |= 0x10;
            }
            else
            {
                report.modifiers &= ~0x10; 
            }
            break;
        
        case VK_RSHIFT:
            if (keyDown)
            {
                report.modifiers |= 0x20;
            }
            else
            {
                report.modifiers &= ~0x20; 
            }
            break;
        
        case VK_RALT:
            if (keyDown)
            {
                report.modifiers |= 0x40;
            }
            else
            {
                report.modifiers &= ~0x40; 
            }
            break;
        
        case VK_RGUI:
            if (keyDown)
            {
                report.modifiers |= 0x80;
            }
            else
            {
                report.modifiers &= ~0x80; 
            }
            break;
        
        default:
            break;
    }
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
    updateModifiers(virtualKey, keyDown);

    uint8_t keyCode = convertToKeyCode(virtualKey);
    if (keyCode > 0)
    {
        if (keyDown)
        {
            pressedKeys.insert(keyCode);
            if (pressedKeys.size() > 6)
            {
                // More than 6 keys pressed at once
                // Don't send anything
                pressedKeys.clear();
            }
        }
        else
        {
            pressedKeys.erase(keyCode);
        }

        auto iterator = pressedKeys.begin();
        for (int i = 0; i < 6; i++)
        {
            if (iterator != pressedKeys.end())
            {
                report.keys[i] = *iterator;
                iterator++;
            }
            else
            {
                report.keys[i] = 0;
            }
        }
    }

    bleKeyboard.sendReport(&report);

/*
    char buffer[200];
    const char *bit_rep[16] = {
        [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
        [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
        [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    sprintf(buffer, "%s%s %02x %02x %02x %02x %02x %02x", 
        bit_rep[report.modifiers >> 4], bit_rep[report.modifiers & 0x0F],
        report.keys[0], report.keys[1], report.keys[2],
        report.keys[3], report.keys[4], report.keys[5]);
    Serial.println(buffer);
*/
}

uint8_t convertToKeyCode(VirtualKey virtualKey)
{
    // Get source virtual key
	auto pos = _virtualKeyMap.find(virtualKey);
	if (pos != _virtualKeyMap.end()) 
	{
        virtualKey = pos->second;
	} 

    switch (virtualKey)
    {
        case VK_a...VK_z:
            return 0x04 + (virtualKey - VK_a);
        case VK_A...VK_Z:
            return 0x04 + (virtualKey - VK_A);

        case VK_1...VK_9:
            return 0x1E + (virtualKey - VK_1);
        case VK_0:
            return 0x27;

        case VK_RETURN:
            return KEY_RETURN - 0x88;
        case VK_ESCAPE:
            return KEY_ESC - 0x88;
        case VK_BACKSPACE:
            return KEY_BACKSPACE - 0x88;
        case VK_TAB:
            return KEY_TAB - 0x88;
        case VK_SPACE:
            return 0x2C;
        case VK_MINUS:
        case VK_UNDERSCORE:
            return 0x2D;
        case VK_PLUS:
        case VK_EQUALS:
            return 0x2E;
        case VK_LEFTBRACE:
        case VK_LEFTBRACKET:
            return 0x2F;
        case VK_RIGHTBRACE:
        case VK_RIGHTBRACKET:
            return 0x30;
        case VK_BACKSLASH:
        case VK_VERTICALBAR:
            return 0x31;
        case VK_SEMICOLON:
        case VK_COLON:
            return 0x33;
        case VK_QUOTE:
        case VK_QUOTEDBL:
            return 0x34;
        case VK_GRAVEACCENT:
        case VK_TILDE:
            return 0x35;
        case VK_COMMA:
        case VK_LESS:
            return 0x36;
        case VK_PERIOD:
        case VK_GREATER:
            return 0x37;
        case VK_SLASH:
        case VK_QUESTION:
            return 0x38;
        case VK_CAPSLOCK:
            return 0x39;

        case VK_F1...VK_F12:
            return 0x3A + (virtualKey - VK_F1);

        case VK_SYSREQ:
            return 0x46;
        case VK_SCROLLLOCK:
            return 0x47;
        case VK_PAUSE:
            return 0x48;
        case VK_INSERT:
            return KEY_INSERT - 0x88;
        case VK_HOME:
            return KEY_HOME - 0x88;
        case VK_PAGEUP:
            return KEY_PAGE_UP - 0x88;
        case VK_DELETE:
            return KEY_DELETE - 0x88;
        case VK_END:
            return KEY_END - 0x88;
        case VK_PAGEDOWN:
            return KEY_PAGE_DOWN - 0x88;
        case VK_RIGHT:
            return KEY_RIGHT_ARROW - 0x88;
        case VK_LEFT:
            return KEY_LEFT_ARROW - 0x88;
        case VK_DOWN:
            return KEY_DOWN_ARROW - 0x88;
        case VK_UP:
            return KEY_UP_ARROW - 0x88;

        case VK_NUMLOCK:
            return 0x53;
        case VK_KP_DIVIDE:
            return 0x54;
        case VK_KP_MULTIPLY:
            return 0x55;
        case VK_KP_MINUS:
            return 0x56;
        case VK_KP_PLUS:
            return 0x57;
        case VK_KP_ENTER:
            return 0x58;
        case VK_KP_1...VK_KP_9:
            return 0x59 + (virtualKey - VK_KP_1);
        case VK_KP_0:
            return 0x62;
        case VK_KP_PERIOD:
            return 0x63;

        default:
            return 0;
    }
}
