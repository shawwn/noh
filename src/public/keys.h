// (C)2005 S2 Games
// keys.h
//
//=============================================================================
#ifndef KEYS_H
#define KEYS_H

//=============================================================================
// Headers
//=============================================================================
#include <ctype.h>
//=============================================================================

enum keyboard_e
{
    KEY_TAB =       9,
    KEY_RETURN =    10,
    KEY_ENTER =     13,
    KEY_ESCAPE =    27,
    KEY_SPACE =     32,
    KEY_BACKSPACE = 256,
    KEY_UP,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_ALT,
    KEY_CTRL,
    KEY_SHIFT,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_INS,
    KEY_DEL,
    KEY_PGDN,
    KEY_PGUP,
    KEY_HOME,
    KEY_END,
    KEY_PAUSE,

    KEY_LBUTTON =       512,
    KEY_MOUSEFIRSTBUTTON =  KEY_LBUTTON,
    KEY_RBUTTON,
    KEY_MBUTTON,
    KEY_MWHEELUP,
    KEY_MWHEELDOWN,
    KEY_BUTTON4,
    KEY_BUTTON5,
    KEY_BUTTON6,
    KEY_BUTTON7,
    KEY_BUTTON8,
    KEY_BUTTON9,
    KEY_MOUSELASTBUTTON =   KEY_BUTTON9,
    KEY_KEYPAD_0,
    KEY_KEYPAD_1,
    KEY_KEYPAD_2,
    KEY_KEYPAD_3,
    KEY_KEYPAD_4,
    KEY_KEYPAD_5,
    KEY_KEYPAD_6,
    KEY_KEYPAD_7,
    KEY_KEYPAD_8,
    KEY_KEYPAD_9,
    KEY_KEYPAD_INS,
    KEY_KEYPAD_DEL,
    KEY_KEYPAD_PLUS,
    KEY_KEYPAD_MINUS,
    KEY_KEYPAD_ENTER,
    KEY_KEYPAD_SLASH,
    KEY_KEYPAD_ASTERISK,
    KEY_LEFT_WINDOWS,
    KEY_RIGHT_WINDOWS,
    KEY_MENU,
    KEY_KEYPAD_DECIMAL,
    KEY_NUMLOCK,
    KEY_SCROLLLOCK,
    KEY_CAPSLOCK,
    KEY_APP,
    KEY_XBUTTON1,
    KEY_XBUTTON2,

    MAX_KEYS,
};


enum Key
{
    eKeyInvalid         = -1,

    eKeyTab             = KEY_TAB,
    eKeyReturn          = KEY_RETURN,
    eKeyEnter           = KEY_ENTER,
    eKeyEscape          = KEY_ESCAPE,
    eKeySpace           = KEY_SPACE,
    eKeyBackspace       = KEY_BACKSPACE,
    eKeyUp              = KEY_UP,
    eKeyLeft            = KEY_LEFT,
    eKeyRight           = KEY_RIGHT,
    eKeyDown            = KEY_DOWN,
    eKeyAlt             = KEY_ALT,
    eKeyCtrl            = KEY_CTRL,
    eKeyShift           = KEY_SHIFT,
    eKeyF1              = KEY_F1,
    eKeyF2              = KEY_F2,
    eKeyF3              = KEY_F3,
    eKeyF4              = KEY_F4,
    eKeyF5              = KEY_F5,
    eKeyF6              = KEY_F6,
    eKeyF7              = KEY_F7,
    eKeyF8              = KEY_F8,
    eKeyF9              = KEY_F9,
    eKeyF10             = KEY_F10,
    eKeyF11             = KEY_F11,
    eKeyF12             = KEY_F12,
    eKeyInsert          = KEY_INS,
    eKeyDelete          = KEY_DEL,
    eKeyPageUp          = KEY_PGDN,
    eKeyPageDown        = KEY_PGUP,
    eKeyHome            = KEY_HOME,
    eKeyEnd             = KEY_END,
    eKeyPause           = KEY_PAUSE,

    eKeyLButton         = KEY_LBUTTON,
    eKeyFirstButton     = KEY_MOUSEFIRSTBUTTON,
    eKeyRButton         = KEY_RBUTTON,
    eKeyMButton         = KEY_MBUTTON,
    eKeyWheelUp         = KEY_MWHEELUP,
    eKeyWheelDown       = KEY_MWHEELDOWN,
    eKeyButton4         = KEY_BUTTON4,
    eKeyButton5         = KEY_BUTTON5,
    eKeyButton6         = KEY_BUTTON6,
    eKeyButton7         = KEY_BUTTON7,
    eKeyButton8         = KEY_BUTTON8,
    eKeyButton9         = KEY_BUTTON9,
    eKeyLastButton      = KEY_MOUSELASTBUTTON,

