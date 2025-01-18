#include "config.h"

#include <string.h>
#include <stdlib.h>

#define INI_ALLOW_NO_VALUE 1
#define INI_MAX_LINE 1024
#define INI_ALLOW_MULTILINE 0
#define INI_STOP_ON_FIRST_ERROR 1
#define INI_INITIAL_ALLOC 512
#include "ini/ini.h"

namespace engine
{
	ENGINE_API int parse_ini(void* user, const char* section, const char* name, const char* value)
	{

		Config* pconfig = (Config*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define FLOAT(v) (float)atof(v)
#define INT(v) atoi(v)

		for (UINT32 i = 0; i < pconfig->entryCount; i++)
		{
			ConfigEntry* entry = pconfig->entries[i];
			if (MATCH(entry->category, entry->name))
			{
				switch (entry->type)
				{
				case CENTRY_TYPE_FLOAT:
				{
					entry->fValue = FLOAT(value);
				}break;
				case CENTRY_TYPE_INT:
				{
					entry->iValue = INT(value);
				}break;
				case CENTRY_TYPE_BOOL:
				{
					bool b = false;
					if (value[0] == '0')
						b = false;
					else if (value[0] == '1')
						b = true;
					else if (strcmp(value, "true") == 0)
						b = true;
					else if (strcmp(value, "false") == 0)
						b = false;
					entry->bValue = b;
				}break;
				case CENTRY_TYPE_STRING:
				{
					int len = strlen(value);
					strcpy_s(entry->sValue, len, value);
				}break;
				default:
					break;
				}
			}
		}

		return 1;
	}

	ENGINE_API void ini_open(const char* filename, Config* config)
	{
		FILE* file;
		fopen_s(&file, filename, "r");
		if (file != NULL)
		{
			ini_parse_file(file, parse_ini, config);
			printf("config found, loading values.\n");
		}
		else
		{
			// default values here
			printf("config not found, loading defaults.\n");
		}
	}
}