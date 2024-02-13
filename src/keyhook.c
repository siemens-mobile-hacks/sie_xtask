#include <swilib.h>
#include "gui.h"
#include "swaper.h"

extern int MAIN_CSM_ID;
extern int CFG_ENA_LOCK;
extern int CFG_ACTIVE_KEY;
extern int CFG_RED_BUT_MODE;
extern int CFG_ENA_LONG_PRESS;
extern int CFG_ACTIVE_KEY_STYLE;

int KeyHook(int submsg, int msg) {
    static int mode; // -1 - XTask GUI present, 0 - XTask GUI absent, 1 - IBUT longpressed, ready for exit
    static int mode_red; // 0 - no press, 1 - long press RED_BUTTON
    static int mode_enter; // 0 - no press, 1 - long press ENTER_BUTTON, 2 - disable KEY_UP process
#ifdef NEWSGOLD
    void *icsm = FindCSMbyID(CSM_root()->idle_id);
    if ((submsg == RED_BUTTON) && (CFG_RED_BUT_MODE)) {
        if ((CSM_root()->csm_q->csm.last == icsm) ||
            (IsCalling())) //(IsGuiOnTop(((int *)icsm)[DISPLACE_OF_IDLEGUI_ID/4]))
        {
            if (msg == KEY_UP) {
                if (mode_red != 2) {
                    mode_red = 0;
                    return KEYHOOK_BREAK;
                }
            }
            mode_red = 2; //Ложим на отпускания
        } else {
            if (msg == KEY_DOWN) {
                if (mode_red == 1) {
                    mode_red = 0;
                    return KEYHOOK_NEXT; //Long press, continue with REDB PRESS
                }
            }
            if (msg == KEY_UP) {
                if (mode_red) {
                    mode_red = 0; //Release after longpress
                    return KEYHOOK_NEXT;
                } else
                    //Release after short press
                {
                    if (CFG_RED_BUT_MODE == 1) {
                        GBS_SendMessage(MMI_CEPID, KEY_DOWN, RIGHT_SOFT);
                    } else {
                        if (!MAIN_CSM_ID) {
                            CSMtoTop(CSM_root()->idle_id, -1);
                        }
                    }
                }
            }
            if (msg == LONG_PRESS) {
                mode_red = 1;
                GBS_SendMessage(MMI_CEPID, KEY_DOWN, RED_BUTTON);
            }
            return KEYHOOK_BREAK;
        }
    }
#endif
    if (CFG_ACTIVE_KEY_STYLE == 3) {
        if (submsg != ENTER_BUTTON) return KEYHOOK_NEXT;
        switch (msg) {
            case KEY_DOWN:
                if (mode_enter == 2) {
                    GBS_SendMessage(MMI_CEPID, KEY_UP, ENTER_BUTTON);
                    return KEYHOOK_NEXT;
                }
                mode_enter = 0;
                return KEYHOOK_BREAK;
            case KEY_UP:
                if (mode_enter == 0) {
                    mode_enter = 2;
                    GBS_SendMessage(MMI_CEPID, KEY_DOWN, ENTER_BUTTON);
                    return KEYHOOK_BREAK;
                }
                if (mode_enter == 2) {
                    mode_enter = 0;
                    return KEYHOOK_NEXT;
                }
                mode_enter = 0;
                return KEYHOOK_BREAK;
            case LONG_PRESS:
                mode_enter = 1;
                if (IsUnlocked() || CFG_ENA_LOCK) {
                    CreateMainGUI();
                }
                mode = 0;
                return KEYHOOK_BREAK;
        }
    }
    // * + # implementation
    if ((CFG_ACTIVE_KEY_STYLE == 2) && !(MAIN_CSM_ID)) {
        if (msg == KEY_UP) {
            mode = 0;
            return KEYHOOK_NEXT;
        }
        if (msg == KEY_DOWN) {
            switch (submsg) {
                case '*':
                    mode = 1;
                    return (0);
                case '#':
                    if (mode == 1) {
                        if (IsUnlocked() || CFG_ENA_LOCK)
                            CreateMainGUI();
                        else mode = 0;
                    } else return KEYHOOK_NEXT;
            }
        }
    }
    if (CFG_ACTIVE_KEY_STYLE < 2) {
        if (submsg != CFG_ACTIVE_KEY) return KEYHOOK_NEXT;
        if (MAIN_CSM_ID) {
            if (msg == KEY_UP) {
                GBS_SendMessage(MMI_CEPID, KEY_DOWN, ENTER_BUTTON);
            }
            return KEYHOOK_BREAK;
        }
        switch (msg) {
            case KEY_DOWN:
                mode = 0;
                if (CFG_ACTIVE_KEY_STYLE == 0)
                    return KEYHOOK_BREAK;
                else
                    return KEYHOOK_NEXT;
            case KEY_UP:
                if (mode == 1) {
                    //Release after longpress
                    mode = 0;
                    if ((CFG_ACTIVE_KEY_STYLE == 1) || (CFG_ENA_LONG_PRESS == 3)) {
                        //Launch on LongPress or Extra on LP - Launch
                        if (IsUnlocked() || CFG_ENA_LOCK) {
                            CreateMainGUI();
                        }
                        return KEYHOOK_BREAK;
                    }
                    if (CFG_ENA_LONG_PRESS == 1) return KEYHOOK_BREAK;
                    if (CFG_ENA_LONG_PRESS == 2) {
                        CSMtoTop(CSM_root()->idle_id, -1);
                        return KEYHOOK_BREAK;
                    }
                    if (CFG_ENA_LONG_PRESS == 4) {
                        CSMtoTop(CSM_root()->idle_id, -1);
                        KbdLock();
                        return KEYHOOK_BREAK;
                    }
                    break;
                }
                if (CFG_ACTIVE_KEY_STYLE == 0) {
                    if (IsUnlocked() || CFG_ENA_LOCK) {
                        CreateMainGUI();
                    }
                    return KEYHOOK_BREAK;
                }
                break;
            case LONG_PRESS:
                mode = 1;
#ifndef NEWSGOLD
                if (CFG_ACTIVE_KEY_STYLE == 1) {
                if (CFG_ENA_LONG_PRESS) {
                    return KEYHOOK_NEXT;
                } else {
                    return KEYHOOK_BREAK;
                }
            }
#else
                return KEYHOOK_BREAK;
#endif
        }
    }
    return KEYHOOK_NEXT;
}
