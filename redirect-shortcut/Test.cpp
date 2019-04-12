// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LnkUtils.h"

#define MAIN_EXE_FILE								_T("QMProxyAcceler.exe")

BOOL RedirectShortcutAtDesktop(const std::wstring& strOld, const std::wstring& strNew)
{
    // 遍历公共/用户桌面，不递归文件夹
    std::wstring strOldPointFile = strOld;
    std::wstring strNewPointFile = strNew;

    TCHAR szAllUserDesktop[MAX_PATH + 2] = {0};
    TCHAR szCurrentDesktop[MAX_PATH + 2] = {0};
    ::SHGetSpecialFolderPath(NULL, szAllUserDesktop, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
    ::SHGetSpecialFolderPath(NULL, szCurrentDesktop, CSIDL_DESKTOPDIRECTORY, FALSE);

    CLnkUtils lnkUtils;

    std::wcout << L"begin search: " << szAllUserDesktop << std::endl;
    lnkUtils.ReplaceLnk(szAllUserDesktop, strOldPointFile.c_str(), strNewPointFile.c_str(), FALSE);

    std::wcout << L"begin search: " << szCurrentDesktop << std::endl;
    lnkUtils.ReplaceLnk(szCurrentDesktop, strOldPointFile.c_str(), strNewPointFile.c_str(), FALSE);

    return TRUE;
}


BOOL RedirectShortcutAtQuickLaunch(const std::wstring& strOld, const std::wstring& strNew)
{
    // 快速启动和任务栏
    TCHAR szDstDir[MAX_PATH + 2] = {0};
    ::SHGetSpecialFolderPath(NULL, szDstDir, CSIDL_APPDATA, FALSE);
    ::PathAppend(szDstDir, L"Microsoft\\Internet Explorer\\Quick Launch\\User Pinned");

    TCHAR szStartMenu[MAX_PATH + 2] = {0};
    TCHAR szTaskBar[MAX_PATH + 2] = {0};
    ::PathCombine(szStartMenu, szDstDir, L"StartMenu");
    ::PathCombine(szTaskBar, szDstDir, L"TaskBar");

    if (!::PathFileExists(szStartMenu) || !::PathFileExists(szTaskBar))
        return FALSE;

    std::wstring strOldPointFile = strOld;
    std::wstring strNewPointFile = strNew;

    CLnkUtils lnkUtils;

    std::wcout << L"begin search: " << szStartMenu << std::endl;
    lnkUtils.ReplaceLnk(szStartMenu, strOldPointFile.c_str(), strNewPointFile.c_str(), FALSE);

    std::wcout << L"begin search: " << szTaskBar << std::endl;
    lnkUtils.ReplaceLnk(szTaskBar, strOldPointFile.c_str(), strNewPointFile.c_str(), FALSE);

    return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
    std::locale::global(std::locale(""));
    
    std::wcout << L"*** Start ***" << std::endl;

    ::CoInitialize(NULL);

    TCHAR szIniFile[MAX_PATH + 2] = L"C:\\ShortcutTest.ini";

    WCHAR szLnkExeFile[MAX_PATH + 2] = { 0 };
    WCHAR szRedirectLnkExe[MAX_PATH + 2] = { 0 };

    GetPrivateProfileString(L"dir", L"OriginalLnkExe", L"", szLnkExeFile, MAX_PATH, szIniFile);
    GetPrivateProfileString(L"dir", L"RedirectLnkExe", L"", szRedirectLnkExe, MAX_PATH, szIniFile);

    if (wcslen(szLnkExeFile) == 0 || wcslen(szRedirectLnkExe) == 0)
    {
        std::wcout << L"Error: check C:\\ShortcutTest.ini file" << std::endl;
        goto Exit0;
    }

    RedirectShortcutAtDesktop(szLnkExeFile, szRedirectLnkExe);
    std::wcout << std::endl;

    RedirectShortcutAtQuickLaunch(szLnkExeFile, szRedirectLnkExe);
    std::wcout << std::endl;
    
    // 刷新桌面
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);

    ::CoUninitialize();

Exit0:
    std::wcout << L"*** End ***" << std::endl << std::endl;
    printf("press any key to return.");

    getchar();

    return 0;  
}

