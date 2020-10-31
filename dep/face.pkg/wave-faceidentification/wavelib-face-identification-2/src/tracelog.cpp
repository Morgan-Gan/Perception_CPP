//---------------------------------------------------------------------------
/*Behind line is added by yaguo to use trace log*/

#include "tracelog.h"

long g_nTraceSize  = 5 * 1024 * 1024;

//---------------------------------------------------------------------------
void tracelog(char* filename, int level,char *str,...)
{
	try
	{
		FILE *fp = NULL;
		va_list vl;
		char *tmpstr = NULL;
		int len;
		int i;

		time_t t;
		struct tm *st = NULL;
		int year,month,day,hour,min,sec;

		if(NULL==str)
		{
			return;
		}
		tmpstr=strdup(str);
		if(NULL==tmpstr)
		{
			return;
		}


		time(&t);
		st=localtime(&t);
		
		year=st->tm_year+1900;
		month=st->tm_mon+1;
		day=st->tm_mday;
		hour=st->tm_hour;
		min=st->tm_min;
		sec=st->tm_sec;

		fp=fopen(filename,"a+");
		if(NULL==fp)
		{
			return;
		}
		
		trimall(tmpstr);
		len=strlen(tmpstr);
		for(i=len-1;i>=0;i--)
		{
			if(tmpstr[i]=='\n')
			{
				tmpstr[i]='\0';
			}
			else
			{
				break;
			}
		}

		
		fseek (fp,0,SEEK_END);
		long  nSize = ftell(fp);
                if ( nSize >= g_nTraceSize) //dwTraceSize 1M
		{
			fclose(fp);
                        char filenametemp[256];
                        sprintf(filenametemp, "%s%04d%02d%02d_%02d%02d%02d", filename, year,month,day,hour,min,sec);
                        rename(filename, filenametemp);

			// 重新创建
			fp = fopen (filename, "w"); 
			if ( fp == NULL)
				return ;
			fclose(fp);
			// 重新打开
			fp = fopen (filename, "a+"); 
			if ( fp == NULL)
				return ;
		}
		
		fprintf(fp,"[");
		va_start(vl,str);
		switch(level)
		{
		case LOG_ERR:
			fprintf(fp,"-ERROR-");
			break;
    		case LOG_WARNING:
			fprintf(fp,"WARNING");
			break;
		case LOG_INFO:
			fprintf(fp,"--INFO-");
			break;
		case LOG_DEBUG:
			fprintf(fp,"-DEBUG-");
			break;
		default:
			fprintf(fp,"UNKNOWN");
			break;
		}
		fprintf(fp,"]");
		fprintf(fp,"[%04d%02d%02d %02d:%02d:%02d]",year,month,day,hour,min,sec);
		vfprintf(fp,tmpstr,vl);
		fprintf(fp,"\n");
		va_end(vl);

		fclose(fp);

		if (tmpstr != NULL)
		{
			free(tmpstr);
			tmpstr = NULL;
		}
	}
	catch (...)
	{
		return;
	}
}
//---------------------------------------------------------------------------
void trimleft(char *str)
{
	unsigned int i;
	unsigned int j;

	for(i=0;;i++)
	{
		if(*(str+i)!=' ')
			break;
	}

	if(i==0)
	{
		return;
	}

	for(j=0;;j++)
	{
		str[j]=str[i+j];
		if(str[j]=='\0')
			break;
	}
}
//---------------------------------------------------------------------------
void trimright(char *str)
{
	int len;
	int i;

	len=strlen(str);
	for(i=len-1;i>=0;i--)
	{
		if(str[i]==' ')
		{
			str[i]='\0';

		}
		else
		{
			break;
		}
	}
}
//---------------------------------------------------------------------------
void trimall(char *str)
{
	trimleft(str);
	trimright(str);
}
//---------------------------------------------------------------------------

/*
int mkdir_recursive( char* path )
{
	char *p;
	if( access( path, 0 ) == 0 )
		return 0;
	for( p=path; *p; p++ ){
		if( p>path && *p == '/' ){
			*p = 0;
			if( access( path, 0 ) != 0 ){
				if( mkdir( path, S_IRWXU ) != 0 )
					return -1;
			}
			*p = '/';
		}
	}
	return mkdir( path, S_IRWXU );
}
*/
