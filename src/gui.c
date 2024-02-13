#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "conf_loader.h"
#include "swaper.h"

typedef struct {
    GUI gui;
    SIE_GUI_SURFACE *surface;
    SIE_MENU_LIST *menu;
} MAIN_GUI;

typedef struct {
    CSM_RAM csm;
    MAIN_GUI *main_gui;
    int gui_id;
} MAIN_CSM;

typedef struct {
    void *next;
    WSHDR *name;
    void *p;
    int is_daemon;
} NAMELIST;

#ifdef NEWSGOLD
typedef struct {
    CSM_RAM csm;
    int unk1;
    int bearer;
    int gui_id;
    int unk2;
    int param_R1;
    int param_ZERO;
} JAVAINTERFACE_CSM;
#else
typedef struct {
    CSM_RAM csm;
    int bearer;
    int gui_id;
    int gui_id2;
} JAVAINTERFACE_CSM;
#endif

extern int CFG_SHOW_IDLESCREEN;
extern char CFG_BM1[128], CFG_BM2[128], CFG_BM3[128], CFG_BM4[128];
extern char CFG_BM5[128], CFG_BM6[128], CFG_BM7[128], CFG_BM8[128], CFG_BM9[128];

extern char *CONFIG_PATH;
extern CSM_RAM *CSM_UNDER_IDLE;
extern char CFG_CSMLIST_PATH[128];

RECT canvas;
const CSM_DESC maincsm;
const int minus11 = -11;

int CSM_ID;
int SHOW_DAEMONS;
char CSM_TEXT[32768];
char MMENU_HDR_TEXT[32];
NAMELIST * volatile nltop;
int NSD_COUNT, REALD_COUNT;

inline int toupper(int c) {
    if ((c >= 'a') && (c <= 'z')) c += 'A' - 'a';
    return c;
}

int strncmp_nocase(const char *s1, const char *s2, unsigned int n) {
    int i;
    int c;
    while (!(i = (c = toupper(*s1++)) - toupper(*s2++)) && (--n)) if (!c) break;
    return i;
}

void ClearNL(void) {
    LockSched();
    NAMELIST *nl = nltop;
    nltop = 0;
    UnlockSched();
    while (nl) {
        NAMELIST *p = nl;
        FreeWS(nl->name);
        nl = p->next;
        mfree(p);
    }
}

void AddNL(WSHDR *ws, int is_daemon) {
    NAMELIST *nnl = malloc(sizeof(NAMELIST));
    nnl->name = ws;
    nnl->next = 0;
    nnl->is_daemon = is_daemon;
    LockSched();
    if (!nltop) {
        nltop = nnl;
    } else {
        nnl->next = nltop;
        nltop = nnl;
    }
    UnlockSched();
}

char *FindName(CSM_RAM *csm) {
    char *p;
    char s[40];
    static char u[40];

    CSM_DESC *desc = csm->constr;
    sprintf(s, "%08x ", (unsigned int)desc);
    p = strstr(CSM_TEXT, s);
    if (p) {
        return p + 9;
    } else {
        sprintf(u, "Unknown %08X!", (unsigned int)desc);
        return u;
    }
}

