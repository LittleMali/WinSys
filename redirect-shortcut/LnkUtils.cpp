#include "stdafx.h"
#include "LnkUtils.h"
#include <shlobj.h>
#include <winbase.h>
#include <shlwapi.h>

BOOL CLnkUtils::ReplaceLnk(LPCTSTR lpDstDir, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath, BOOL bWalkInDir /* = FALSE */)
{
    if (!lpDstDir || !::PathFileExists(lpDstDir))
        return FALSE;

    TCHAR szFindFile[MAX_PATH + 2] = {0};
    _sntprintf_s(szFindFile, MAX_PATH, MAX_PATH, L"%s\\*.*", lpDstDir);

    WIN32_FIND_DATA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = ::FindFirstFile(szFindFile, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    do 
    {
        if (StrCmpI(findData.cFileName, L".") == 0 || StrCmpI(findData.cFileName, L"..") == 0)
        {
            continue;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && bWalkInDir)
        {
            TCHAR szChildDir[MAX_PATH + 2] = {0};
            ::PathCombine(szChildDir, lpDstDir, findData.cFileName);

            ReplaceLnk(szChildDir, lpOldFilePath, lpNewFilePath, bWalkInDir);
        }
        else
        {
            LPTSTR lpExt = ::PathFindExtension(findData.cFileName);
            if (lpExt && StrCmpI(lpExt, L".lnk") == 0)
            {
                TCHAR szLnkFile[MAX_PATH + 2] = {0};
                ::PathCombine(szLnkFile, lpDstDir, findData.cFileName);

                CheckLnk(szLnkFile, lpOldFilePath, lpNewFilePath);
            }
        }

    } while (::FindNextFile(hFind, &findData));

    ::FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    return TRUE;
}

BOOL CLnkUtils::CheckLnk(LPCTSTR lpLnk, LPCTSTR lpOldFilePath, LPCTSTR lpNewFilePath)
{
    if (!lpLnk)
        return FALSE;

    HRESULT hres = 0;
    IShellLink* psl = NULL;
    IPersistFile* ppf = NULL;

    TCHAR szGetRawPath[MAX_PATH + 2] = {0}, szExpandPath[MAX_PATH + 2] = {0};
    WIN32_FIND_DATA wfd;

    // Get a pointer to the IShellLink interface.
    hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
    if (FAILED(hres) || !psl)
    {
        goto Exit0;
    }

    // Get a pointer to the IPersistFile interface. 
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hres) || !ppf)
    {
        goto Exit0;
    }

    // Load the shortcut. 
    hres = ppf->Load(lpLnk, STGM_READWRITE);
    if (FAILED(hres))
    {
        goto Exit0;
    }

    // Get the path to the link target. 
    hres = psl->GetPath(szGetRawPath, MAX_PATH, &wfd, SLGP_RAWPATH);
    if (FAILED(hres))
    {
        goto Exit0;
    }

    if (0 == ::ExpandEnvironmentStrings(szGetRawPath, szExpandPath, MAX_PATH))
    {
        goto Exit0;
    }

    if (StrCmpI(szExpandPath, lpOldFilePath) != 0)
    {
        goto Exit0;
    }

    // we find target lnk, replace it
    std::wcout << L"match and redirect lnk: " << lpLnk << std::endl;

    hres = psl->SetPath(lpNewFilePath);
    if (FAILED(hres))
    {
        goto Exit0;
    }

    hres = ppf->Save(NULL, 0);
    if (FAILED(hres))
    {
        goto Exit0;
    }

Exit0:
    if (ppf)
    {
        ppf->Release();
        ppf = NULL;
    }

    if (psl)
    {
        psl->Release();
        psl = NULL;
    }

    return TRUE;
}
