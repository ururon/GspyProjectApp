#ifndef _VIRTUALKEY_TABLE_H
#define _VIRTUALKEY_TABLE_H



UCHAR VkCodeToHid[0x100] =
{
0,      //0x00
0,      //0x01
0,      //0x02
0,      //0x03
0,      //0x04
0,      //0x05
0,      //0x06
0,      //0x07
0x2a,   //0x08, VK_BACK
0x2b,   //0x09, VK_TAB
0,      //0x0A,
0,      //0x0B,
0,      //0x0C, VK_CLEAR
0x28,   //0x0D, VK_RETURN     /58 ????
0,      //0x0E,
0,      //0x0F,
0,      //0x10,
0,      //0x11,
0,      //0x12,
0x48,   //0x13, VK_PAUSE
0x39,   //0x14, VK_CAPITAL
0,      //0x15,
0,      //0x16,
0,      //0x17,
0,      //0x18,
0,      //0x19,
0,      //0x1A,
0x29,   //0x1B, VK_ESCAPE
0,      //0x1C,
0,      //0x1D,
0,      //0x1E,
0,      //0x1F,
0x2c,   //0x20, VK_SPACE
0x4b,   //0x21, VK_PRIOR   pgup
0x4e,   //0x22, VK_NEXT    pgdn
0x4d,   //0x23, VK_END
0x4a,   //0x24, VK_HOME
0x50,   //0x25, VK_LEFT
0x52,   //0x26, VK_UP
0x4f,   //0x27, VK_RIGHT
0x51,   //0x28, VK_DOWN
0,      //0x29,
0,      //0x2A
0,		//0x2B,

0x46,   //0x2C, VK_PRINT
0x49,   //0x2D, VK_INSERT
0x4c,   //0x2E, VK_DELETE
0,      //0x2F,
0x27,   //0x30,  0
0x1e,   //0x31,  1
0x1f,   //0x32,  2
0x20,   //0x33,  3
0x21,   //0x34,  4
0x22,   //0x35,  5
0x23,   //0x36,  6
0x24,   //0x37,  7
0x25,   //0x38,  8
0x26,   //0x39,  9
0,      //0x3A,
0,      //0x3B,
0,      //0x3C,
0,      //0x3D,
0,      //0x3E,
0,      //0x3F,
0,      //0x40,
0x04,   //0x41,   a
0x05,   //0x42,   b
0x06,   //0x43,   c
0x07,   //0x44,   d
0x08,   //0x45,   e
0x09,   //0x46,   f
0x0a,   //0x47,   g
0x0b,   //0x48,   h
0x0c,   //0x49,   i
0x0d,   //0x4A,   j
0x0e,   //0x4B,   k
0x0f,   //0x4C,   l
0x10,   //0x4D,   m
0x11,   //0x4E,   n
0x12,   //0x4F,   o
0x13,   //0x50,   p
0x14,   //0x51,   q
0x15,   //0x52,   r
0x16,   //0x53,   s
0x17,   //0x54,   t
0x18,   //0x55,   u
0x19,   //0x56,   v
0x1a,   //0x57,   w
0x1b,   //0x58,   x
0x1c,   //0x59,   y
0x1d,   //0x5A,   z
0,      //0x5B,
0,      //0x5C,
0,      //0x5D,
0,      //0x5E,
0,      //0x5F,
0x62,   //0x60,VK_NUMPAD0
0x59,   //0x61,VK_NUMPAD1
0x5a,   //0x62,VK_NUMPAD2
0x5b,   //0x63,VK_NUMPAD3
0x5c,   //0x64,VK_NUMPAD4
0x5d,   //0x65,VK_NUMPAD5
0x5e,   //0x66,VK_NUMPAD6
0x5f,   //0x67,VK_NUMPAD7
0x60,   //0x68,VK_NUMPAD8
0x61,   //0x69,VK_NUMPAD9
0x55,   //0x6A,VK_MULTIPLY
0x57,   //0x6B,VK_ADD
0,      //0x6C,
0x56,   //0x6D,VK_SUBTRACT
0x63,   //0x6E,VK_DECIMAL
0x54,   //0x6F,VK_DIVIDE
0x3a,   //0x70,VK_F1
0x3b,   //0x71,VK_F2
0x3c,   //0x72,VK_F3
0x3d,   //0x73,VK_F4
0x3e,   //0x74,VK_F5
0x3f,   //0x75,VK_F6
0x40,   //0x76,VK_F7
0x41,   //0x77,VK_F8
0x42,   //0x78,VK_F9
0x43,   //0x79,VK_F10
0x44,   //0x7A,VK_F11
0x45,   //0x7B,VK_F12
0,      //0x7C,
0,      //0x7D,
0,      //0x7E,
0,      //0x7F,
0,      //0x70,
0,      //0x81,
0,      //0x82,
0,      //0x83,
0,      //0x84,
0,      //0x85,
0,      //0x86,
0,      //0x87,
0,      //0x88,
0,      //0x89,
0,      //0x8A,
0,      //0x8B,
0,      //0x8C,
0,      //0x8D,
0,      //0x8E,
0,      //0x8F,
0x53,   //0x90,VK_NUMLOCK
0x47,   //0x91,VK_SCROLL
0,      //0x92,
0,      //0x93,
0,      //0x94,
0,      //0x95,
0,      //0x96,
0,      //0x97,
0,      //0x98,
0,      //0x99,
0,      //0x9A,
0,      //0x9B,
0,      //0x9C,
0,      //0x9D,
0,      //0x9E,
0,      //0x9F,
0xe1,   //0xA0,VK_LSHIFT
0xe5,   //0xA1,VK_RSHIFT
0xe0,   //0xA2,VK_LCONTROL
0xe4,   //0xA3,VK_RCONTROL
0xe2,   //0xA4 VK_LALT
0xe6,   //0xA5 VK_RALT
0,      //0xA6
0,      //0xA7
0,      //0xA8
0,      //0xA9
0,      //0xAA
0,      //0xAB
0,      //0xAC
0,      //0xAD
0,      //0xAE
0,      //0xAF

0,      //0xB0
0,      //0xB1
0,      //0xB2
0,      //0xB3
0,      //0xB4
0,      //0xB5
0,      //0xB6
0,      //0xB7
0,      //0xB8
0,      //0xB9
0x33,   //0xBA ;:
0x2e,   //0xBB +=
0x36,   //0xBC ,<
0x2d,   //0xBD  -_
0x37,   //0xBE .>
0x38,   //0xBF  /?

0x35,    //0xC0, `~
0,       //0xC1
0,       //0xC2
0,       //0xC3
0,       //0xC4
0,       //0xC5
0,       //0xC6
0,       //0xC7
0,       //0xC8
0,       //0xC9
0,       //0xCA
0,       //0xCB
0,       //0xCC
0,       //0xCD
0,       //0xCE
0,       //0xCF

0,       //0xD0
0,       //0xD1
0,       //0xD2
0,       //0xD3
0,       //0xD4
0,       //0xD5
0,       //0xD6
0,       //0xD7
0,       //0xD8
0,       //0xD9
0,       //0xDA
0x2f,    //0xDB  [{
0x31,    //0xDC  \|
0x30,    //0xDD  ]}
0x34,    //0xDE  '"
0,       //0xDF

0,       //0xE0
0,       //0xE1
0,       //0xE2
0,       //0xE3
0,       //0xE4
0,       //0xE5
0,       //0xE6
0,       //0xE7
0,       //0xE8
0,       //0xE9
0,       //0xEA
0,       //0xEB
0,       //0xEC
0,       //0xED
0,       //0xEE
0,       //0xEF

0,       //0xF0
0,       //0xF1
0,       //0xF2
0,       //0xF3
0,       //0xF4
0,       //0xF5
0,       //0xF6
0,       //0xF7
0,       //0xF8
0,       //0xF9
0,       //0xFA
0,       //0xFB
0,       //0xFC
0,       //0xFD
0,       //0xFE
0xff     //0xFF
};