    eKeyKeypad0         = KEY_KEYPAD_0,
    eKeyKeypad1         = KEY_KEYPAD_1,
    eKeyKeypad2         = KEY_KEYPAD_2,
    eKeyKeypad3         = KEY_KEYPAD_3,
    eKeyKeypad4         = KEY_KEYPAD_4,
    eKeyKeypad5         = KEY_KEYPAD_5,
    eKeyKeypad6         = KEY_KEYPAD_6,
    eKeyKeypad7         = KEY_KEYPAD_7,
    eKeyKeypad8         = KEY_KEYPAD_8,
    eKeyKeypad9         = KEY_KEYPAD_9,
    eKeyKeypadInsert    = KEY_KEYPAD_INS,
    eKeyKeypadDelete    = KEY_KEYPAD_DEL,
    eKeyKeypadPlus      = KEY_KEYPAD_PLUS,
    eKeyKeypadMinus     = KEY_KEYPAD_MINUS,
    eKeyKeypadEnter     = KEY_KEYPAD_ENTER,
    eKeyKeypadSlash     = KEY_KEYPAD_SLASH,
    eKeyKeypadAsterisk  = KEY_KEYPAD_ASTERISK,
    eKeyLeftWindows     = KEY_LEFT_WINDOWS,
    eKeyRightWindows    = KEY_RIGHT_WINDOWS,
    eKeyMenu            = KEY_MENU,
    eKeyKeypadDecimal   = KEY_KEYPAD_DECIMAL,
    eKeyNumLock         = KEY_NUMLOCK,
    eKeyScrollLock      = KEY_SCROLLLOCK,
    eKeyCapsLock        = KEY_CAPSLOCK,
    eKeyApp             = KEY_APP,

/*  eKey                = KEY_XBUTTON1
    eKey                = KEY_XBUTTON2
*/
};

inline bool IsKey(int key)
{
    switch ((Key)key )
    {
        case eKeyTab:
        case eKeyReturn:
        case eKeyEnter:
        case eKeyEscape:
        case eKeySpace:
        case eKeyBackspace:
        case eKeyUp:
        case eKeyLeft:
        case eKeyRight:
        case eKeyDown:
        case eKeyAlt:
        case eKeyCtrl:
        case eKeyShift:
        case eKeyF1:
        case eKeyF2:
        case eKeyF3:
        case eKeyF4:
        case eKeyF5:
        case eKeyF6:
        case eKeyF7:
        case eKeyF8:
        case eKeyF9:
        case eKeyF10:
        case eKeyF11:
        case eKeyF12:
        case eKeyInsert:
        case eKeyDelete:
        case eKeyPageUp:
        case eKeyPageDown:
        case eKeyHome:
        case eKeyEnd:
        case eKeyPause:
        case eKeyKeypad0:
        case eKeyKeypad1:
        case eKeyKeypad2:
        case eKeyKeypad3:
        case eKeyKeypad4:
        case eKeyKeypad5:
        case eKeyKeypad6:
        case eKeyKeypad7:
        case eKeyKeypad8:
        case eKeyKeypad9:
        case eKeyKeypadInsert:
        case eKeyKeypadDelete:
        case eKeyKeypadPlus:
        case eKeyKeypadMinus:
        case eKeyKeypadEnter:
        case eKeyKeypadSlash:
        case eKeyKeypadAsterisk:
        case eKeyLeftWindows:
        case eKeyRightWindows:
        case eKeyMenu:
        case eKeyKeypadDecimal:
        case eKeyNumLock:
        case eKeyScrollLock:
        case eKeyCapsLock:
        case eKeyApp:
        case eKeyLButton:
        case eKeyRButton:
        case eKeyMButton:
        case eKeyWheelUp:
        case eKeyWheelDown:
        case eKeyButton4:
        case eKeyButton5:
        case eKeyButton6:
        case eKeyButton7:
        case eKeyButton8:
        case eKeyButton9:
            return true;
    }

    if ( isgraph(key) || (key==' ')) return true;

    return false;
}

inline Key MapKey(int key, int rawchar)
{
    return IsKey(key) ? (Key)key : (IsKey(rawchar) ? (Key)rawchar : eKeyInvalid);
}

#endif
