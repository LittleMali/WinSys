#ifndef _LNK_UTILS_H_
#define _LNK_UTILS_H_

#include "shobjidl.h"
#include "shlguid.h"
#include <string>

class CLnkUtils
{
public:
    BOOL ReplaceLnk(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, BOOL bWalkInDir = FALSE);

private:
    BOOL CheckLnk(LPCTSTR lpLnk, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath);

};

#endif
