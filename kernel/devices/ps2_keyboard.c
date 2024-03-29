#include "ps2_keyboard.h"

// Map code to corresponding character
const char* set1_scancodes[] = { 
                            "", "<ESC>", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                            "-", "=", "\b \b", "\t", "q", "w", "e", "r", "t", "y", "u", "i", 
                            "o", "p", "[", "]", "\n", "^", "a", "s", "d", "f", "g", "h", 
                            "j", "k", "l", ";", "'", "`", "<L-SHIFT>", "\\", "z", "x", "c", "v", 
                            "b", "n", "m", ",", ".", "/", "<R-SHIFT>", "*", "<L-ALT>", " ", "<C-LOCK>", "<F1>", "<F2>",
                            "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<N-LOCK>", "<S-LOCK>", "7", "8",
                            "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "",
                            "", "", "<F11>", "<F12>"
                        };

const char* shift_set1_scancodes[] = { 
                            "", "", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
                            "_", "+", "\b \b", "\t", "Q", "W", "E", "R", "T", "Y", "U", "I", 
                            "O", "P", "{", "}", "\n", "^", "A", "S", "D", "F", "G", "H", 
                            "J", "K", "L", ":", "\"", "~", "", "|", "Z", "X", "C", "V", 
                            "B", "N", "M", "<", ">", "?", "", "*", "", " ", "", "", "",
                            "", "", "", "", "", "", "", "", "", "", "", "",
                            "", "", "", "", "", "", "", "", "", "", "", "",
                            "", "", "", ""
                        };

// TODO: Finish adding scancodes
const char* set2_scancodes[] = { 
                            "", "<F9>", "", "<F5>", "<F3>", "<F1>", "<F2>", "<F12>", "", "<F10>", "<F8>", "<F6>",
                            "<F4>", "\t", "`", "", "", "<L-ALT>", "<L-SHIFT>", "", "<L-CTRL>", "q", "1", "", 
                            "", "", "z", "s", "a", "w", "2", "", "", "c", "x", "d", 
                            "e", "4", "3", "", "", " ", "v", "f", "t", "r", "5", "", 
                            "g", "y", "6", "", "", "", "m", "j", "u", "7", "8", "", "",
                            "", ",", "k", "i", "o", "0", "9", "", ".", "/", "l", ";",
                            "p", "-", "", "", "", "'", "", "[", "=", "", "", "<C-LOCK>",
                            "<R-SHIFT>", "\n", "]", "", "\\"
                        };
