#include <swilib.h>
#include <stdlib.h>
#include "conf_loader.h"
#include "rect_patcher.h"
#include "swaper.h"

typedef struct {
    CSM_RAM csm;
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
extern char CFG_BM1NAME[32], CFG_BM1FILE[128];
extern char CFG_BM2NAME[32], CFG_BM2FILE[128];
extern char CFG_BM3NAME[32], CFG_BM3FILE[128];
extern char CFG_BM4NAME[32], CFG_BM4FILE[128];
extern char CFG_BM5NAME[32], CFG_BM5FILE[128];
extern char CFG_BM6NAME[32], CFG_BM6FILE[128];
extern char CFG_BM7NAME[32], CFG_BM7FILE[128];
extern char CFG_BM8NAME[32], CFG_BM8FILE[128];
extern char CFG_BM9NAME[32], CFG_BM9FILE[128];

extern char *CONFIG_PATH;
extern CSM_RAM *CSM_UNDER_IDLE;
extern char CFG_CSMLIST_PATH[128];

int CSM_ID;
int SHOW_DAEMONS;
char CSM_TEXT[32768];
const CSM_DESC maincsm;
char MMENU_HDR_TEXT[32];
NAMELIST * volatile nltop;
int NSD_COUNT, REALD_COUNT;

const int minus11 = -11;

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

NAMELIST *get_nlitem(int curitem) {
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


void mm_menu_iconhndl(void *data, int curitem, void *unk) {
    NAMELIST *nl;
    WSHDR *ws;
    void *item = AllocMenuItem(data);
    nl = get_nlitem(curitem);
    if (nl) {
        if (nl->name) {
            ws = AllocMenuWS(data, nl->name->wsbody[0] + 2);
            wstrcpy(ws, nl->name);
            if (nl->is_daemon) {
                wsInsertChar(ws, 0x0002, 1);
                wsInsertChar(ws, 0xE008, 1);
            }
        } else {
            ws = AllocMenuWS(data, 16);
            wsprintf(ws, "no name???");
        }
    } else {
        ws = AllocMenuWS(data, 16);
        wsprintf(ws, "error!!!");
    }
    SetMenuItemText(data, item, ws, curitem);
}

const char *GetBookmarkName(int bm) {
    switch (bm) {
        case 0:
            return CFG_BM1NAME;
        case 1:
            return CFG_BM2NAME;
        case 2:
            return CFG_BM3NAME;
        case 3:
            return CFG_BM4NAME;
        case 4:
            return CFG_BM5NAME;
        case 5:
            return CFG_BM6NAME;
        case 6:
            return CFG_BM7NAME;
        case 7:
            return CFG_BM8NAME;
        case 8:
            return CFG_BM9NAME;
    }
    return NULL;
}

const char *GetBookmarkFile(int bm) {
    switch (bm) {
        case 0:
            return CFG_BM1FILE;
        case 1:
            return CFG_BM2FILE;
        case 2:
            return CFG_BM3FILE;
        case 3:
            return CFG_BM4FILE;
        case 4:
            return CFG_BM5FILE;
        case 5:
            return CFG_BM6FILE;
        case 6:
            return CFG_BM7FILE;
        case 7:
            return CFG_BM8FILE;
        case 8:
            return CFG_BM9FILE;
    }
    return NULL;
}

int RunBM(int bm) {
    const char *s = GetBookmarkFile(bm);
    if (s) {
        if (strlen(s)) {
            WSHDR *ws;
            ws = AllocWS(150);
            str_2ws(ws, s, 128);
            ExecuteFile(ws, 0, 0);
            FreeWS(ws);
            return 1;
        }
    }
    return 0;
}

const HEADER_DESC bm_menuhdr = {{0, 0, 131, 21}, NULL,
                                (int)"Bookmarks...", LGP_NULL};
const int menusoftkeys[] = {0, 1, 2};
const SOFTKEY_DESC menu_sk[] = {
    {0x0018, 0x0000, (int)"Config"},
    {0x0001, 0x0000, (int)"Close"},
    {0x003D, 0x0000, (int)LGP_DOIT_PIC}
};

const SOFTKEYSTAB menu_skt =
        {
                menu_sk, 0
        };

void bm_menu_ghook(void *data, int cmd) {
    if (cmd == 0x0A) DisableIDLETMR();
}

int bm_menu_onkey(void *data, GUI_MSG *msg);

void bm_menu_iconhndl(void *data, int curitem, void *unk);

const MENU_DESC bm_menu =
        {
                8, bm_menu_onkey, bm_menu_ghook, NULL,
                menusoftkeys,
                &menu_skt,
                0x10,//0x11,
                bm_menu_iconhndl,
                NULL,   //Items
                NULL,   //Procs
                0   //n
        };

int bm_menu_onkey(void *data, GUI_MSG *msg) {
    int i;
    i = GetCurMenuItem(data);
    if (msg->keys == 0x18) {
        //    GeneralFunc_F1(1);
//    ShowMSG(1,(int)"Under construction!");
        WSHDR *ws;
        ws = AllocWS(150);
        str_2ws(ws, CONFIG_PATH, 128);
        ExecuteFile(ws, 0, 0);
        FreeWS(ws);
        GeneralFunc_flag1(((MAIN_CSM *)FindCSMbyID(CSM_ID))->gui_id, 1);
        return (1);
    }
    if (msg->keys == 0x3D) {
        if (RunBM(i)) {
            GeneralFunc_flag1(((MAIN_CSM *)FindCSMbyID(CSM_ID))->gui_id, 1);
            return (1);
        }
        return (-1);
    }
    return (0);
}


void bm_menu_iconhndl(void *data, int curitem, void *unk) {
    const char *s;
    WSHDR *ws;
    int l;
    void *item = AllocMenuItem(data);

    s = GetBookmarkName(curitem);
    if (s) {
        if ((l = strlen(s))) {
            ws = AllocMenuWS(data, l);
            wsprintf(ws, "%t", s);
        } else {
            s = GetBookmarkFile(curitem);
            if (s) {
                if (strlen(s)) {
                    char *p = strrchr(s, '\\');
                    if (p) {
                        unsigned int pos;
                        ws = AllocWS((l = strlen(p + 1)));
                        str_2ws(ws, p + 1, l);
                        if ((pos = wstrrchr(ws, ws->wsbody[0], '.')) != 0xFFFF) {
                            CutWSTR(ws, pos - 1);
                        }
                    } else goto L_EMPTY;
                } else goto L_EMPTY;
            } else {
                L_EMPTY:
                ws = AllocMenuWS(data, 10);
                wsprintf(ws, "Empty");
            }
        }
    } else {
        ws = AllocMenuWS(data, 10);
        wsprintf(ws, "error");
    }
    SetMenuItemText(data, item, ws, curitem);
}

void ShowBMmenu(void) {
    patch_header(&bm_menuhdr);
    CreateMenu(0, 0, &bm_menu, &bm_menuhdr, 0, 9, 0, 0);
}

int mm_menu_onkey(void *data, GUI_MSG *msg) {
//  MAIN_CSM *csm=MenuGetUserPointer(data);
    NAMELIST *nl = get_nlitem(GetCurMenuItem(data));
    int i;
    if (msg->gbsmsg->msg == KEY_DOWN) {
        i = msg->gbsmsg->submess;
        {
            if (i == '0') {
                ShowBMmenu();
                return (-1);
            }
            if ((i >= '1') && (i <= '9')) {
                if (RunBM(i - '1')) return 1;
                return (-1);
            }
        }
        switch (i) {
            case LEFT_BUTTON:
                if (REALD_COUNT + NSD_COUNT > 0) {
                    i = ((CSM_RAM*)(nl->p))->id;
                    if (i != CSM_root()->idle_id) CloseCSM(i);
                }
                return -1;
            case '#':
                if (REALD_COUNT + NSD_COUNT > 0) {
                    i = ((CSM_RAM*)(nl->p))->id;
                    if (i != CSM_root()->idle_id) CloseCSM(i);
                }
                return 0;
            case '*':
                SHOW_DAEMONS = !SHOW_DAEMONS;
                RefreshGUI();
                return 0;
            case LEFT_SOFT:
                CSMtoTop(CSM_root()->idle_id, -1);
                return 1;
            case ENTER_BUTTON:
                if (!nl->is_daemon) {
                    CSMtoTop(((CSM_RAM *)(nl->p))->id, -1);
                }
                return (1);
            case RIGHT_SOFT:
                return 1;
        }
    }
    return 0;
}

const HEADER_DESC mm_menuhdr = {{0, 0, 131, 21}, NULL,
                                (int)MMENU_HDR_TEXT, LGP_NULL};
const SOFTKEY_DESC mm_menu_sk[] = {
    {0x0018, 0x0000, (int)"Idle"},
    {0x0001, 0x0000, (int)"Back"},
    {0x003D, 0x0000, (int)LGP_DOIT_PIC}
};

const SOFTKEYSTAB mm_menu_skt = {
    mm_menu_sk, 0
};

void mm_menu_ghook(void *data, int cmd) {
    int i, j;
    if (cmd == 9) {
        ClearNL();
    }
    if (cmd == 0x0A) {
        DisableIDLETMR();
        i = GetCurMenuItem(data);
        Menu_SetItemCountDyn(data, j = GetNumberOfDialogs());
        if (i >= j) SetCursorToMenuItem(data, j - 1);
    }
}

const MENU_DESC mm_menu = {
    8, mm_menu_onkey, mm_menu_ghook, NULL,
    menusoftkeys,
    &mm_menu_skt,
    0x10,//0x11,
    mm_menu_iconhndl,
    NULL,   //Items
    NULL,   //Procs
    0   //n
};

void maincsm_oncreate(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM *)data;
    csm->csm.state = 0;
    csm->csm.unk1 = 0;
    int f;
    int sz = 0;
    unsigned int ul;
    if ((f = _open(CFG_CSMLIST_PATH, A_ReadOnly + A_BIN, P_READ, &ul)) != -1) {
        sz = _read(f, CSM_TEXT, 32767, &ul);
        _close(f, &ul);
    }
    if (sz >= 0) CSM_TEXT[sz] = 0;
    patch_header(&mm_menuhdr);
    GetNumberOfDialogs();
    if (REALD_COUNT > 1) {
        csm->gui_id = CreateMenu(0, 0, &mm_menu, &mm_menuhdr, 1, REALD_COUNT, csm,
                                 0);    // if more than 1 dialog, position of cursor in menu will be 1
    }
    else {
        csm->gui_id = CreateMenu(0, 0, &mm_menu, &mm_menuhdr, 0, REALD_COUNT, csm, 0);    // else - 0
    }
}

int maincsm_onmessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM *)data;
    if (msg->msg == MSG_CSM_DESTROYED) {
        RefreshGUI();
    }
    if (msg->msg == MSG_GUI_DESTROYED) {
        if ((int)msg->data0 == csm->gui_id) {
            csm->csm.state = -3;
        }
    }
    if (msg->msg == MSG_INCOMMING_CALL) {
        csm->csm.state = -3;
    }
    return (1);
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
