#ifndef _READCONFIG
#define _READCONFIG

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "minIni.h"

#define CONFIG_ERROR -1
#define CONFIG_OK 0

int readConfigurationS(char* field,const char *instance,char *buffer,int buffersize);
int readConfigurationI(char *field,const char *instance,int *buffer);
int readConfigurationF(char *field,const char *instance,float *buffer);

void setinifile(char * nombre);
#endif /*_READCONFIG*/
