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
const uint16_t color_grey   = 0xDDDD;
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
        if (selected > 9) {
            selected = 0;
        }
        if (selected < 0) {
            selected = 9;
        }
}

int mainMenu() {
    dialogBG("HelpMii", "v0.1", "Wii Linux Support Program");
    dialogFG(14, 25, "Main Menu", 10);
    selected = 1;
    update = true;
    go = false;
    key = 0;
    selected = 1;
    okCancel = true;
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
    }
    return selected;
}

int main(void)
{
    os_FontSelect(os_SmallFont);

    mainMenu();
    
    while (!os_GetCSC());
    os_ClrHomeFull();
    return 0;
}
