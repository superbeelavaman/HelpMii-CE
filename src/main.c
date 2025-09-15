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

void textLine(int id, char* name, int posx, int posy, int maxpage, int maxwidth) {
    if ((id >= selected) && (id - selected <= maxpage)) {
        drawRect(color_grey, posx*font_width, (posy + id - selected)*font_height, (posx + maxwidth)*font_width, (posy + id - selected + 1)*font_height);
        displayString(name, color_grey, color_black, posx, posy + id - selected);
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
        if ((key == sk_Left) || (key == sk_Right)) {
            okCancel = !okCancel;
            update = true;
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
            selected = 0;
        }
        if (selected < 0) {
            selected = scrolllimit;
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
    while (!go) {
        if (update) {
            textLine(  0, "HelpMii - The Wii Linux",  3, 5, 9, 24);
            textLine(  1, "Support Program.",         3, 5, 9, 24);
            textLine(  2, "",                         3, 5, 9, 24);
            textLine(  3, "HelpMii Version v0.1",     3, 5, 9, 24);
            textLine(  4, "Using SuperbeeLavaman",    3, 5, 9, 24);
            textLine(  5, "Dialog framework v0.1.",   3, 5, 9, 24);
            textLine(  6, "",                         3, 5, 9, 24);
            textLine(  7, "HelpMii was made by",      3, 5, 9, 24);
            textLine(  8, "Techflash, Tech64, and",   3, 5, 9, 24);
            textLine(  9, "other contributors.",      3, 5, 9, 24);
            textLine( 10, "CE Port written by",       3, 5, 9, 24);
            textLine( 11, "SuperbeeLavaman.",         3, 5, 9, 24);
            textLine( 12, "This program is licensed", 3, 5, 9, 24);
            textLine( 13, "under the terms of the",   3, 5, 9, 24);
            textLine( 14, "GNU General Public",       3, 5, 9, 24);
            textLine( 15, "License, version 2.",      3, 5, 9, 24);
            textLine( 16, "You may find these terms", 3, 5, 9, 24);
            textLine( 17, "under the HelpMii",        3, 5, 9, 24);
            textLine( 18, "install directory, under", 3, 5, 9, 24);
            textLine( 19, "the LICENSE file.",        3, 5, 9, 24);
            textLine( 20, "",                         3, 5, 9, 24);

            okCancelButton(true, 11, 16);
            update = false;
        }
        dialogHandleInputs();
        okCancel = true;
    }
    return 0;
}

void renderQRCode() {
    char qr_bytes[] = {
        0b00000001, 0b10111111, 0b01100100, 0b00000111,
        0b01111101, 0b00100100, 0b10111101, 0b11110111,
        0b01000101, 0b10001110, 0b11000101, 0b00010111,
        0b01000101, 0b00101111, 0b00101101, 0b00010111,
        0b01000101, 0b11010011, 0b10000101, 0b00010111,
        0b01111101, 0b01011000, 0b01010101, 0b11110111,
        0b00000001, 0b01010101, 0b01010100, 0b00000111,
        0b11111111, 0b11110010, 0b11111111, 0b11111111,
        0b00000100, 0b00010100, 0b00101010, 0b10101111,
        0b10000011, 0b00111110, 0b01100000, 0b01110111,
        0b10010100, 0b00100110, 0b01110100, 0b11111111,
        0b00000011, 0b10001101, 0b11000101, 0b00101111,
        0b00001100, 0b10101110, 0b00101111, 0b10011111,
        0b10110011, 0b01010000, 0b01001100, 0b01110111,
        0b11100100, 0b10111000, 0b11110010, 0b10011111,
        0b11100111, 0b01110010, 0b11110001, 0b01101111,
        0b00010000, 0b01010101, 0b10001110, 0b10011111,
        0b01010010, 0b11111111, 0b00000000, 0b01010111,
        0b01101101, 0b10100110, 0b00110110, 0b01011111,
        0b01101110, 0b11001100, 0b01100011, 0b11101111,
        0b01010000, 0b11101111, 0b11100000, 0b01000111,
        0b11111111, 0b00110000, 0b00110111, 0b00000111,
        0b00000001, 0b00111000, 0b11000101, 0b00011111,
        0b01111101, 0b11010010, 0b11000111, 0b01111111,
        0b01000101, 0b00110101, 0b10110000, 0b01011111,
        0b01000101, 0b01011110, 0b01000011, 0b10000111,
        0b01000101, 0b01100110, 0b00111000, 0b00001111,
        0b01111101, 0b01101100, 0b01010101, 0b00101111,
        0b00000001, 0b01001111, 0b11101000, 0b01011111,
    };
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
                    vram[16*idx+640*idy+2*idz+2890] = 0xFFFF;
                    vram[16*idx+640*idy+2*idz+2891] = 0xFFFF;
                    vram[16*idx+640*idy+2*idz+3210] = 0xFFFF;
                    vram[16*idx+640*idy+2*idz+3211] = 0xFFFF;
                } else {
                    vram[16*idx+640*idy+2*idz+2890] = 0x0000;
                    vram[16*idx+640*idy+2*idz+2891] = 0x0000;
                    vram[16*idx+640*idy+2*idz+3210] = 0x0000;
                    vram[16*idx+640*idy+2*idz+3211] = 0x0000;
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
    while(!os_GetCSC());
}

void guidePage() {}

void supportPage() {
    
}

int main(void)
{
    os_FontSelect(os_SmallFont);
    bool quit = false;
    int pageid = 0;

    while (!quit){
        if (pageid == 0) {
            pageid = mainMenu();
            if (pageid == 0) {
                quit = true;
            }
        } else if (pageid == 9) {
            aboutDisplay();
            pageid = 0;
        } else if (pageid == 1) {
            qrCodePage();
            pageid = 0;
        } else if (pageid == 2) {
            guidePage();
            pageid = 0;
        } else if (pageid == 3) {
            supportPage();
            pageid = 0;
        } else { //THIS SHOULD  N E V E R  RUN!!!
            pageid = 0;
        }
    }
    os_ClrHomeFull();
    return 0;
}
