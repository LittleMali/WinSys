#ifndef _LNK_UTILS_H_
#define _LNK_UTILS_H_

#include <string>
#include <iostream>
#include <shlobj.h>
#include <shlwapi.h>
#include <winbase.h>

class CLnkUtils
{
public:
    BOOL ReplaceLnk(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, BOOL bWalkInDir = FALSE);

private:
    BOOL CheckLnk(LPCTSTR lpLnk, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath);

};

#endif
