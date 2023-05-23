#ifndef __STATE_H
#define __STATE_H

typedef enum {
    S_MENU,
    S_DRAWING,
    S_SAVE,
    S_CHANGE_BRIGHTNESS,
} State;

extern State state;

#endif // __STATE_H

