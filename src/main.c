/*
 *--------------------------------------
 * Program Name: HelMmii for TI-84 Plus CE
 * Author: Techflash, Tech64, SuperbeeLavaman
 * License: GNU General Public License version 2
 * Description: Port of Wii-Linux HelpMii for TI-84 Plus CE
 *--------------------------------------
*/

#include <string.h>
#include <ti/getcsc.h>
#include <ti/screen.h>
#include <sys/lcd.h>

const uint16_t color_blue   = 0x001F;
const uint16_t color_lblue  = 0x0FFF;
const uint16_t color_grey   = 0b1110011100111100;
const uint16_t color_white  = 0xFFFF;
const uint16_t color_black  = 0x0000;
const uint16_t color_red    = 0xF800;
const uint16_t color_yellow = 0xFFE0;


uint16_t* vram = (uint16_t*)lcd_Ram;

void drawRect(uint16_t bgcolour, int x1, int y1, int x2, int y2) {
    int y = y1;
    while (y < y2) {
        int linestart = LCD_WIDTH * y + x1;
        vram[linestart] = bgcolour;
        memcpy(vram+1+linestart, vram+linestart, 2*(x2-x1)-2);
        y++;
    }
}

const int font_height = 13;
const int font_width  = 10;
const int grid_height = 18;
const int grid_width  = 32;
void displayChar(char* text, uint16_t bgcolour, uint16_t fgcolour, int posx, int posy) {
    os_SetDrawBGColor(bgcolour);
    os_SetDrawFGColor(fgcolour);
    uint8_t kerning = (font_width - os_FontGetWidth(text)) / 2;
    if (kerning >= font_width) { kerning = 0; }
    os_FontDrawText(text, posx*font_width + kerning, posy*font_height);
}

void displayString(char* string, uint16_t bgcolour, uint16_t fgcolour, int posx, int posy) {
    size_t i = 0; 
    char character[] = {0,0};
    while (string[i]) {
        character[0] = string[i];
        displayChar(character, bgcolour, fgcolour, posx+i, posy);
        i++;
    }

}

void dialogBG(char* title, char* version, char* caption) {
    drawRect(color_blue, 0, 0, 320, 240);
    displayString(title, color_blue, color_lblue, 0, 0);
    displayString(version, color_blue, color_lblue, 28, 0);
    displayString(caption, color_blue, color_lblue, 0, 1);
    drawRect(color_lblue, 2, 28, 318, 30);
}

void dialogFG(int heighttiles, int widthtiles, char* title, int shadowdist) {
    int titlelength = strlen(title);
    int height = heighttiles * font_height;
    int width  = widthtiles  * font_width;
    int cornerx = (LCD_WIDTH  - width  - shadowdist) / 2;
    int cornery = (LCD_HEIGHT - height - shadowdist) / 2;
    drawRect(color_black, cornerx + shadowdist, cornery + shadowdist + 39, LCD_WIDTH - cornerx, LCD_HEIGHT - cornery);
    drawRect(color_grey, cornerx, cornery + 39, LCD_WIDTH - cornerx - shadowdist, LCD_HEIGHT - cornery - shadowdist);
    int textx = (grid_width  - titlelength ) / 2;
    int texty = (grid_height - heighttiles) / 2 + 3;
    displayString(title, color_grey, color_blue, textx, texty);
}

int selected = 1;
bool okCancel = true;
bool update = true;
bool go = false;
int key = 0;
int scrolllimit = 0;
bool justText = 0;

void option(int id, char* name, int posx, int posy) {
    int fgcolour;
    int bgcolour;
    if (id == selected) {
        fgcolour = color_white;
        bgcolour = color_blue;
    } else {
        fgcolour = color_black;
        bgcolour = color_grey;
    }
    char character[] = {0,0};
    character[0] = "0123456789"[id];
    drawRect(bgcolour, posx*font_width, posy*font_height, (posx + strlen(name) + 2)*font_width, (posy + 1)*font_height);
    displayChar(character, bgcolour, color_red, posx, posy);
    displayString(name, bgcolour, fgcolour, posx + 2, posy);
}

void textLine(int id, char* name, int posx, int posy, int maxpage, int maxwidth, uint16_t bg, uint16_t fg) {
    if ((id >= selected) && (id - selected <= maxpage)) {
        drawRect(bg, posx*font_width, (posy + id - selected)*font_height, (posx + maxwidth)*font_width, (posy + id - selected + 1)*font_height);
        displayString(name, bg, fg, posx, posy + id - selected);
    }
}

void okCancelButton(bool isok, int posx, int posy) {
    uint16_t fgcolour;
    uint16_t bgcolour;
    uint16_t bracketcolour;
    if (isok == okCancel) {
        fgcolour = color_yellow;
        bgcolour = color_blue;
        bracketcolour = color_white;
    } else {
        fgcolour = color_black;
        bgcolour = color_grey;
        bracketcolour = color_black;
    }
    char* str = "      ";
    if (isok) {
        str = "  OK  ";
    } else {
        str = "Cancel";
    }
    drawRect(bgcolour, posx*font_width, posy*font_height, (posx + 8)*font_width, (posy + 1)*font_height);
    displayChar("<", bgcolour, bracketcolour, posx, posy);
    displayString(str, bgcolour, fgcolour, posx+1, posy);
    displayChar(">", bgcolour, bracketcolour, posx+7, posy);
}

void dialogHandleInputs() {
    key = os_GetCSC();
        if (justText) {
            if (key == sk_Left)  {
                selected -= 8;
                update = true;
            }
            if (key == sk_Right) {
                selected += 8;
                update = true;
            }
        } else {
            if ((key == sk_Left) || (key == sk_Right)) {
                okCancel = !okCancel;
                update = true;
            }
        }
        if (key == sk_Up) {
            selected -= 1;
            update = true;
        }
        if (key == sk_Down) {
            selected += 1;
            update = true;
        }
        if (key == sk_Enter) {
            go = true;
        }
        if (selected > scrolllimit) {
            if (justText) {
                selected = scrolllimit;
            } else {
                selected = 0;
            }
        }
        if (selected < 0) {
            if (justText) {
                selected = 0;
            } else {
            selected = scrolllimit;
            }
        }
}

int mainMenu() {
    dialogBG("HelpMii", "v0.1", "Wii Linux Support Program");
    dialogFG(14, 25, "Main Menu", 10);
    update = true;
    go = false;
    key = 0;
    selected = 1;
    okCancel = true;
    scrolllimit = 9;
    justText = false;
    while (!go) {
        if (update) {
            option(1, "Join the Discord",    5,  7);
            option(2, "Read the Guide",      5,  8);
            option(3, "Upload support logs", 5,  9);
            option(9, "About HelpMii",       5, 10);
            option(0, "Quit",                5, 11);
            okCancelButton(true,   6, 14);
            okCancelButton(false, 16, 14);
            update = false;
        }
        dialogHandleInputs();
        if (selected == 4) {
            selected = 9;
        } 
        if (selected == 8) {
            selected = 3;
        }
    }
    if (okCancel) {
        return selected;
    } else {
        return 0;
    }
}

