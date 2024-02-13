#include <swilib.h>
#include <xtask_ipc.h>
#include "swaper.h"
#include "keyhook.h"
#include "conf_loader.h"

#ifdef NEWSGOLD
    #define USE_ONE_KEY
#endif

#define TMR_SECOND(A) (1300L*A/6)

extern int CSM_ID;
extern int CFG_ACTIVE_KEY;
extern int CFG_SHOW_DAEMONS;
extern int CFG_ACTIVE_KEY_STYLE;
extern char CFG_UNDER_IDLE_CONSTR[];

extern int SHOW_DAEMONS;
extern const char *CONFIG_PATH;

CSM_DESC icsmd;
int (*old_onMessage)(CSM_RAM *, GBS_MSG *);
void (*old_onClose)(CSM_RAM *);

GBSTMR TMR_START;
CSM_RAM *CSM_UNDER_IDLE;
volatile int SHOW_LOCK;
volatile int CALLHIDE_MODE=0;
const char IPC_NAME[] = IPC_XTASK_NAME;

extern unsigned long strtoul (const char *nptr,char **endptr,int base);

CSM_RAM *GetUnderIdleCSM(void) {
    CSM_RAM *u;
    CSM_DESC *UnderIdleDesc;
    if (strlen((char*)CFG_UNDER_IDLE_CONSTR) == 8) {
        UnderIdleDesc = (CSM_DESC*)strtoul((char*)CFG_UNDER_IDLE_CONSTR, 0, 0x10);
    } else {
        UnderIdleDesc = ((CSM_RAM*)(FindCSMbyID(CSM_root()->idle_id))->prev)->constr;
        sprintf((char*)CFG_UNDER_IDLE_CONSTR, "%08X", (unsigned int)UnderIdleDesc);
        SaveConfigData(CONFIG_PATH);
    }
    LockSched();
    u = CSM_root()->csm_q->csm.first;
    while (u && u->constr != UnderIdleDesc) u = u->next;
    UnlockSched();
    return u;
}


int MyIDLECSM_onMessage(CSM_RAM *data, GBS_MSG *msg) {
    int csm_result;
    int icgui_id;
    int idlegui_id;

#ifndef NEWSGOLD
#define EXT_BUTTON 0x63 \
    if ((CFG_ACTIVE_KEY_STYLE != 2) && (CFG_ACTIVE_KEY_STYLE != 3)) {//не "* + #" и не "Enter Button"
        if (CFG_ACTIVE_KEY == EXT_BUTTON) { //мнимая кнопка браузера
            if (msg->msg == 439) { //вызван браузер
                switch (msg->submess) {
                    case 1:
                        GBS_SendMessage(MMI_CEPID, LONG_PRESS, EXT_BUTTON);
                        break;
                    case 2:
                        GBS_SendMessage(MMI_CEPID, KEY_UP, EXT_BUTTON);
                        break; // Никакого default!!!
                }
            }
            else { //браузер не вызывался
                goto L1;
            }
      }
      else { //кнопка вызова не является мнимой кнопкой вызова браузера
          goto L1;
      }
    }
    else {
        L1:
        csm_result = old_onMessage(data, msg);
    }
#else
    csm_result = old_onMessage(data, msg); //Вызываем старый обработчик событий
#endif

    icgui_id = ((int*)data)[DISPLACE_OF_INCOMMINGGUI / 4];
    idlegui_id = ((int*)data)[DISPLACE_OF_IDLEGUI_ID / 4];
    if (!icgui_id) CALLHIDE_MODE = 0;
    if (msg->msg == MSG_RECONFIGURE_REQ) {
        if (strcmpi(CONFIG_PATH, (char *)msg->data0) == 0) {
            ShowMSG(1, (int)"XTask config updated!");
            InitConfig();
        }
    } else
        //IPC
    if (msg->msg == MSG_IPC) {
        if (msg->submess != 392305998) {
            IPC_REQ *ipc;
            if ((ipc = (IPC_REQ*)msg->data0)) {
                if (strcmp(ipc->name_to, IPC_NAME) == 0) {
                    switch (msg->submess) {
                        case IPC_XTASK_SHOW_CSM:
                            if (CSM_ID) break;
                            if ((!IsCalling()) && (!SHOW_LOCK)) {
                                if ((CSM_root()->csm_q->csm.last != data) || IsGuiOnTop(idlegui_id)) {
                                    CSMtoTop((int)(ipc->data), -1);
                                }
                            }
                            break;
                        case IPC_XTASK_IDLE:
                            if (CSM_ID) break;
                            if ((!IsCalling())/*&&(!SHOW_LOCK)*/)
                                CSMtoTop(CSM_root()->idle_id, -1);
                            break;
                        case IPC_XTASK_LOCK_SHOW:
                            SHOW_LOCK++;
                            break;
                        case IPC_XTASK_UNLOCK_SHOW:
                            if (SHOW_LOCK) SHOW_LOCK--;
                            break;
                    }
                }
            }
        }
    } else if (msg->msg == MSG_INCOMMING_CALL) {
        CALLHIDE_MODE = 1;
    }
    if (CALLHIDE_MODE) {
        if ((IsGuiOnTop(icgui_id))/*||(IsGuiOnTop(ocgui_id))*/) {
            CSMtoTop(CSM_root()->idle_id, ((CSM_RAM *)(CSM_root()->csm_q->csm.last))->id);
            CALLHIDE_MODE = 0;
        }
    }
    return csm_result;
}

void MyIDLECSM_onClose(CSM_RAM *data) {
    GBS_DelTimer(&TMR_START);
    RemoveKeybMsgHook((void*)KeyHook);
    kill_elf();
}

void DoSplices(void) {
    SHOW_DAEMONS = CFG_SHOW_DAEMONS;
    LockSched();
    if (!AddKeybMsgHook_end((void*)KeyHook)) {
        ShowMSG(1, (int)"Невозможно зарегистрировать обработчик!");
        SUBPROC((void*)kill_elf);
    } else {
        extern const int CFG_ENA_HELLO_MSG;
        if (CFG_ENA_HELLO_MSG) {
            ShowMSG(1, (int)"XTask3 установлен!");
        }
        {
            CSM_RAM *icsm = FindCSMbyID(CSM_root()->idle_id);
            memcpy(&icsmd, icsm->constr, sizeof(icsmd));
            old_onClose = icsmd.onClose;
            old_onMessage = icsmd.onMessage;
            icsmd.onClose = MyIDLECSM_onClose;
            icsmd.onMessage = MyIDLECSM_onMessage;
            icsm->constr = &icsmd;
        }
        CSM_UNDER_IDLE = GetUnderIdleCSM(); //Ищем idle_dialog
    }
    UnlockSched();
}
  
int main(void) {
  if (InitConfig() != 2) {
      GBS_StartTimerProc(&TMR_START, TMR_SECOND(60), (void*)DoSplices);
  }
  else {
      DoSplices();
  }
    return 0;
}
