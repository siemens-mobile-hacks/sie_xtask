#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "idle_csm_hook.h"

typedef struct {
    GUI gui;
    SIE_MENU_LIST *menu;
    SIE_GUI_SURFACE *surface;
} MAIN_GUI;

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg);

extern RECT canvas;
extern int MAIN_CSM_ID;

void Exit() {
    void csm_onclose(CSM_RAM *data);

    LockSched();
    CloseCSM(MAIN_CSM_ID);
    RemoveIdleCSMHook();
    UnlockSched();
}

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(data->surface);
    Sie_Menu_List_Draw(data->menu);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    SIE_MENU_LIST_ITEM item;
    zeromem(&item, sizeof(SIE_MENU_LIST_ITEM));
    data->menu = Sie_Menu_List_Init(data->surface->gui_id);
    item.proc = Exit;
    Sie_Menu_List_AddItem(data->menu, &item, "Exit");
    data->gui.state = 1;
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    Sie_Menu_List_Destroy(data->menu);
    Sie_GUI_Surface_Destroy(data->surface);
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnFocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    data->gui.state = 1;
    Sie_GUI_Surface_OnUnFocus(data->surface);
}

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    Sie_Menu_List_OnKey(data->menu, msg);
    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        switch (msg->gbsmsg->submess) {
            case RIGHT_SOFT:
                return 1;
        }
    }
    return 0;
}

static int OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    return Sie_GUI_Surface_OnKey(data->surface, data, msg);
}

extern void kill_data(void *p, void (*func_p)(void *));

static int method8(void) { return 0; }

static int method9(void) { return 0; }

static const void *const gui_methods[11] = {
        (void*)OnRedraw,
        (void*)OnCreate,
        (void*)OnClose,
        (void*)OnFocus,
        (void*)OnUnFocus,
        (void*)OnKey,
        0,
        (void*)kill_data,
        (void*)method8,
        (void*)method9,
        0
};

void CreateMenuOptions() {
    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    LockSched();
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    main_gui->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers,
                                             CreateGUI(main_gui));
    Sie_GUI_Surface_SetHeader(main_gui->surface, "Options");
    UnlockSched();
}