int GetNumberOfDialogs(void) {
    WSHDR *ws;
    char ss[64];
    CSM_RAM *icsm;
    int count = 0, c, i;

    void *ircsm = FindCSMbyID(CSM_root()->idle_id);
    ClearNL();
    //Find new style daemons
    NSD_COUNT = 0;
    if (SHOW_DAEMONS) {
        icsm = ((CSM_RAM*)(CSM_root()->csm_q->csm.first))->next;
        while (((unsigned int)(icsm->constr) >> 27) == 0x15) {
            WSHDR *tws = (WSHDR*)(((char *)icsm->constr) + sizeof(CSM_DESC));
            if ((tws->ws_malloc == NAMECSM_MAGIC1) && (tws->ws_mfree == NAMECSM_MAGIC2)) {
                ws = AllocWS(64);
                wstrcpy(ws, tws);
                AddNL(ws, 1);
                nltop->p = icsm;
                count++;
                NSD_COUNT++;         // count new style dialogs
            }
            icsm = icsm->next;
        }
    }
    icsm = CSM_UNDER_IDLE->next;
    do {
        if (icsm == ircsm) {
            if (CFG_SHOW_IDLESCREEN) {
                ws = AllocWS(40);
                wsprintf(ws, "IDLE Screen");
                AddNL(ws, 0);
                nltop->p = icsm;
                count++;
            }
        } else {
            if (icsm->constr != &maincsm) {
                WSHDR *tws = (WSHDR*)(((char *)icsm->constr) + sizeof(CSM_DESC));
                char *s;
                if ((tws->ws_malloc == NAMECSM_MAGIC1) && (tws->ws_mfree == NAMECSM_MAGIC2)) {
                    ws = AllocWS(64);
                    wstrcpy(ws, tws);
                    AddNL(ws, 0);
                    nltop->p = icsm;
                    count++;
                } else {
                    s = FindName(icsm);
#ifdef NEWSGOLD
                    if (!strncmp_nocase(s, "Java", 4)) {
                        int i = ((JAVAINTERFACE_CSM *)icsm)->bearer;
                        int j = ((JAVAINTERFACE_CSM *)icsm)->param_R1;
                        if (i == 2) continue;
                        ws = AllocWS(64);
                        switch (i) {
                            case 1:
                                wsprintf(ws, "Browser", j);
                                break;
                            case 2:
                                wsprintf(ws, "Jam %d", j);
                                break;
                            case 3:
                                switch (j) {
                                    case 2:
                                        wsprintf(ws, "Phone Java");
                                        break;
                                    case 3:
                                        wsprintf(ws, "User Java");
                                        break;
                                    default:
                                        wsprintf(ws, "Unknown Java (%d)", j);
                                        break;
                                }
                                break;
                            default:
                                wsprintf(ws, "Unknown %d(%d) bearer", i, j);
                                break;
                        }
                        goto L_ADD;
                    }
#else
                    if (!strncmp_nocase(s, "Java", 4)) {
                        int i = ((JAVAINTERFACE_CSM*)icsm)->bearer;
                        ws = AllocWS(64);
                        switch(i) {
                            case 1:
                                wsprintf(ws, "Browser");
                                break;
                            case 0xF:
                                wsprintf(ws,"User Java");
                                break;
                            case 0x11:
                                #ifdef X75
                                wsprintf(ws, "Java: %t", GetLastJavaApplication());
                                #else
                                wsprintf(ws, "Java");
                                #endif
                                break;
                            case 18:
                                wsprintf(ws, "E-mail client");
                            break;
                            default:
                                wsprintf(ws, "Unknown %d bearer", i);
                        }
                        goto L_ADD;
                    }
#endif
                    if (strncmp(s, "!SKIP!", 6)) {
                        ws = AllocWS(64);
                        i = 0;
                        while ((c = *s++) >= ' ') {
                            if (i < (sizeof(ss) - 1)) ss[i++] = c;
                        }
                        ss[i] = 0;
                        wsprintf(ws, "%t", ss);
                        L_ADD:
                        AddNL(ws, 0);
                        nltop->p = icsm;
                        count++;
                    }
                }
            }
        }
    } while ((icsm = icsm->next));
    sprintf(MMENU_HDR_TEXT, "XTask3.0 : %d dialogs", count);
    REALD_COUNT = count - NSD_COUNT;
    return count;
}

NAMELIST *GetNLItem(int curitem) {
    NAMELIST *nl;
    nl = nltop;
    int i = 0;
    while (nl) {
        if (i == curitem) break;
        i++;
        nl = nl->next;
    }
    return nl;
}

SIE_MENU_LIST_ITEM *InitMenuItems(unsigned int *count) {
    NAMELIST *nl = nltop;
    SIE_MENU_LIST_ITEM *items = NULL;
    static char color_daemons[] = SIE_COLOR_TEXT_ERROR;

    unsigned int i = 0;
    while (nl) {
        if (nl->name) {
            SIE_MENU_LIST_ITEM *item;
            items = realloc(items, sizeof(SIE_MENU_LIST_ITEM) * (i + 1));
            item = &(items[i]);
            zeromem(item, sizeof(SIE_MENU_LIST_ITEM));

            size_t len = wstrlen(nl->name);
            item->ws = AllocWS((int)len);
            wstrcpy(item->ws, nl->name);
            if (nl->is_daemon) {
                item->color = color_daemons;
            }
            i++;
        } else {
//            item->ws = AllocWS(16);
//            wsprintf(item->ws, "no name???");
        };
        nl = nl->next;
    }
    *count = i;
    return items;
}

