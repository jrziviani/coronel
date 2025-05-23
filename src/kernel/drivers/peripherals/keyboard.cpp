#include "keyboard.hpp"
#include "arch/iarch.hpp"
#include "arch/amd64/instructions.hpp"
#include "libs/stdint.hpp"

constexpr uint16_t DATA_IN_BUFFER = 0x01;
constexpr uint16_t STATUS_PORT = 0x64;
constexpr uint16_t DATA_PORT = 0x60;

#define KEY_CODES                                         \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_ESC,          KEY_ESC,          '\033', '\033') \
    X(KEY_1,            KEY_EXCLAMATION,  '1',    '!')    \
    X(KEY_2,            KEY_AT,           '2',    '@')    \
    X(KEY_3,            KEY_HASH,         '3',    '#')    \
    X(KEY_4,            KEY_DOLLAR,       '4',    '$')    \
    X(KEY_5,            KEY_PERCENT,      '5',    '%')    \
    X(KEY_6,            KEY_CIRCUMFLEX,   '6',    '^')    \
    X(KEY_7,            KEY_AMPERSAND,    '7',    '&')    \
    X(KEY_8,            KEY_ASTERISK,     '8',    '*')    \
    X(KEY_9,            KEY_LEFTPARENT,   '9',    '(')    \
    X(KEY_0,            KEY_RIGHTPARENT,  '0',    ')')    \
    X(KEY_MINUS,        KEY_UNDERSCORE,   '-',    '_')    \
    X(KEY_EQUAL,        KEY_PLUS,         '=',    '+')    \
    X(KEY_BACKSPACE,    KEY_BACKSPACE,    0x8,    0x8)    \
    X(KEY_TAB,          KEY_TAB,          '\t',   '\t')   \
    X(KEY_Q,            KEY_Q,            'q',    'Q')    \
    X(KEY_W,            KEY_W,            'w',    'W')    \
    X(KEY_E,            KEY_E,            'e',    'E')    \
    X(KEY_R,            KEY_R,            'r',    'R')    \
    X(KEY_T,            KEY_T,            't',    'T')    \
    X(KEY_Y,            KEY_Y,            'y',    'Y')    \
    X(KEY_U,            KEY_U,            'u',    'U')    \
    X(KEY_I,            KEY_I,            'i',    'I')    \
    X(KEY_O,            KEY_O,            'o',    'O')    \
    X(KEY_P,            KEY_P,            'p',    'P')    \
    X(KEY_LEFTBRACKET,  KEY_LEFTBRACE,    '[',    '{')    \
    X(KEY_RIGHTBRACKET, KEY_RIGHTBRACE,   ']',    '}')    \
    X(KEY_RETURN,       KEY_RETURN,       '\n',   '\n')   \
    X(KEY_CONTROL,      KEY_CONTROL,      0,      0)      \
    X(KEY_A,            KEY_A,            'a',    'A')    \
    X(KEY_S,            KEY_S,            's',    'S')    \
    X(KEY_D,            KEY_D,            'd',    'D')    \
    X(KEY_F,            KEY_F,            'f',    'F')    \
    X(KEY_G,            KEY_G,            'g',    'G')    \
    X(KEY_H,            KEY_H,            'h',    'H')    \
    X(KEY_J,            KEY_J,            'j',    'J')    \
    X(KEY_K,            KEY_K,            'k',    'K')    \
    X(KEY_L,            KEY_L,            'l',    'L')    \
    X(KEY_SEMICOLON,    KEY_COLON,        ';',    ':')    \
    X(KEY_APOSTROPHE,   KEY_DOUBLEQUOTE,  '\'',   '"')    \
    X(KEY_BACKTICK,     KEY_TILDE,        '`',    '~')    \
    X(KEY_LEFTSHIFT,    KEY_LEFTSHIFT,    0,      0)      \
    X(KEY_BACKSLASH,    KEY_PIPE,         '\\',   '|')    \
    X(KEY_Z,            KEY_Z,            'z',    'Z')    \
    X(KEY_X,            KEY_X,            'x',    'X')    \
    X(KEY_C,            KEY_C,            'c',    'C')    \
    X(KEY_V,            KEY_V,            'v',    'V')    \
    X(KEY_B,            KEY_B,            'b',    'B')    \
    X(KEY_N,            KEY_N,            'n',    'N')    \
    X(KEY_M,            KEY_M,            'm',    'M')    \
    X(KEY_COMMA,        KEY_LESSTHAN,     ',',    '<')    \
    X(KEY_PERIOD,       KEY_GREATERTHAN,  '.',    '>')    \
    X(KEY_SLASH,        KEY_QUESTION,     '/',    '?')    \
    X(KEY_RIGHTSHIFT,   KEY_RIGHTSHIFT,   0,      0)      \
    X(KEY_ASTERISK,     KEY_ASTERISK,     '*',    '*')    \
    X(KEY_ALT,          KEY_ALT,          0,      0)      \
    X(KEY_SPACE,        KEY_SPACE,        ' ',    ' ')    \
    X(KEY_CAPSLOCK,     KEY_CAPSLOCK,     0,      0)      \
    X(KEY_F1,           KEY_F1,           0,      0)      \
    X(KEY_F2,           KEY_F2,           0,      0)      \
    X(KEY_F3,           KEY_F3,           0,      0)      \
    X(KEY_F4,           KEY_F4,           0,      0)      \
    X(KEY_F5,           KEY_F5,           0,      0)      \
    X(KEY_F6,           KEY_F6,           0,      0)      \
    X(KEY_F7,           KEY_F7,           0,      0)      \
    X(KEY_F9,           KEY_F9,           0,      0)      \
    X(KEY_F10,          KEY_F10,          0,      0)      \
    X(KEY_NUMLOCK,      KEY_NUMLOCK,      0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_HOME,         KEY_HOME,         0,      0)      \
    X(KEY_UP,           KEY_UP,           0,      0)      \
    X(KEY_PAGEUP,       KEY_PAGEUP,       0,      0)      \
    X(KEY_MINUS,        KEY_MINUS,        '-',    '-')    \
    X(KEY_LEFT,         KEY_LEFT,         0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_RIGHT,        KEY_RIGHT,        0,      0)      \
    X(KEY_PLUS,         KEY_PLUS,         '+',    '+')    \
    X(KEY_END,          KEY_END,          0,      0)      \
    X(KEY_DOWN,         KEY_DOWN,         0,      0)      \
    X(KEY_PAGEDOWN,     KEY_PAGEDOWN,     0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_DELETE,       KEY_DELETE,       0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_BACKSLASH,    KEY_PIPE,        '\\',   '|')     \
    X(KEY_F11,          KEY_F11,          0,      0)      \
    X(KEY_F12,          KEY_F12,          0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_INVALID,      KEY_INVALID,      0,      0)      \
    X(KEY_LOGO,         KEY_LOGO,         0,      0)

