//---------------------------------------------------------------------------
#ifndef parserH
#define parserH
//---------------------------------------------------------------------------
//typedef void (* tsend_cb)(void *arg, char *pdata, unsigned short len);

#ifdef _Windows
extern char cfg_name[256];
extern char key_name[256];
extern char crt_name[256];
extern int SaveCfg(void);
#endif

extern int parse(const char *str, char *bres, int len);

#endif