static void RefreshMenu(MAIN_GUI *data) {
    unsigned int row = (data->menu) ? data->menu->row : 1;
    Sie_Menu_List_Destroy(data->menu);
    GetNumberOfDialogs();
    data->menu = Sie_Menu_List_Init(data->surface->gui_id);
    data->menu->items = InitMenuItems(&(data->menu->n_items));
    Sie_Menu_List_SetRow(data->menu, row);
    Sie_GUI_Surface_SetHeader(data->surface, MMENU_HDR_TEXT);
}

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(data->surface);
    Sie_Menu_List_Draw(data->menu);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    data->gui.state = 1;
    RefreshMenu(data);
}


static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnFocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    Sie_GUI_Surface_OnUnFocus(data->surface);
    data->gui.state = 1;
}

const char *GetExec(int id) {
    switch (id) {
        case 0:
            return CFG_BM1;
        case 1:
            return CFG_BM2;
        case 2:
            return CFG_BM3;
        case 3:
            return CFG_BM4;
        case 4:
            return CFG_BM5;
        case 5:
            return CFG_BM6;
        case 6:
            return CFG_BM7;
        case 7:
            return CFG_BM8;
        case 8:
            return CFG_BM9;
    }
    return NULL;
}

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    Sie_Menu_List_OnKey(data->menu, msg);
    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        NAMELIST *nl;
        if (msg->gbsmsg->submess >= '1' && msg->gbsmsg->submess <= '9') {
            const char *bookmark = GetExec((msg->gbsmsg->submess - '0') - 1);
            return Sie_Exec_Execute(bookmark) ? 1 : 0;
        }
        switch (msg->gbsmsg->submess) {
            case SIE_MENU_LIST_KEY_PREV:
            case SIE_MENU_LIST_KEY_NEXT:
                Sie_Menu_List_Draw(data->menu);
                break;
            case SIE_MENU_LIST_KEY_ENTER:
                nl = GetNLItem((int)(data->menu->row));
                if (!nl->is_daemon) {
                    CSMtoTop(((CSM_RAM *)(nl->p))->id, -1);
                }
                return 1;
            case LEFT_SOFT:
                CSMtoTop(CSM_root()->idle_id, -1);
                return 1;
            case RIGHT_SOFT:
                return 1;
            case '*':
                SHOW_DAEMONS = !SHOW_DAEMONS;
                RefreshMenu(data);
                DirectRedrawGUI();
                break;
            case '#':
                nl = GetNLItem((int)(data->menu->row));
                if (REALD_COUNT + NSD_COUNT > 0) {
                    int i = ((CSM_RAM*)(nl->p))->id;
                    if (i != CSM_root()->idle_id) {
                        CloseCSM(i);
                    }
                }
                break;
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

const void *const gui_methods[11] = {
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

void LoadCSMList() {
    size_t size = 0;
    unsigned int err = 0;
    int fp = _open(CFG_CSMLIST_PATH, A_ReadOnly + A_BIN, P_READ, &err);
    if (fp != -1) {
        size = _read(fp, CSM_TEXT, 32767, &err);
        _close(fp, &err);
    }
    if (size >= 0) CSM_TEXT[size] = 0;
}

void maincsm_oncreate(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    Sie_GUI_InitCanvas(&canvas);
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    csm->csm.state = 0;
    csm->csm.unk1 = 0;
    csm->main_gui = main_gui;
    csm->main_gui->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers,
                                                  CreateGUI(main_gui));
    csm->gui_id = (int)(csm->main_gui->surface->gui_id);
    LoadCSMList();
}

int maincsm_onmessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if (msg->msg == MSG_CSM_DESTROYED) {
        RefreshMenu(csm->main_gui);
        DirectRedrawGUI_ID((int)(csm->gui_id));
    }
    if (msg->msg == MSG_GUI_DESTROYED) {
        if ((int)msg->data0 == csm->gui_id) {
            csm->csm.state = -3;
        }
    }
    if (msg->msg == MSG_INCOMMING_CALL) {
        csm->csm.state = -3;
    }
    return 1;
}

void maincsm_onclose(CSM_RAM *csm) {
    ClearNL();
    CSM_ID = 0;
}

const CSM_DESC maincsm = {
    maincsm_onmessage,
    maincsm_oncreate,
#ifdef NEWSGOLD
0,
0,
0,
0,
#endif
maincsm_onclose,
sizeof(MAIN_CSM),
1,
&minus11
};


void ShowMenu(void) {
    MAIN_CSM main_csm;
    InitConfig();
    if (!CSM_ID) {
        CSM_ID = CreateCSM(&maincsm, &main_csm, 2);
    }
}