static const char en_US[] = {
#define X(name, shift_name, code, shift_code) code,
    KEY_CODES
#undef X
};

static const char en_US_shift[] = {
#define X(name, shift_name, code, shift_code) shift_code,
    KEY_CODES
#undef X
};

constexpr char SHIFT  = 0x1;
constexpr char CTRL   = 0x2;
constexpr char ALT    = 0x4;
constexpr char CAPS   = 0x8;
constexpr char NUM    = 0x10;
constexpr char SCROLL = 0x20;
constexpr char ESC    = 0x40;

peripherals::keyboard::keyboard(iarch *arch) : arch_(arch), shift_(false)
{
}

void peripherals::keyboard::on_keyboard(const interrupt_t &)
{
    auto status = insn::inb(STATUS_PORT);
    if ((status & DATA_IN_BUFFER) == 0) {
        return;
    }

    auto data = insn::inb(DATA_PORT);

    if (data & 0x80) {
        on_keyup(data);
    }
    else {
        on_keydown(data);
    }
}

void peripherals::keyboard::on_keyup(unsigned char c)
{
    switch (c) {
        case 0xaa: // left shift release
        case 0xb6: // right shift release
            shift_ = false;
            return;
    }
}

void peripherals::keyboard::on_keydown(unsigned char c)
{
    switch (c) {
        case 0x2a: // left shift
        case 0x36: // right shift
            shift_ = true;
            return;
    }

    if (shift_) {
        arch_->get_video()->printc(en_US_shift[c]);
    }
    else {
        arch_->get_video()->printc(en_US[c]);
    }
}

peripherals::keyboard &peripherals::add_keyboard(iarch *arch)
{
    static peripherals::keyboard instance(arch);
    static keyboard_handler_t handler(instance, &peripherals::keyboard::on_keyboard);

    arch->set_keyboard_handler(&handler);

    return instance;
}