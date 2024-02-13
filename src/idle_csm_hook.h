#ifndef __IDLE_CSM_HOOK_H_
#define __IDLE_CSM_HOOK_H_

#include <swilib.h>

typedef int(*onmessage_t)(CSM_RAM *data, GBS_MSG *msg);
typedef void(*onclose_t)(CSM_RAM *data);

void AddIdleCSMHook();
void RemoveIdleCSMHook();

#endif
