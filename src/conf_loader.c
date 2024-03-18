#include <swilib.h>
#include <stdlib.h>
#include <cfg_items.h>
#include <string.h>
#include "conf_loader.h"

char *CONFIG_PATH = "";

size_t GetConfigSize(CFG_HDR *cfghdr0, int *__config_begin, int *__config_end) {
    volatile unsigned int _segb = (volatile unsigned int)__config_begin;
    volatile unsigned int _sege = (volatile unsigned int)__config_end;
    return (_sege - _segb) - 4;
}

int SaveConfigData(const char *path) {
    unsigned int err = 0;
    int fp = _open(path, A_ReadWrite + A_Create + A_Truncate,
               P_READ + P_WRITE, &err);
    if (fp != -1) {
        __CONFIG_EXTERN(0, cfghdr0);
        size_t size = GetConfigSize(__CONFIG(0, cfghdr0));
        int wsize = _write(fp, &cfghdr0, (int)size, &err);
        _close(fp, &err);
        if (wsize == size) {
            return 1;
        }
        _close(fp, &err);
    }
    return 0;
}


int LoadConfigData(const char *path, CFG_HDR *cfghdr0, int *__config_begin, int *__config_end)
{
    int result = 0;
    int fp;
    unsigned int err;
    size_t size = GetConfigSize(cfghdr0, __config_begin, __config_end);
    char *buf = malloc(size);
    if (buf) {
        fp = _open(path, A_ReadOnly + A_BIN, 0, &err);
        if (fp  != -1) {
            if (_read(fp, buf, (int)size, &err) == size) {
                memcpy(cfghdr0, buf, size);
                _close(fp, &err);
                result = 2;
            } else {
                _close(fp, &err);
                goto L_SAVENEWCFG;
            }
        } else {
            L_SAVENEWCFG:
            result = SaveConfigData(path);
        }
        mfree(buf);
    }
    if (result > 0) {
        CONFIG_PATH = (char*)path;
    }
    return result;
}


int InitConfig() {
    __CONFIG_EXTERN(0, cfghdr0);
    int result = LoadConfigData("0:\\zbin\\etc\\SieXTask.bcfg", __CONFIG(0, cfghdr0));
    if (!result) {
        result = LoadConfigData("4:\\zbin\\etc\\SieXTask.bcfg", __CONFIG(0, cfghdr0));
    }
    return result;
}
