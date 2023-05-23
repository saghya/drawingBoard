#ifndef __MENU_H
#define __MENU_H

typedef enum {
    M_NEW_DRAWING,
    M_LOAD_DRAWING,
    M_SAVE_DRAWING,
    M_CHANGE_BRIGHTNESS
} MenuItem;

extern volatile MenuItem selectedMenu;

void menuLoop();
 
#endif // __MENU_H