int aboutDisplay() {
    dialogFG(18, 30, "About HelpMii-CE", 10);
    update = true;
    go = false;
    key = 0;
    selected = 0;
    okCancel = true;
    scrolllimit = 11;
    justText = true;
    while (!go) {
        if (update) {
            textLine(  0, "HelpMii - The Wii Linux",  3, 5, 9, 24, color_grey, color_black);
            textLine(  1, "Support Program.",         3, 5, 9, 24, color_grey, color_black);
            textLine(  2, "",                         3, 5, 9, 24, color_grey, color_black);
            textLine(  3, "HelpMii Version v0.1",     3, 5, 9, 24, color_grey, color_black);
            textLine(  4, "Using SuperbeeLavaman",    3, 5, 9, 24, color_grey, color_black);
            textLine(  5, "Dialog framework v0.1.",   3, 5, 9, 24, color_grey, color_black);
            textLine(  6, "",                         3, 5, 9, 24, color_grey, color_black);
            textLine(  7, "HelpMii was made by",      3, 5, 9, 24, color_grey, color_black);
            textLine(  8, "Techflash, Tech64, and",   3, 5, 9, 24, color_grey, color_black);
            textLine(  9, "other contributors.",      3, 5, 9, 24, color_grey, color_black);
            textLine( 10, "CE Port written by",       3, 5, 9, 24, color_grey, color_black);
            textLine( 11, "SuperbeeLavaman.",         3, 5, 9, 24, color_grey, color_black);
            textLine( 12, "This program is licensed", 3, 5, 9, 24, color_grey, color_black);
            textLine( 13, "under the terms of the",   3, 5, 9, 24, color_grey, color_black);
            textLine( 14, "GNU General Public",       3, 5, 9, 24, color_grey, color_black);
            textLine( 15, "License, version 2.",      3, 5, 9, 24, color_grey, color_black);
            textLine( 16, "You may find these terms", 3, 5, 9, 24, color_grey, color_black);
            textLine( 17, "under the HelpMii",        3, 5, 9, 24, color_grey, color_black);
            textLine( 18, "install directory, under", 3, 5, 9, 24, color_grey, color_black);
            textLine( 19, "the LICENSE file.",        3, 5, 9, 24, color_grey, color_black);
            textLine( 20, "",                         3, 5, 9, 24, color_grey, color_black);

            okCancelButton(true, 11, 16);
            update = false;
        }
        dialogHandleInputs();
        okCancel = true;
    }
    return 0;
}

void renderQRCode() {
    char qr_bytes[] = {                                 // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        0b00000001, 0b10111111, 0b01100100, 0b00000111, // X       XX XXXXXX XX  X       X // each line ends with an extra 111 to fill out the
        0b01111101, 0b00100100, 0b10111101, 0b11110111, // X XXXXX X  X  X  X XXXX XXXXX X // remaining bits and match the white border.
        0b01000101, 0b10001110, 0b11000101, 0b00010111, // X X   X XX   XXX XX   X X   X X 
        0b01000101, 0b00101111, 0b00101101, 0b00010111, // X X   X X  X XXXX  X XX X   X X 
        0b01000101, 0b11010011, 0b10000101, 0b00010111, // X X   X XXX X  XXX    X X   X X  
        0b01111101, 0b01011000, 0b01010101, 0b11110111, // X XXXXX X X XX    X X X XXXXX X  
        0b00000001, 0b01010101, 0b01010100, 0b00000111, // X       X X X X X X X X       X  
        0b11111111, 0b11110010, 0b11111111, 0b11111111, // XXXXXXXXXXXXX  X XXXXXXXXXXXXXX  
        0b00000100, 0b00010100, 0b00101010, 0b10101111, // X     X     X X    X X X X X XX  
        0b10000011, 0b00111110, 0b01100000, 0b01110111, // XX     XX  XXXXX  XX      XXX X  
        0b10010100, 0b00100110, 0b01110100, 0b11111111, // XX  X X    X  XX  XXX X  XXXXXX  
        0b00000011, 0b10001101, 0b11000101, 0b00101111, // X      XXX   XX XXX   X X  X XX  
        0b00001100, 0b10101110, 0b00101111, 0b10011111, // X    XX  X X XXX   X XXXXX  XXX  
        0b10110011, 0b01010000, 0b01001100, 0b01110111, // XX XX  XX X X     X  XX   XXX X  
        0b11100100, 0b10111000, 0b11110010, 0b10011111, // XXXX  X  X XXX   XXXX  X X  XXX  
        0b11100111, 0b01110010, 0b11110001, 0b01101111, // XXXX  XXX XXX  X XXXX   X XX XX  
        0b00010000, 0b01010101, 0b10001110, 0b10011111, // X   X     X X X XX   XXX X  XXX  
        0b01010010, 0b11111111, 0b00000000, 0b01010111, // X X X  X XXXXXXXX         X X X  
        0b01101101, 0b10100110, 0b00110110, 0b01011111, // X XX XX XX X  XX   XX XX  X XXX  
        0b01101110, 0b11001100, 0b01100011, 0b11101111, // X XX XXX XX  XX   XX   XXXXX XX  
        0b01010000, 0b11101111, 0b11100000, 0b01000111, // X X X    XXX XXXXXXX      X   X  
        0b11111111, 0b00110000, 0b00110111, 0b00000111, // XXXXXXXXX  XX      XX XXX     X  
        0b00000001, 0b00111000, 0b11000101, 0b00011111, // X       X  XXX   XX   X X   XXX  
        0b01111101, 0b11010010, 0b11000111, 0b01111111, // X XXXXX XXX X  X XX   XXX XXXXX  
        0b01000101, 0b00110101, 0b10110000, 0b01011111, // X X   X X  XX X XX XX     X XXX  
        0b01000101, 0b01011110, 0b01000011, 0b10000111, // X X   X X X XXXX  X    XXX    X  
        0b01000101, 0b01100110, 0b00111000, 0b00001111, // X X   X X XX  XX   XXX       XX  
        0b01111101, 0b01101100, 0b01010101, 0b00101111, // X XXXXX X XX XX   X X X X  X XX  
        0b00000001, 0b01001111, 0b11101000, 0b01011111, // X       X X  XXXXXXX X    X XXX
    };                                                  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    int idx;
    int idy = 0;
    int idz;
    while (idy < 29) {
        idx = 0;
        while (idx < 4) {
            idz = 0;
            char pixels = qr_bytes[idx + (4*idy)];
            while (idz < 8) {
                bool pixel = (pixels << idz) & 0x80;
                if (pixel) {
                    vram[16*idx+640*idy+2*idz+2890] = 0xFFFF; // each pixel is pasted 4 times
                    vram[16*idx+640*idy+2*idz+2891] = 0xFFFF; // at 1px horizontal and vertical
                    vram[16*idx+640*idy+2*idz+3210] = 0xFFFF; // offsets to achieve 2x scaling.
                    vram[16*idx+640*idy+2*idz+3211] = 0xFFFF;
                } else {
                    vram[16*idx+640*idy+2*idz+2890] = 0x0000; // this display runs in rgb565 by
                    vram[16*idx+640*idy+2*idz+2891] = 0x0000; // default and is required when
                    vram[16*idx+640*idy+2*idz+3210] = 0x0000; // os functions like the font
                    vram[16*idx+640*idy+2*idz+3211] = 0x0000; // renderer are used.
                }
                idz++;
            }
            idx++;
        }
        idy++;
    }
}


