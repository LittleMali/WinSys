#pragma once

#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <shlobj.h>   
#include <shellapi.h>
#include <assert.h>
#include <crtdbg.h>

#pragma warning(disable:4786)   
#include <vector>
#include <list>
#include "tstring_qs.h"

typedef std::list<tstring> StringList;
typedef std::list<int> IntList;

#define  ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))



