//#include <swilib.h>
#include <cfg_items.h>

__CFG_BEGIN(0)

__root CFG_HDR cfghdr0 = {CFG_STR_UTF8, "CSM list file", 3, 127};
#ifdef NEWSGOLD
__root char CFG_CSMLIST_PATH[128] = "4:\\ZBin\\etc\\CSMlist.txt";
#else
__root char CFG_CSMLIST_PATH[128] = "0:\\ZBin\\etc\\CSMlist.txt";
#endif

__root CFG_HDR cfghdr1 = {CFG_CBOX,"Enable hello message",0,2};
__root int CFG_ENA_HELLO_MSG = 1;
__root CFG_CBOX_ITEM cfgcbox1_1[2] = {{"Yes"}, {"No"}};

__root CFG_HDR cfghdr2 = {CFG_STR_WIN1251, "UnderIdleCSM_DESC", 0, 8};
__root char CFG_UNDER_IDLE_CONSTR[9] = "";

__root CFG_HDR cfghdr3 = {CFG_CBOX, "Activation style", 0, 4};
__root int CFG_ACTIVE_KEY_STYLE = 0;
__root CFG_CBOX_ITEM cfgcbox3_1[4] = {
        {"Short press"}, {"Long press"},
        {"* + #"},{"Enter Button"},
};

__root CFG_HDR cfghdr4 = {CFG_KEYCODE, "Activation key", 0, 99};
#ifndef NEWSGOLD
__root int CFG_ACTIVE_KEY = 99;
#else
    #ifdef ELKA
    __root int CFG_ACTIVE_KEY = 0x15;
    #else
    __root int CFG_ACTIVE_KEY = 0x11;
    #endif
#endif

__root CFG_HDR cfghdr5 = {CFG_CBOX, "Extra: on longpress", 0, 5};
__root int CFG_ENA_LONG_PRESS = 2;
__root CFG_CBOX_ITEM cfgcbox5_1[5] = {
        {"As in FW"}, {"Do nothing"},
        {"Hide all tasks"}, {"Launch XTask"}, {"Hide & Lock"},
};

#ifdef NEWSGOLD
__root CFG_HDR cfghdr6 = {CFG_CBOX, "Red button mode", 0, 3};
__root int CFG_RED_BUT_MODE = 2;
__root CFG_CBOX_ITEM cfgcbox6_1[3] = {
        {"As in FW"}, {"Classic"}, {"Hide all tasks"}
};
#endif

__root CFG_HDR cfghdr7 = {CFG_CBOX, "Enable if keyboard locked", 0, 2};
__root int CFG_ENA_LOCK = 1;
__root CFG_CBOX_ITEM cfgcbox7_1[2]={{"No"}, {"Yes"}};

__root CFG_HDR cfghdr8 = {CFG_CBOX, "Show NewStyleDaemons", 0, 2};
__root int CFG_SHOW_DAEMONS = 1;
__root CFG_CBOX_ITEM cfgcbox8_1[2]={{"No"}, {"Yes"}};

__root CFG_HDR cfghdr9 = {CFG_CBOX, "Show IDLE screen", 0, 2};
__root int CFG_SHOW_IDLESCREEN = 1;
__root CFG_CBOX_ITEM cfgcbox9_1[2]={{"No"}, {"Yes"}};

__root CFG_HDR cfghdr10 = {CFG_STR_WIN1251, "Bookmark 1 name", 1, 31};
__root char CFG_BM1NAME[32] = "";
__root CFG_HDR cfghdr11 = {CFG_STR_UTF8, "Bookmark 1 file", 3, 127};
__root char CFG_BM1FILE[128]="";

__root CFG_HDR cfghdr12 = {CFG_STR_WIN1251, "Bookmark 2 name", 1, 31};
__root char CFG_BM2NAME[32] = "";
__root CFG_HDR cfghdr13 = {CFG_STR_UTF8, "Bookmark 2 file", 3, 127};
__root char CFG_BM2FILE[128] = "";

__root CFG_HDR cfghdr14 = {CFG_STR_WIN1251, "Bookmark 3 name", 1, 31};
__root char CFG_BM3NAME[32] = "";
__root CFG_HDR cfghdr15 = {CFG_STR_UTF8, "Bookmark 3 file", 3, 127};
__root char CFG_BM3FILE[128] = "";

__root CFG_HDR cfghdr16 = {CFG_STR_WIN1251, "Bookmark 4 name", 1, 31};
__root char CFG_BM4NAME[32] = "";
__root CFG_HDR cfghdr17 = {CFG_STR_UTF8, "Bookmark 4 file", 3, 127};
__root char CFG_BM4FILE[128] = "";

__root CFG_HDR cfghdr18 = {CFG_STR_WIN1251, "Bookmark 5 name", 1, 31};
__root char CFG_BM5NAME[32] = "";
__root CFG_HDR cfghdr19 = {CFG_STR_UTF8, "Bookmark 5 file", 3, 127};
__root char CFG_BM5FILE[128] = "";

__root CFG_HDR cfghdr20 = {CFG_STR_WIN1251, "Bookmark 6 name", 1, 31};
__root char CFG_BM6NAME[32] = "";
__root CFG_HDR cfghdr21 = {CFG_STR_UTF8, "Bookmark 6 file", 3, 127};
__root char CFG_BM6FILE[128] = "";

__root CFG_HDR cfghdr22 = {CFG_STR_WIN1251, "Bookmark 7 name", 1, 31};
__root char CFG_BM7NAME[32] = "";
__root CFG_HDR cfghdr23 = {CFG_STR_UTF8, "Bookmark 7 file", 3, 127};
__root char CFG_BM7FILE[128] = "";

__root CFG_HDR cfghdr24 = {CFG_STR_WIN1251, "Bookmark 8 name", 1, 31};
__root char CFG_BM8NAME[32] = "";
__root CFG_HDR cfghdr25 = {CFG_STR_UTF8, "Bookmark 8 file", 3, 127};
__root char CFG_BM8FILE[128] = "";

__root CFG_HDR cfghdr26 = {CFG_STR_WIN1251, "Bookmark 9 name", 1, 31};
__root char CFG_BM9NAME[32] = "";
__root CFG_HDR cfghdr27 = {CFG_STR_UTF8, "Bookmark 9 file", 3, 127};
__root char CFG_BM9FILE[128] = "";

__CFG_END(0)
