#pragma once
#include "common.h"

namespace engine
{
	enum EntryType
	{
		CENTRY_TYPE_FLOAT,
		CENTRY_TYPE_INT,
		CENTRY_TYPE_BOOL,
		CENTRY_TYPE_STRING,
	};

	struct ConfigEntry
	{
		const char* category;
		const char* name;
		EntryType type;
		float	fValue;
		int		iValue;
		bool	bValue;
		char*	sValue;
	};

	struct Config
	{
		int entryCount;
		ConfigEntry* entries[];
	};

	ENGINE_API int parse_ini(void* user, const char* section, const char* name, const char* value);

	ENGINE_API void ini_open(const char* filename, Config* config);

}