void qrCodePage() {
    drawRect(color_black, 0, 0, 320, 240);
    drawRect(color_white, 2, 2, 76, 76);
    renderQRCode();
    displayString("Scan this QR to join", color_black, color_white, 8, 1);
    displayString("the Discord server.", color_black, color_white, 8, 2);
    displayString("          |", color_black, color_white, 8, 3);
    displayString("<---------/", color_black, color_white, 8, 4);
    displayString("Not Scanning? Try another app.", color_black, color_white, 1, 6);
    displayString("Still not scanning, or don't", color_black, color_white, 1, 8);
    displayString("have a device to scan it?", color_black, color_white, 1,9);
    displayString("Use this link.", color_black, color_white, 1, 10);
    displayString("https://discord.gg/XfMHMhSQ8d", color_black, color_white, 1, 12);
    displayString("Press any key to continue...", color_black, color_white, 1, 14);
    while(!os_GetCSC());
}

void guidePage() {
    drawRect(color_black, 0, 0, 320, 240);
#ifdef GETTING_STARTED_GUIDE
    update = true;
    go = false;
    key = 0;
    selected = 0;
    okCancel = true;
    scrolllimit = 453;
    justText = true;
    while (!go) {
        if (update) {
            textLine(  0, "Getting Started with Wii Linux", 0, 0, 17, 31, color_black, color_white);
            textLine(  1, "==============================", 0, 0, 17, 31, color_black, color_white);
            textLine(  2, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(  3, "Welcome to your new Wii Linux",  0, 0, 17, 31, color_black, color_white);
            textLine(  4, "install! This guide will show",  0, 0, 17, 31, color_black, color_white);
            textLine(  5, "you how to use a few basic",     0, 0, 17, 31, color_black, color_white);
            textLine(  6, "functions of your system.",      0, 0, 17, 31, color_black, color_white);
            textLine(  7, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(  8, "A few notes on the formatting",  0, 0, 17, 31, color_black, color_white);
            textLine(  9, "of this guide:",                 0, 0, 17, 31, color_black, color_white);
            textLine( 10, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 11, "- A word with brackets, like",   0, 0, 17, 31, color_black, color_white);
            textLine( 12, "  \"this[6]\" refers to a",      0, 0, 17, 31, color_black, color_white);
            textLine( 13, "  section number. If you want",  0, 0, 17, 31, color_black, color_white);
            textLine( 14, "  more info on that topic, skip",0, 0, 17, 31, color_black, color_white);
            textLine( 15, "  to that number of the guide.", 0, 0, 17, 31, color_black, color_white);
            textLine( 16, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 17, "- You may want to use the Left", 0, 0, 17, 31, color_black, color_white);
            textLine( 18, "  and Right keys to scroll",     0, 0, 17, 31, color_black, color_white);
            textLine( 19, "  around quickly, or use up and",0, 0, 17, 31, color_black, color_white);
            textLine( 20, "  down to move around one line", 0, 0, 17, 31, color_black, color_white);
            textLine( 21, "  at a time.",                   0, 0, 17, 31, color_black, color_white);
            textLine( 22, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 23, "- A word (or set of words)",     0, 0, 17, 31, color_black, color_white);
            textLine( 24, "  wrapped in `backticks` is a",  0, 0, 17, 31, color_black, color_white);
            textLine( 25, "  command that you can execute", 0, 0, 17, 31, color_black, color_white);
            textLine( 26, "  on your system.",              0, 0, 17, 31, color_black, color_white);
            textLine( 27, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 28, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 29, "Table of contents",              0, 0, 17, 31, color_black, color_white);
            textLine( 30, "=================",              0, 0, 17, 31, color_black, color_white);
            textLine( 31, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 32, "Section 1 - Basic system nav.",  0, 0, 17, 31, color_black, color_white);
            textLine( 33, "Section 2 - Edit text files",    0, 0, 17, 31, color_black, color_white);
            textLine( 34, "Section 3 - Setting up network", 0, 0, 17, 31, color_black, color_white);
            textLine( 35, "Section 4 - Enable swap & ZRAM", 0, 0, 17, 31, color_black, color_white);
            textLine( 36, "Section 5 - FAQ / help",         0, 0, 17, 31, color_black, color_white);
            textLine( 37, "Section 6 - Contacting support", 0, 0, 17, 31, color_black, color_white);
            textLine( 38, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 39, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 40, "1. Basic System Navigation",     0, 0, 17, 31, color_black, color_white);
            textLine( 41, "==========================",     0, 0, 17, 31, color_black, color_white);
            textLine( 42, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 43, "To change beteen TTYs, press",   0, 0, 17, 31, color_black, color_white);
            textLine( 44, "Alt+F1 though Alt+F12 on your",  0, 0, 17, 31, color_black, color_white);
            textLine( 45, "keyboard. The system boots up",  0, 0, 17, 31, color_black, color_white);
            textLine( 46, "TTY1 (Alt+F1)",                  0, 0, 17, 31, color_black, color_white);
            textLine( 47, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 48, "You'll need to know how to do",  0, 0, 17, 31, color_black, color_white);
            textLine( 49, "this for the rest of the guide,",0, 0, 17, 31, color_black, color_white);
            textLine( 50, "so practice with it now if you", 0, 0, 17, 31, color_black, color_white);
            textLine( 51, "need it. Press Alt+F2 to switch",0, 0, 17, 31, color_black, color_white);
            textLine( 52, "to TTY2, then Alt+F1 to get",    0, 0, 17, 31, color_black, color_white);
            textLine( 53, "back to TTY1.",                  0, 0, 17, 31, color_black, color_white);
            textLine( 54, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 55, "For the rest of the guide, if",  0, 0, 17, 31, color_black, color_white);
            textLine( 56, "you would like to run a command",0, 0, 17, 31, color_black, color_white);
            textLine( 57, "while running the guide on TTY1",0, 0, 17, 31, color_black, color_white);
            textLine( 58, "you can and should switch TTYs", 0, 0, 17, 31, color_black, color_white);
            textLine( 59, "to do so, rather than exiting",  0, 0, 17, 31, color_black, color_white);
            textLine( 60, "and re-entering the guide every",0, 0, 17, 31, color_black, color_white);
            textLine( 61, "time.",                          0, 0, 17, 31, color_black, color_white);
            textLine( 62, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 63, "To exit this guide at any time,",0, 0, 17, 31, color_black, color_white);
            textLine( 64, "press [Enter] on the keypad.",   0, 0, 17, 31, color_black, color_white);
            textLine( 65, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 66, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 67, "2. Editing text files.",         0, 0, 17, 31, color_black, color_white);
            textLine( 68, "======================",         0, 0, 17, 31, color_black, color_white);
            textLine( 69, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 70, "Wii Linux ArchPOWER comes with", 0, 0, 17, 31, color_black, color_white);
            textLine( 71, "2 text editors pre-installed.",  0, 0, 17, 31, color_black, color_white);
            textLine( 72, "`vim`, and `nano`. If you",      0, 0, 17, 31, color_black, color_white);
            textLine( 73, "aren't already familiar with",   0, 0, 17, 31, color_black, color_white);
            textLine( 74, "`vim`, I would recommend using", 0, 0, 17, 31, color_black, color_white);
            textLine( 75, "`nano`, as it behaves much more",0, 0, 17, 31, color_black, color_white);
            textLine( 76, "like text editors that you may", 0, 0, 17, 31, color_black, color_white);
            textLine( 77, "already be used to.",            0, 0, 17, 31, color_black, color_white);
            textLine( 78, "",                               0, 0, 17, 31, color_black, color_white);
            textLine( 79, "To edit a text file, use the",   0, 0, 17, 31, color_black, color_white);
            textLine( 80, "following steps:",               0, 0, 17, 31, color_black, color_white);
            textLine( 81, " 1. Run the command",            0, 0, 17, 31, color_black, color_white);
            textLine( 82, "    `[editor] /path/to/file`",   0, 0, 17, 31, color_black, color_white);
            textLine( 83, "    replacing `[editor]` with",  0, 0, 17, 31, color_black, color_white);
            textLine( 84, "    your editor of choice,",     0, 0, 17, 31, color_black, color_white);
            textLine( 85, "    either `vim` or `nano`. As", 0, 0, 17, 31, color_black, color_white);
            textLine( 86, "    well, replace",              0, 0, 17, 31, color_black, color_white);
            textLine( 87, "    `/path/to/file` with the",   0, 0, 17, 31, color_black, color_white);
            textLine( 88, "    path to the file that you",  0, 0, 17, 31, color_black, color_white);
            textLine( 89, "    desire to edit.",            0, 0, 17, 31, color_black, color_white);
            textLine( 90, " 2. Make any modifications to",  0, 0, 17, 31, color_black, color_white);
            textLine( 91, "    the file that you want.",    0, 0, 17, 31, color_black, color_white);
            textLine( 92, " 3. Save and quit the  file.",   0, 0, 17, 31, color_black, color_white);
            textLine( 93, "    here's how you do it in",    0, 0, 17, 31, color_black, color_white);
            textLine( 94, "    each editor:",               0, 0, 17, 31, color_black, color_white);
            textLine( 95, "    `vim`: '<ESC><ESC>' to get", 0, 0, 17, 31, color_black, color_white);
            textLine( 96, "    out of whatever mode you're",0, 0, 17, 31, color_black, color_white);
            textLine( 97, "    in, back to normal mode.",   0, 0, 17, 31, color_black, color_white);
            textLine( 98, "    ':' to enter command mode.", 0, 0, 17, 31, color_black, color_white);
            textLine( 99, "    'wq<Enter>' to send the",    0, 0, 17, 31, color_black, color_white);
            textLine(100, "    command to *W*rite the file",0, 0, 17, 31, color_black, color_white);
            textLine(101, "    and *Q*uit.",                0, 0, 17, 31, color_black, color_white);
            textLine(102, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(103, "    `nano`: '<Ctrl-X>' to tell", 0, 0, 17, 31, color_black, color_white);
            textLine(104, "    `nano` to exit. 'y' to",     0, 0, 17, 31, color_black, color_white);
            textLine(105, "    confirm saving the file.",   0, 0, 17, 31, color_black, color_white);
            textLine(106, "    press '<Enter>' to confirm", 0, 0, 17, 31, color_black, color_white);
            textLine(107, "    the file path.",             0, 0, 17, 31, color_black, color_white);
            textLine(108, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(109, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(110, "3. Setting up networking access",0, 0, 17, 31, color_black, color_white);
            textLine(111, "===============================",0, 0, 17, 31, color_black, color_white);
            textLine(112, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(113, "You'll probably want to give",   0, 0, 17, 31, color_black, color_white);
            textLine(114, "your Wii access to the",         0, 0, 17, 31, color_black, color_white);
            textLine(115, "internet. Your install of Wii",  0, 0, 17, 31, color_black, color_white);
            textLine(116, "Linux ArchPOWER comes with",     0, 0, 17, 31, color_black, color_white);
            textLine(117, "NetworkManager by default.",     0, 0, 17, 31, color_black, color_white);
            textLine(118, "You can use this to set up",     0, 0, 17, 31, color_black, color_white);
            textLine(119, "networking.",                    0, 0, 17, 31, color_black, color_white);
            textLine(120, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(121, "Firstly, you'll want to verify", 0, 0, 17, 31, color_black, color_white);
            textLine(122, "that your network adapter is",   0, 0, 17, 31, color_black, color_white);
            textLine(123, "detected:",                      0, 0, 17, 31, color_black, color_white);
            textLine(124, "  To check if it's supported,",  0, 0, 17, 31, color_black, color_white);
            textLine(125, "  you can run `ip a` from the",  0, 0, 17, 31, color_black, color_white);
            textLine(126, "  shell.",                       0, 0, 17, 31, color_black, color_white);
            textLine(127, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(128, "  'wlan0' is the internal WiFi", 0, 0, 17, 31, color_black, color_white);
            textLine(129, "  adapter (Broadcom 4318).",     0, 0, 17, 31, color_black, color_white);
            textLine(130, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(131, "  If you see a device name",     0, 0, 17, 31, color_black, color_white);
            textLine(132, "  starting with 'en' or 'eth',", 0, 0, 17, 31, color_black, color_white);
            textLine(133, "  that's your USB Ethernet",     0, 0, 17, 31, color_black, color_white);
            textLine(134, "  adapter.",                     0, 0, 17, 31, color_black, color_white);
            textLine(135, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(136, "  If you see a device name",     0, 0, 17, 31, color_black, color_white);
            textLine(137, "  starting with 'wl' that is",   0, 0, 17, 31, color_black, color_white);
            textLine(138, "  NOT 'wlan0', that's your USB", 0, 0, 17, 31, color_black, color_white);
            textLine(139, "  WiFi adapter.",                0, 0, 17, 31, color_black, color_white);
            textLine(140, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(141, "  If you don't see it, your",    0, 0, 17, 31, color_black, color_white);
            textLine(142, "  adapter may not be supported", 0, 0, 17, 31, color_black, color_white);
            textLine(143, "  by this kernel version. You",  0, 0, 17, 31, color_black, color_white);
            textLine(144, "  may want to contact support[6",0, 0, 17, 31, color_black, color_white);
            textLine(145, "  if you want to check.",        0, 0, 17, 31, color_black, color_white);
            textLine(146, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(147, "Here's a few cases for how you", 0, 0, 17, 31, color_black, color_white);
            textLine(148, "could do so:",                   0, 0, 17, 31, color_black, color_white);
            textLine(149, "- Connection via Ethernet with", 0, 0, 17, 31, color_black, color_white);
            textLine(150, "  DHCP",                         0, 0, 17, 31, color_black, color_white);
            textLine(151, "    It should just work out of", 0, 0, 17, 31, color_black, color_white);
            textLine(152, "    the box, assuming that your",0, 0, 17, 31, color_black, color_white);
            textLine(153, "    USB NIC is supported by the",0, 0, 17, 31, color_black, color_white);
            textLine(154, "    Linux 4.5 Kernel.",          0, 0, 17, 31, color_black, color_white);
            textLine(155, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(156, "    If it's not working, check", 0, 0, 17, 31, color_black, color_white);
            textLine(157, "    that the cable is clicked",  0, 0, 17, 31, color_black, color_white);
            textLine(158, "    all the way in on both",     0, 0, 17, 31, color_black, color_white);
            textLine(159, "    sides. If it's still not",   0, 0, 17, 31, color_black, color_white);
            textLine(160, "    working, or you don't see",  0, 0, 17, 31, color_black, color_white);
            textLine(161, "    the interface name, contact",0, 0, 17, 31, color_black, color_white);
            textLine(162, "    support[6]",                 0, 0, 17, 31, color_black, color_white);
            textLine(163, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(164, "- Connection via Ethernet with", 0, 0, 17, 31, color_black, color_white);
            textLine(165, "  a static address",             0, 0, 17, 31, color_black, color_white);
            textLine(166, "    You can configure this with",0, 0, 17, 31, color_black, color_white);
            textLine(167, "    NetworkManager.",            0, 0, 17, 31, color_black, color_white);
            textLine(168, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(169, "    Firstly, check that your",   0, 0, 17, 31, color_black, color_white);
            textLine(170, "    NIC is supported. If it is,",0, 0, 17, 31, color_black, color_white);
            textLine(171, "    then you should see its",    0, 0, 17, 31, color_black, color_white);
            textLine(172, "    interface.",                 0, 0, 17, 31, color_black, color_white);
            textLine(173, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(174, "    If so, then you can set it", 0, 0, 17, 31, color_black, color_white);
            textLine(175, "    up like you would any other",0, 0, 17, 31, color_black, color_white);
            textLine(176, "    ethernet interface, using",  0, 0, 17, 31, color_black, color_white);
            textLine(177, "    `nmcli` or `nmtui`.",        0, 0, 17, 31, color_black, color_white);
            textLine(178, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(179, "    An example in `nmtui`:",     0, 0, 17, 31, color_black, color_white);
            textLine(180, "    1. Start `nmtui`.",          0, 0, 17, 31, color_black, color_white);
            textLine(181, "    2. Choose 'Edit a ",         0, 0, 17, 31, color_black, color_white);
            textLine(182, "       connection'.",            0, 0, 17, 31, color_black, color_white);
            textLine(183, "    3. Pick your adapter -",     0, 0, 17, 31, color_black, color_white);
            textLine(184, "       probably 'Wired",         0, 0, 17, 31, color_black, color_white);
            textLine(185, "       connection 1'.",          0, 0, 17, 31, color_black, color_white);
            textLine(186, "    4. Select 'IPv4",            0, 0, 17, 31, color_black, color_white);
            textLine(187, "       CONFIGURATION', set the", 0, 0, 17, 31, color_black, color_white);
            textLine(188, "       type to '<Manual>'.",     0, 0, 17, 31, color_black, color_white);
            textLine(189, "    5. To the right, select",    0, 0, 17, 31, color_black, color_white);
            textLine(190, "       '<Show>'.",               0, 0, 17, 31, color_black, color_white);
            textLine(191, "    6. To the right of",         0, 0, 17, 31, color_black, color_white);
            textLine(192, "       'Addresses', select",     0, 0, 17, 31, color_black, color_white);
            textLine(193, "       '<Add...>'",              0, 0, 17, 31, color_black, color_white);
            textLine(194, "    7. Enter your desired IP",   0, 0, 17, 31, color_black, color_white);
            textLine(195, "       address and subnet mask,",0, 0, 17, 31, color_black, color_white);
            textLine(196, "       e.g. 192.168.0.10/24",    0, 0, 17, 31, color_black, color_white);
            textLine(197, "    8. To the right of the",     0, 0, 17, 31, color_black, color_white);
            textLine(198, "       gateway, enter your",     0, 0, 17, 31, color_black, color_white);
            textLine(199, "       desired gateway/router,", 0, 0, 17, 31, color_black, color_white);
            textLine(200, "       e.g. 192.168.0.1",        0, 0, 17, 31, color_black, color_white);
            textLine(201, "    9. (optional) To the right", 0, 0, 17, 31, color_black, color_white);
            textLine(202, "       of DNS servers, select",  0, 0, 17, 31, color_black, color_white);
            textLine(203, "       '<Add.>'.",               0, 0, 17, 31, color_black, color_white);
            textLine(204, "   10. (optional) Enter your",   0, 0, 17, 31, color_black, color_white);
            textLine(205, "       desired DNS server.",     0, 0, 17, 31, color_black, color_white);
            textLine(206, "   11. Move down to the bottom", 0, 0, 17, 31, color_black, color_white);
            textLine(207, "       and select '<OK>' to",    0, 0, 17, 31, color_black, color_white);
            textLine(208, "       save your changes.",      0, 0, 17, 31, color_black, color_white);
            textLine(209, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(210, "- Connection via builtin (b43)", 0, 0, 17, 31, color_black, color_white);
            textLine(211, "  WiFi with DHCP",               0, 0, 17, 31, color_black, color_white);
            textLine(212, "    You can configure this with",0, 0, 17, 31, color_black, color_white);
            textLine(213, "    NetworkManager",             0, 0, 17, 31, color_black, color_white);
            textLine(214, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(215, "    You can set up WiFi like",   0, 0, 17, 31, color_black, color_white);
            textLine(216, "    you would with any other",   0, 0, 17, 31, color_black, color_white);
            textLine(217, "    WiFi adapter under",         0, 0, 17, 31, color_black, color_white);
            textLine(218, "    NetworkManager, using",      0, 0, 17, 31, color_black, color_white);
            textLine(219, "    `nmcli` or `nmtui`.",        0, 0, 17, 31, color_black, color_white);
            textLine(220, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(221, "    Here's how you do it  under",0, 0, 17, 31, color_black, color_white);
            textLine(222, "    `nmtui`.",                   0, 0, 17, 31, color_black, color_white);
            textLine(223, "    1. Start `nmtui`.",          0, 0, 17, 31, color_black, color_white);
            textLine(224, "    2. Choose 'Activate a",      0, 0, 17, 31, color_black, color_white);
            textLine(225, "       connection'.",            0, 0, 17, 31, color_black, color_white);
            textLine(226, "    3. Choose your WiFi",        0, 0, 17, 31, color_black, color_white);
            textLine(227, "       Network, under the WiFi", 0, 0, 17, 31, color_black, color_white);
            textLine(228, "       section.",                0, 0, 17, 31, color_black, color_white);
            textLine(229, "       Don't see anything? Try", 0, 0, 17, 31, color_black, color_white);
            textLine(230, "       pressing 'ESC' on your",  0, 0, 17, 31, color_black, color_white);
            textLine(231, "       keyboard a few times to", 0, 0, 17, 31, color_black, color_white);
            textLine(232, "       exit out of `nmtui`,",    0, 0, 17, 31, color_black, color_white);
            textLine(233, "       then try starting it",    0, 0, 17, 31, color_black, color_white);
            textLine(234, "       back up again.",          0, 0, 17, 31, color_black, color_white);
            textLine(235, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(236, "       Still don't see it?",     0, 0, 17, 31, color_black, color_white);
            textLine(237, "       Contact support[6].",     0, 0, 17, 31, color_black, color_white);
            textLine(238, "    4. If your network requires",0, 0, 17, 31, color_black, color_white);
            textLine(239, "       a password, NetworkMan-", 0, 0, 17, 31, color_black, color_white);
            textLine(240, "       ager will prompt you for",0, 0, 17, 31, color_black, color_white);
            textLine(241, "       it. Enter it, and it",    0, 0, 17, 31, color_black, color_white);
            textLine(242, "       should connect.",         0, 0, 17, 31, color_black, color_white);
            textLine(243, "       If it didn't connect",    0, 0, 17, 31, color_black, color_white);
            textLine(244, "       (asking you for the pas-",0, 0, 17, 31, color_black, color_white);
            textLine(245, "       sword again), you likely",0, 0, 17, 31, color_black, color_white);
            textLine(246, "       typed it wrong. If it",   0, 0, 17, 31, color_black, color_white);
            textLine(247, "       keeps doing ths, but",    0, 0, 17, 31, color_black, color_white);
            textLine(248, "       you're sure you entered", 0, 0, 17, 31, color_black, color_white);
            textLine(249, "       it correctly, contact",   0, 0, 17, 31, color_black, color_white);
            textLine(250, "       support[6].",             0, 0, 17, 31, color_black, color_white);
            textLine(251, "    5. You're now connected!",   0, 0, 17, 31, color_black, color_white);
            textLine(252, "       Exit `nmtui` by follow-", 0, 0, 17, 31, color_black, color_white);
            textLine(253, "       ing the path, or press",  0, 0, 17, 31, color_black, color_white);
            textLine(254, "       'ESC' a few times.",      0, 0, 17, 31, color_black, color_white);
            textLine(255, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(256, "- Connection via builtin (b43)", 0, 0, 17, 31, color_black, color_white);
            textLine(257, "  WiFi with a static address",   0, 0, 17, 31, color_black, color_white);
            textLine(258, "    Follow the guide for",       0, 0, 17, 31, color_black, color_white);
            textLine(259, "    setting it up with DHCP",    0, 0, 17, 31, color_black, color_white);
            textLine(260, "    above, then once done,",     0, 0, 17, 31, color_black, color_white);
            textLine(261, "    follow the guide for sett-", 0, 0, 17, 31, color_black, color_white);
            textLine(262, "    ing up static Ethernet,",    0, 0, 17, 31, color_black, color_white);
            textLine(263, "    replacing 'Wired connection",0, 0, 17, 31, color_black, color_white);
            textLine(264, "    1' with your WiFi network",  0, 0, 17, 31, color_black, color_white);
            textLine(265, "    name.",                      0, 0, 17, 31, color_black, color_white);
            textLine(266, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(267, "- Connection via USB WiFi (DHCP",0, 0, 17, 31, color_black, color_white);
            textLine(268, "  or static)",                   0, 0, 17, 31, color_black, color_white);
            textLine(269, "    Follow the above guides,",   0, 0, 17, 31, color_black, color_white);
            textLine(270, "    but while connecting to the",0, 0, 17, 31, color_black, color_white);
            textLine(271, "    WiFi, ensure to chose the",  0, 0, 17, 31, color_black, color_white);
            textLine(272, "    network listed under your",  0, 0, 17, 31, color_black, color_white);
            textLine(273, "    USB adapter, rather than",   0, 0, 17, 31, color_black, color_white);
            textLine(274, "    the builtin one.",           0, 0, 17, 31, color_black, color_white);
            textLine(275, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(276, "4. Enabling swap and ZRAM",      0, 0, 17, 31, color_black, color_white);
            textLine(277, "=========================",      0, 0, 17, 31, color_black, color_white);
            textLine(278, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(279, "Enabling swap space is quite",   0, 0, 17, 31, color_black, color_white);
            textLine(280, "easy.",                          0, 0, 17, 31, color_black, color_white);
            textLine(281, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(282, "If you wanted to use a swap",    0, 0, 17, 31, color_black, color_white);
            textLine(283, "partition, and you don't have",  0, 0, 17, 31, color_black, color_white);
            textLine(284, "one right now, you've messed",   0, 0, 17, 31, color_black, color_white);
            textLine(285, "up, and it's probably not worth",0, 0, 17, 31, color_black, color_white);
            textLine(286, "mucking around to try to create",0, 0, 17, 31, color_black, color_white);
            textLine(287, "one. Simply use a swap file",    0, 0, 17, 31, color_black, color_white);
            textLine(288, "instead.",                       0, 0, 17, 31, color_black, color_white);
            textLine(289, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(290, "Regardless of file or part-",    0, 0, 17, 31, color_black, color_white);
            textLine(291, "ition, this is the setup",       0, 0, 17, 31, color_black, color_white);
            textLine(292, "process:",                       0, 0, 17, 31, color_black, color_white);
            textLine(293, "  '[your swap]' refers to the",  0, 0, 17, 31, color_black, color_white);
            textLine(294, "  location that  you want to ",  0, 0, 17, 31, color_black, color_white);
            textLine(295, "  swap to. This could be a file",0, 0, 17, 31, color_black, color_white);
            textLine(296, "  (e.g. '/swapfile/'), a",       0, 0, 17, 31, color_black, color_white);
            textLine(297, "  dedicated device (e.g.",       0, 0, 17, 31, color_black, color_white);
            textLine(298, "  '/dev/sdb'), or a partition",  0, 0, 17, 31, color_black, color_white);
            textLine(299, "  (e.g. '/dev/mmcblk0p3).",      0, 0, 17, 31, color_black, color_white);
            textLine(300, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(301, "  If you want a swap file, and", 0, 0, 17, 31, color_black, color_white);
            textLine(302, "  it doesn't already exist, you",0, 0, 17, 31, color_black, color_white);
            textLine(303, "  should create it now - run",   0, 0, 17, 31, color_black, color_white);
            textLine(304, "  the following command,",       0, 0, 17, 31, color_black, color_white);
            textLine(305, "  setting the size appropriate-",0, 0, 17, 31, color_black, color_white);
            textLine(306, "  ly for your usecase. (8GB",    0, 0, 17, 31, color_black, color_white);
            textLine(307, "  should be enough for everyth-",0, 0, 17, 31, color_black, color_white);
            textLine(308, "  ing)",                         0, 0, 17, 31, color_black, color_white);
            textLine(309, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(310, "`fallocate -l [size] /swapfile`",0, 0, 17, 31, color_black, color_white);
            textLine(311, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(312, "  After running that to",        0, 0, 17, 31, color_black, color_white);
            textLine(313, "  generate a swapfile, (again,", 0, 0, 17, 31, color_black, color_white);
            textLine(314, "  if you desire to swap to a",   0, 0, 17, 31, color_black, color_white);
            textLine(315, "  file rather than a partition)",0, 0, 17, 31, color_black, color_white);
            textLine(316, "  you should be fine to follow", 0, 0, 17, 31, color_black, color_white);
            textLine(317, "  the below guide.",             0, 0, 17, 31, color_black, color_white);
            textLine(318, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(319, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(320, "  1. ONCE (and only once, don't",0, 0, 17, 31, color_black, color_white);
            textLine(321, "     do this again to re-",      0, 0, 17, 31, color_black, color_white);
            textLine(322, "     enable), run",              0, 0, 17, 31, color_black, color_white);
            textLine(323, "     `mkswap [your swap]`",      0, 0, 17, 31, color_black, color_white);
            textLine(324, "  2. To enable it for the",      0, 0, 17, 31, color_black, color_white);
            textLine(325, "     current boot, run",         0, 0, 17, 31, color_black, color_white);
            textLine(326, "     `swapon [your swap]`",      0, 0, 17, 31, color_black, color_white);
            textLine(327, "     (you can, intuitively, run",0, 0, 17, 31, color_black, color_white);
            textLine(328, "     `swapoff [your swap]` to",  0, 0, 17, 31, color_black, color_white);
            textLine(329, "     disable it).",              0, 0, 17, 31, color_black, color_white);
            textLine(330, "  3. To turn it on automati-",   0, 0, 17, 31, color_black, color_white);
            textLine(331, "     cally on boot, add the",    0, 0, 17, 31, color_black, color_white);
            textLine(332, "     following line to the",     0, 0, 17, 31, color_black, color_white);
            textLine(333, "     bottom of '/etc/fstab',",   0, 0, 17, 31, color_black, color_white);
            textLine(334, "     using your editor[2] of",   0, 0, 17, 31, color_black, color_white);
            textLine(335, "     choice.",                   0, 0, 17, 31, color_black, color_white);
            textLine(336, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(337, "'[your swap] none swap defaults",0, 0, 17, 31, color_black, color_white);
            textLine(338, "  0 0'",                         0, 0, 17, 31, color_black, color_white);
            textLine(339, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(340, "  4. Make the line neat and",    0, 0, 17, 31, color_black, color_white);
            textLine(341, "     line up with the rest, if", 0, 0, 17, 31, color_black, color_white);
            textLine(342, "     desired.",                  0, 0, 17, 31, color_black, color_white);
            textLine(343, "  5. Save and quit the file[2].",0, 0, 17, 31, color_black, color_white);
            textLine(344, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(345, "Done! You've now enabled swap-", 0, 0, 17, 31, color_black, color_white);
            textLine(346, "to-disk, for the current boot,", 0, 0, 17, 31, color_black, color_white);
            textLine(347, "and any future boot.",           0, 0, 17, 31, color_black, color_white);
            textLine(348, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(349, "Now, if you would like to",      0, 0, 17, 31, color_black, color_white);
            textLine(350, "enable ZRAM (a RAMDisk that",    0, 0, 17, 31, color_black, color_white);
            textLine(351, "compresses its contents, and",   0, 0, 17, 31, color_black, color_white);
            textLine(352, "can be swapped to), it's a",     0, 0, 17, 31, color_black, color_white);
            textLine(353, "little more involved, but",      0, 0, 17, 31, color_black, color_white);
            textLine(354, "is mostly the same process as",  0, 0, 17, 31, color_black, color_white);
            textLine(355, "above. ZRAM is faster than",     0, 0, 17, 31, color_black, color_white);
            textLine(356, "swap-to-disk, but it involves",  0, 0, 17, 31, color_black, color_white);
            textLine(357, "spending CPU time to compress",  0, 0, 17, 31, color_black, color_white);
            textLine(358, "the RAM. It's up to you if",     0, 0, 17, 31, color_black, color_white);
            textLine(359, "this is a worthwhile tradeoff.", 0, 0, 17, 31, color_black, color_white);
            textLine(360, "If you aren't doing CPU-",       0, 0, 17, 31, color_black, color_white);
            textLine(361, "intensive workloads, but more",  0, 0, 17, 31, color_black, color_white);
            textLine(362, "RAM-intensive ones, it may be.", 0, 0, 17, 31, color_black, color_white);
            textLine(363, "  This process is mostly the",   0, 0, 17, 31, color_black, color_white);
            textLine(364, "  same as it is shown on the",   0, 0, 17, 31, color_black, color_white);
            textLine(365, "  Arch Wiki, here:",             0, 0, 17, 31, color_black, color_white);
            textLine(366, "  https://wiki.archlinux.org/",  0, 0, 17, 31, color_black, color_white);
            textLine(367, "title/Zram",                     0, 0, 17, 31, color_black, color_white);
            textLine(368, "  If you would like more info",  0, 0, 17, 31, color_black, color_white);
            textLine(369, "  about ZRAM, you can view",     0, 0, 17, 31, color_black, color_white);
            textLine(370, "  that page.",                   0, 0, 17, 31, color_black, color_white);
            textLine(371, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(372, "  1. Create and edit[2] the",    0, 0, 17, 31, color_black, color_white);
            textLine(373, "     file '/etc/modules-load.d", 0, 0, 17, 31, color_black, color_white);
            textLine(374, "/zram.conf'.",                   0, 0, 17, 31, color_black, color_white);
            textLine(375, "     Place the word 'zram' in",  0, 0, 17, 31, color_black, color_white);
            textLine(376, "     it.",                       0, 0, 17, 31, color_black, color_white);
            textLine(377, "  2. Save and quit the file.",   0, 0, 17, 31, color_black, color_white);
            textLine(378, "  3. Create and edit the file",  0, 0, 17, 31, color_black, color_white);
            textLine(379, "     '/etc/udev/rules.d/99-zra", 0, 0, 17, 31, color_black, color_white);
            textLine(380, "m.rules'.",                      0, 0, 17, 31, color_black, color_white);
            textLine(381, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(382, "     It may be helpful to do",   0, 0, 17, 31, color_black, color_white);
            textLine(383, "     this over SSH from",        0, 0, 17, 31, color_black, color_white);
            textLine(384, "     another machine, or via a", 0, 0, 17, 31, color_black, color_white);
            textLine(385, "     graphical terminal app on", 0, 0, 17, 31, color_black, color_white);
            textLine(386, "     your Wii, so that you can", 0, 0, 17, 31, color_black, color_white);
            textLine(387, "     copy-paste it.",            0, 0, 17, 31, color_black, color_white);
            textLine(388, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(389, "     Place the following line",  0, 0, 17, 31, color_black, color_white);
            textLine(390, "     into it. It should all be", 0, 0, 17, 31, color_black, color_white);
            textLine(391, "     on one line.",              0, 0, 17, 31, color_black, color_white);
            textLine(392, "     ACTION==\"add\", KERNEL==\"zr",0, 0, 17, 31, color_black, color_white);
            textLine(393, "am0\", ATTR{comp_algorithm}=\"zst",0, 0, 17, 31, color_black, color_white);
            textLine(394, "d\", ATTR{disksize}=\"100M\", RUN=",0, 0, 17, 31, color_black, color_white);
            textLine(395, "\"/usr/bin/mkswap -U clear /dev/",0, 0, 17, 31, color_black, color_white);
            textLine(396, "%k\", TAG+=\"systemd\"",           0, 0, 17, 31, color_black, color_white);
            textLine(397, "  4. Save and quit the file,",   0, 0, 17, 31, color_black, color_white);
            textLine(398, "     after making sure that",    0, 0, 17, 31, color_black, color_white);
            textLine(399, "     you typed it correctly.",   0, 0, 17, 31, color_black, color_white);
            textLine(400, "  5. Follow step 3 through 5",   0, 0, 17, 31, color_black, color_white);
            textLine(401, "     of the above guide for",    0, 0, 17, 31, color_black, color_white);
            textLine(402, "     swap-to-disk, to enable",   0, 0, 17, 31, color_black, color_white);
            textLine(403, "     it in '/etc/fstab'. Do",    0, 0, 17, 31, color_black, color_white);
            textLine(404, "     not try to enable it yet,", 0, 0, 17, 31, color_black, color_white);
            textLine(405, "     as the device does not",    0, 0, 17, 31, color_black, color_white);
            textLine(406, "     exist.",                    0, 0, 17, 31, color_black, color_white);
            textLine(407, "  6. Reboot your Wii for the",   0, 0, 17, 31, color_black, color_white);
            textLine(408, "     changes to take effect.",   0, 0, 17, 31, color_black, color_white);
            textLine(409, "     If you run `swapon` with",  0, 0, 17, 31, color_black, color_white);
            textLine(410, "     no paremeters, you should", 0, 0, 17, 31, color_black, color_white);
            textLine(411, "     now see '/dev/zram0'",      0, 0, 17, 31, color_black, color_white);
            textLine(412, "     listed in it.",             0, 0, 17, 31, color_black, color_white);
            textLine(413, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(414, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(415, "5. Other questions/help",        0, 0, 17, 31, color_black, color_white);
            textLine(416, "=======================",        0, 0, 17, 31, color_black, color_white);
            textLine(417, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(418, "Should you need help with",      0, 0, 17, 31, color_black, color_white);
            textLine(419, "anything else not covered in",   0, 0, 17, 31, color_black, color_white);
            textLine(420, "this guide, you can refer to",   0, 0, 17, 31, color_black, color_white);
            textLine(421, "other online resources, such as",0, 0, 17, 31, color_black, color_white);
            textLine(422, "the Arch Wiki.",                 0, 0, 17, 31, color_black, color_white);
            textLine(423, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(424, "If it's Wii-Linux specific, or", 0, 0, 17, 31, color_black, color_white);
            textLine(425, "you can't find anything that",   0, 0, 17, 31, color_black, color_white);
            textLine(426, "helps, do reach out (see below)",0, 0, 17, 31, color_black, color_white);
            textLine(427, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(428, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(429, "6. Contacting support",          0, 0, 17, 31, color_black, color_white);
            textLine(430, "=====================",          0, 0, 17, 31, color_black, color_white);
            textLine(431, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(432, "Should you need help with your", 0, 0, 17, 31, color_black, color_white);
            textLine(433, "Wii Linux install, we're here",  0, 0, 17, 31, color_black, color_white);
            textLine(434, "to help! Run the `helpmii`",     0, 0, 17, 31, color_black, color_white);
            textLine(435, "program for steps for how to",   0, 0, 17, 31, color_black, color_white);
            textLine(436, "get help, or, should you be",    0, 0, 17, 31, color_black, color_white);
            textLine(437, "unable to do so, join the",      0, 0, 17, 31, color_black, color_white);
            textLine(438, "Discord server linked on",       0, 0, 17, 31, color_black, color_white);
            textLine(439, "wii-linux.org!",                 0, 0, 17, 31, color_black, color_white);
            textLine(440, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(441, "Legal Info",                     0, 0, 17, 31, color_black, color_white);
            textLine(442, "==========",                     0, 0, 17, 31, color_black, color_white);
            textLine(443, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(444, "The information on this guide",  0, 0, 17, 31, color_black, color_white);
            textLine(445, "is provided 'AS-IS' without",    0, 0, 17, 31, color_black, color_white);
            textLine(446, "any warranty, express or",       0, 0, 17, 31, color_black, color_white);
            textLine(447, "implied, including but not",     0, 0, 17, 31, color_black, color_white);
            textLine(448, "limited to any implied",         0, 0, 17, 31, color_black, color_white);
            textLine(449, "warranties of merchantability,", 0, 0, 17, 31, color_black, color_white);
            textLine(450, "fitness for a particular",       0, 0, 17, 31, color_black, color_white);
            textLine(451, "purpose, or non-infringenent.",  0, 0, 17, 31, color_black, color_white);
            textLine(452, "Use the information at your own",0, 0, 17, 31, color_black, color_white);
            textLine(453, "risk, and the author assumes no",0, 0, 17, 31, color_black, color_white);
            textLine(454, "responsibility for any issues",  0, 0, 17, 31, color_black, color_white);
            textLine(455, "or damages that may arise from", 0, 0, 17, 31, color_black, color_white);
            textLine(456, "its use.",                       0, 0, 17, 31, color_black, color_white);
            textLine(457, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(458, "This guide is written by",       0, 0, 17, 31, color_black, color_white);
            textLine(459, "\"Techflash\" (Michael Garofalo),",0, 0, 17, 31, color_black, color_white);
            textLine(460, "and is under the Creative",      0, 0, 17, 31, color_black, color_white);
            textLine(461, "Commons CC BY-NS-SA 4.0 License",0, 0, 17, 31, color_black, color_white);
            textLine(462, "avalible at the link below.",    0, 0, 17, 31, color_black, color_white);
            textLine(463, "https://creativecommons.org/lic",0, 0, 17, 31, color_black, color_white);
            textLine(464, "enses/by-nc-sa/4.0/",            0, 0, 17, 31, color_black, color_white);
            textLine(465, "",                               0, 0, 17, 31, color_black, color_white);
            textLine(466, "You may redistribute this guide",0, 0, 17, 31, color_black, color_white);
            textLine(467, "under the same license, however",0, 0, 17, 31, color_black, color_white);
            textLine(468, "attribution is required, and",   0, 0, 17, 31, color_black, color_white);
            textLine(469, "you may not use it for",         0, 0, 17, 31, color_black, color_white);
            textLine(470, "commercial purposes.",           0, 0, 17, 31, color_black, color_white);
            
            update = false;
        }
        dialogHandleInputs();
        okCancel = true;
    }
#else
    displayString("For performance reasons", color_black, color_white, 1, 1);
    displayString("this build does not contain", color_black, color_white, 1, 2);
    displayString("the Getting Started guide.", color_black, color_white, 1, 3);
    displayString("Please use the guide present", color_black, color_white, 1, 4);
    displayString("in the Wii version of the app", color_black, color_white, 1, 5);
    displayString("by running `helpmii` in the", color_black, color_white, 1, 6);
    displayString("console.", color_black, color_white, 1,7);
    displayString("Press any key to continue...", color_black, color_white, 1, 14);
    while(!os_GetCSC());
#endif
}

void supportPage() {
    drawRect(color_black, 0, 0, 320, 240);
    displayString("Unfortunately, this device", color_black, color_white, 1, 1);
    displayString("is unable to obtain logs to", color_black, color_white, 1, 2);
    displayString("upload to support.", color_black, color_white, 1, 3);
    displayString("Please run             on", color_black, color_white, 1, 5);
    displayString("`$ helpmii`", color_black, color_grey, 12, 5);
    displayString("your Wii running Linux and", color_black, color_white, 1, 6);
    displayString("select option 3.", color_black, color_white, 1, 7);
    while(!os_GetCSC());
}

int main(void)
{
    os_FontSelect(os_SmallFont);
    bool quit = false;
    int pageid = 0;

    while (!quit){
        if (pageid == 0) {
            pageid = mainMenu();
            if (pageid == 0) { // option 0 selected, or something is broken.
                quit = true;   // tell the main loop to end.
            }
        } else if (pageid == 9) {
            aboutDisplay(); //About Helpmii
            pageid = 0;
        } else if (pageid == 1) {
            qrCodePage();   //Join the Wii-Linux Discord
            pageid = 0;
        } else if (pageid == 2) {
            guidePage();    //Read the Guide (unimplemented)
            pageid = 0;
        } else if (pageid == 3) {
            supportPage();  //Upload logs to Wii-Linux support
            pageid = 0;
        } else { //THIS SHOULD  N E V E R  RUN!!! Fallback to main menu.
            pageid = 0;
        }
    }
    os_ClrHomeFull(); //Reset the screen to os default before leaving
    return 0;
}