////////////////////////////////////////////////////////////////////////////////////////
UCHAR HIDtoVkCode[0x100] =
{
0,        //0x00  NO KEY
0,        //0x01  NO KEY
0,        //0x02  NO KEY
0,        //0x03  NO KEY
0x41,     //0x04, a
0x42,     //0x05, b
0x43,     //0x06, c
0x44,     //0x07, d
0x45,     //0x08, e
0x46,     //0x09, f
0x47,     //0x0A, g
0x48,     //0x0B, h
0x49,     //0x0C, i
0x4a,     //0x0D, j
0x4b,     //0x0E, k
0x4c,     //0x0F, l
0x4d,     //0x10, m
0x4e,     //0x11, n
0x4f,     //0x12, o
0x50,     //0x13, p
0x51,     //0x14, q
0x52,     //0x15, r
0x53,     //0x16, s
0x54,     //0x17, t
0x55,     //0x18, u
0x56,     //0x19, v
0x57,     //0x1A, w
0x58,     //0x1B, x
0x59,     //0x1C, y
0x5a,     //0x1D, z
0x31,     //0x1E, 1
0x32,     //0x1F, 2
0x33,     //0x20, 3
0x34,     //0x21, 4
0x35,     //0x22, 5
0x36,     //0x23, 6
0x37,     //0x24, 7
0x38,     //0x25, 8
0x39,     //0x26, 9
0x30,     //0x27, 0
0x0d,     //0x28, ret
0x1b,     //0x29, esc
0x08,     //0x2A, backspace
0x09,	  //0x2B, tab

0x20,     //0x2C, space
0xbd,     //0x2D, -_
0xbb,     //0x2E, +=
0xdb,     //0x2F, [{
0xdd,     //0x30, ]}
0xdc,     //0x31, \|
0 ,       //0x32,  NO KEY
0xba,     //0x33, ;:
0xde,     //0x34, '"
0xc0,     //0x35, `~
0xbc,     //0x36, ,<
0xbe,     //0x37, .>
0xbf,     //0x38, /?
0x14,     //0x39, Cap lk
0x70,     //0x3A, F1
0x71,     //0x3B, F2
0x72,     //0x3C, F3
0x73,     //0x3D, F4
0x74,     //0x3E, F5
0x75,     //0x3F, F6
0x76,     //0x40, F7
0x77,     //0x41, F8
0x78,     //0x42, F9
0x79,     //0x43, F10
0x7a,     //0x44, F11
0x7b,     //0x45, F12
0x2c,     //0x46, Prt Scr
0x91,     //0x47, Scroll Lk
0x13,     //0x48, break pause
0x2d,     //0x49, Ins
0x24,     //0x4A, Home
0x21,     //0x4B, PgUP
0x2e,     //0x4C, del
0x23,     //0x4D, end
0x22,     //0x4E, PgDn
0x27,     //0x4F, Right Arrow
0x25,     //0x50, Left Arrow
0x28,     //0x51, Down Arrow
0x26,     //0x52, Up Arrow
0x90,     //0x53, Num Lk
0x6f,     //0x54, Pad /
0x6a,     //0x55, Pad *
0x6d,     //0x56, Pad -
0x6b,     //0x57, Pad +
0x0d,     //0x58, Pad Enter
0x61,     //0x59, Pad 1
0x62,     //0x5A, Pad 2
0x63,     //0x5B, Pad 3
0x64,     //0x5C, Pad 4
0x65,     //0x5D, Pad 5
0x66,     //0x5E, Pad 6
0x67,      //0x5F, Pad 7
0x68,     //0x60, Pad 8
0x69,     //0x61, Pad 9
0x60,     //0x62, Pad 0
0x6e,     //0x63, Pad Del
0,        //0x64,
0,        //0x65,
0,        //0x66,
0,        //0x67,
0,        //0x68,
0,        //0x69,
0,        //0x6A,
0,        //0x6B,
0,         //0x6C,
0,        //0x6D,
0,        //0x6E,
0,        //0x6F,
0,        //0x70,
0,        //0x71,
0,       //0x72,
0,        //0x73,
0,        //0x74,
0,        //0x75,
0,        //0x76,
0,        //0x77,
0,        //0x78,
0,        //0x79,
0,        //0x7A,
0,        //0x7B,
0,        //0x7C,
0,        //0x7D,
0,        //0x7E,
0,        //0x7F,
0,        //0x70,
0,        //0x81,
0,        //0x82,
0,        //0x83,
0,        //0x84,
0,        //0x85,
0,        //0x86,
0,        //0x87,
0,        //0x88,
0,        //0x89,
0,        //0x8A,
0,        //0x8B,
0,        //0x8C,
0,        //0x8D,
0,        //0x8E,
0,        //0x8F,
0,        //0x90,
0,        //0x91,
0,        //0x92,
0,        //0x93,
0,        //0x94,
0,        //0x95,
0,        //0x96,
0,        //0x97,
0,        //0x98,
0,        //0x99,
0,        //0x9A,
0,        //0x9B,
0,        //0x9C,
0,        //0x9D,
0,        //0x9E,
0,        //0x9F,
0,        //0xA0,
0,        //0xA1,
0,        //0xA2,
0,        //0xA3,
0,        //0xA4
0,        //0xA5
0,        //0xA6
0,        //0xA7
0,        //0xA8
0,        //0xA9
0,        //0xAA
0,        //0xAB
0,        //0xAC
0,        //0xAD
0,        //0xAE
0,        //0xAF

0,        //0xB0
0,        //0xB1
0,        //0xB2
0,        //0xB3
0,        //0xB4
0,        //0xB5
0,        //0xB6
0,        //0xB7
0,        //0xB8
0,        //0xB9
0,        //0xBA
0,        //0xBB
0,        //0xBC
0,        //0xBD
0,        //0xBE
0,        //0xBF

0,        //0xC0
0,        //0xC1
0,        //0xC2
0,        //0xC3
0,        //0xC4
0,        //0xC5
0,        //0xC6
0,        //0xC7
0,        //0xC8
0,        //0xC9
0,        //0xCA
0,        //0xCB
0,        //0xCC
0,        //0xCD
0,        //0xCE
0,        //0xCF

0,        //0xD0
0,        //0xD1
0,        //0xD2
0,        //0xD3
0,        //0xD4
0,        //0xD5
0,        //0xD6
0,        //0xD7
0,        //0xD8
0,        //0xD9
0,        //0xDA
0,        //0xDB
0,        //0xDC
0,        //0xDD
0,        //0xDE
0,        //0xDF

0xa2,     //0xE0, Left Ctrl
0xa0,     //0xE1, Left Shift
0xa4,     //0xE2, Left Alt
0,        //0xE3,
0xa3,     //0xE4, Right Control
0xa1,     //0xE5, Right Shift
0xa5,     //0xE6, Right Alt
0,        //0xE7
0,        //0xE8
0,        //0xE9
0,        //0xEA
0,        //0xEB
0,        //0xEC
0,        //0xED
0,        //0xEE
0,        //0xEF

0,        //0xF0
0,        //0xF1
0,        //0xF2
0,        //0xF3
0,        //0xF4
0,        //0xF5
0,        //0xF6
0,        //0xF7
0,        //0xF8
0,        //0xF9
0,        //0xFA
0,        //0xFB
0,        //0xFC
0,        //0xFD
0,        //0xFE
0xff      //0xFF
};




#endif //_VIRTUALKEY_TABLE_H
