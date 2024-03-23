#include <swilib.h>
#include <string.h>
#include "idle_csm_hook.h"

CSM_DESC *old_idle_csmd;
CSM_DESC new_idle_csmd;
onmessage_t old_idle_onmessage;
onclose_t old_idle_onclose;

extern int csm_onmessage(CSM_RAM *data, GBS_MSG *msg);
extern void csm_onclose(CSM_RAM *data);

void AddIdleCSMHook() {
    CSM_RAM *icsm = FindCSMbyID(CSM_root()->idle_id);
    old_idle_csmd = icsm->constr;
    old_idle_onmessage = old_idle_csmd->onMessage;
    old_idle_onclose = old_idle_csmd->onClose;
    memcpy(&new_idle_csmd, icsm->constr, sizeof(CSM_DESC));
    new_idle_csmd.onMessage = csm_onmessage;
    new_idle_csmd.onClose = csm_onclose;
    icsm->constr = &new_idle_csmd;
}

void RemoveIdleCSMHook() {
    CSM_RAM *icsm = FindCSMbyID(CSM_root()->idle_id);
    icsm->constr = old_idle_csmd;
    csm_onclose(icsm);
}
