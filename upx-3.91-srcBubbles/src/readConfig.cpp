#include "readConfig.h"

char inifile[500];

void setinifile(char * nombre)
{
	strcpy(inifile,nombre);
}
int readConfigurationS(char* field,const char *instance,char *buffer,int buffersize)
{
	int n=0;
	n = ini_gets(field,instance, "", buffer, buffersize, inifile);
	if(n < 1)
		return CONFIG_ERROR;
	return CONFIG_OK;
}
int readConfigurationI(char *field,const char *instance,int *buffer)
{
	int n=0;
	n = ini_getl(field, instance, -1, inifile);
	if(n < 0)
		return CONFIG_ERROR;
	*(buffer)=n;
	return CONFIG_OK;
}  
int readConfigurationF(char *field,const char *instance,float *buffer)
{
	float n=0.0;
	n = ini_getf(field, instance, -1.0, inifile);
	if(n < 0)
		return CONFIG_ERROR;
	*(buffer)=n;
	return CONFIG_OK;
}  
