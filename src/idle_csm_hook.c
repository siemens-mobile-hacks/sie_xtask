#include <swilib.h>
#include "idle_csm_hook.h"

CSM_DESC *old_idle_csmd;
CSM_DESC new_idle_csmd;

void AddIdleCSMHook() {
    CSM_RAM *icsm = FindCSMbyID(CSM_root()->idle_id);
    old_idle_csmd = icsm->constr;
    memcpy(&new_idle_csmd, icsm->constr, sizeof(CSM_DESC));
    icsm->constr = &new_idle_csmd;
}

void RemoveIdleCSMHook() {
    extern void csm_onclose(CSM_RAM *data);

    CSM_RAM *icsm = FindCSMbyID(CSM_root()->idle_id);
    icsm->constr = old_idle_csmd;
    csm_onclose(icsm);
}
