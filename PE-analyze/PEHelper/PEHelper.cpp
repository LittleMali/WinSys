// PEHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PEUtils.h"
#include <string>
#include <algorithm>


#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib,"ImageHlp")

#define PROXY_ACC_DLL_32       L"QMProxyAccLspBak.dll"

HINSTANCE g_hDll32 = NULL;

DWORD MemoryRVAToFileOffset(DWORD dwImageBase, DWORD dwMemRVA);
void PrintImportDesc(DWORD dwImageBase, PIMAGE_IMPORT_DESCRIPTOR pImportDescArr, int nSize);
void PrintExportDir(DWORD dwImageBase, PIMAGE_EXPORT_DIRECTORY pExportDirArr, int nSize);

BOOL __stdcall WSPHook(WSPPROC_TABLE *lpProcTable);
typedef BOOL (__stdcall *PfnWSPHook)(WSPPROC_TABLE *lpProcTable);

int main()
{
    //test();
    HMODULE hCurProc = ::GetModuleHandle(NULL);

    wchar_t szDir[MAX_PATH] = { 0 }, szDllPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szDir, MAX_PATH);
    ::PathRemoveFileSpec(szDir);
    ::PathCombine(szDllPath, szDir, PROXY_ACC_DLL_32);
    HMODULE hDll = ::LoadLibraryW(szDllPath);
    if (!hDll)
        return 0;

    HMODULE hProc = hDll;
    PBYTE pImgBase = (PBYTE)hProc;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hProc;

    PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(pImgBase + pDosHeader->e_lfanew);
    PIMAGE_FILE_HEADER pNtFileHeader = &(pNtHeader->FileHeader);
    PIMAGE_OPTIONAL_HEADER pNtOptionalHeader = &(pNtHeader->OptionalHeader);
    PIMAGE_DATA_DIRECTORY pDataDirectoryArr = &(pNtOptionalHeader->DataDirectory[0]);
    
    PIMAGE_SECTION_HEADER pSectionHeaderArr = (PIMAGE_SECTION_HEADER)(IMAGE_FIRST_SECTION(pNtHeader));
    int nNtHeaderSize = sizeof(IMAGE_NT_HEADERS);
    PIMAGE_SECTION_HEADER pSectionHeaderArr1 = (PIMAGE_SECTION_HEADER)(pImgBase + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

    DWORD dwRva = pDataDirectoryArr[1].VirtualAddress;
    DWORD dwFileOffset = MemoryRVAToFileOffset((DWORD)pImgBase, (DWORD)dwRva);

    //////////////////////////////////////////////////////////////////////////
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pImgBase + pDataDirectoryArr[1].VirtualAddress);
    int nImportSize = pDataDirectoryArr[1].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
    // Note: nSize already minus 1, exclude last null element
    PrintImportDesc((DWORD)pImgBase, pImportDesc, nImportSize);

    //////////////////////////////////////////////////////////////////////////
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(pImgBase + pDataDirectoryArr[0].VirtualAddress);
    // Note: nSize already minus 1, excude last null element
    int nExportSize = pDataDirectoryArr[0].Size / sizeof(IMAGE_EXPORT_DIRECTORY) - 1;
    PrintExportDir((DWORD)pImgBase, pExportDir, nExportSize);

    PfnWSPHook pfnHook = (PfnWSPHook)GetProcAddress(hDll, "WSPHook");
    printf("pfnHook=%08x\n", pfnHook);
    printf("pfnHook RVA=%08x\n", (DWORD)pfnHook - (DWORD)hDll);

    return 0;
}

DWORD MemoryRVAToFileOffset(DWORD dwImageBase, DWORD dwMemRVA)
{
    DWORD dwFileOffset = 0;

    PBYTE pImgBase = (PBYTE)dwImageBase;
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)dwImageBase;
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(pImgBase + pDosHdr->e_lfanew);
    PIMAGE_SECTION_HEADER pSectionHdrArr = (PIMAGE_SECTION_HEADER)(pImgBase + pDosHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS));
    
    for (int i = 0; i < pNtHdr->FileHeader.NumberOfSections; ++i)
    {
        if (pSectionHdrArr[i].VirtualAddress <= dwMemRVA &&
            dwMemRVA < (pSectionHdrArr[i].VirtualAddress + pSectionHdrArr[i].SizeOfRawData))
        {
            DWORD dwOffset = dwMemRVA - pSectionHdrArr[i].VirtualAddress;
            dwFileOffset = pSectionHdrArr[i].PointerToRawData + dwOffset;
            break;
        }
    }

    //for (PIMAGE_SECTION_HEADER pIter = pSectionHdrArr; pIter != NULL; ++pIter)
    //{
    //    if (pIter->VirtualAddress <= dwMemRVA &&
    //        dwMemRVA < (pIter->VirtualAddress + pIter->SizeOfRawData))
    //    {
    //        DWORD dwOffset = dwMemRVA - pIter->VirtualAddress;
    //        dwFileOffset = pIter->PointerToRawData + dwOffset;
    //        break;
    //    }
    //}

    return dwFileOffset;
}

void PrintImportDesc(DWORD dwImageBase, PIMAGE_IMPORT_DESCRIPTOR pImportDescArr, int nSize)
{
    //for (int i = 0; i < nSize; ++i)
    //{
    //  pImportDescArr[i]
    //}
    
    for (PIMAGE_IMPORT_DESCRIPTOR pIter = pImportDescArr; pIter->Name != NULL; ++pIter)
    {
        // 循环结束的条件是：最后一个元素的内容全0 。
        char* pDllName = (char *)(dwImageBase + pIter->Name);
        printf("dll name: %s\n", pDllName);

        PIMAGE_THUNK_DATA pOriginFirst = (PIMAGE_THUNK_DATA)(dwImageBase + pIter->OriginalFirstThunk);
        for (; pOriginFirst->u1.Ordinal != 0; ++pOriginFirst)
        {
            // 循环结束的条件是：最后一个元素的内容全0 。
            if (IMAGE_SNAP_BY_ORDINAL32(pOriginFirst->u1.Ordinal))
            {
                printf("func hint: %p\n", pOriginFirst->u1.Ordinal & IMAGE_ORDINAL_FLAG32);
            }
            else
            {
                PIMAGE_IMPORT_BY_NAME pFuncImport = (PIMAGE_IMPORT_BY_NAME)(dwImageBase + pOriginFirst->u1.ForwarderString);
                DWORD dwFuncHint = pFuncImport->Hint;
                char* pFuncName = pFuncImport->Name;
                printf("func hint: %08x, name: %s\n", dwFuncHint, pFuncName);

                DWORD dwFileOff = MemoryRVAToFileOffset(dwImageBase, pOriginFirst->u1.Ordinal);
                printf("file offset: %d\n", dwFileOff);
            }
        }

        PIMAGE_THUNK_DATA pFirst = (PIMAGE_THUNK_DATA)(dwImageBase + pIter->FirstThunk);
        for (; pFirst->u1.Function != 0; ++pFirst)
        {
            // 循环结束的条件是：最后一个元素的内容全0 。
            printf("func addr: %08x\n", pFirst->u1.Function);
        }
    }
}

void PrintExportDir(DWORD dwImageBase, PIMAGE_EXPORT_DIRECTORY pExportDirArr, int nSize)
{
    for (int i = 0; i < nSize; ++i)
    {
        char* pName = (char*)(dwImageBase + pExportDirArr[i].Name);
        printf("dll name: %s\n", pName);
        printf("base: %d\n", pExportDirArr[i].Base);
        printf("export fun total cnt: %d\n", pExportDirArr[i].NumberOfFunctions);
        printf("export fun names cnt: %d\n", pExportDirArr[i].NumberOfNames);

        DWORD* pFuncAddrArr = (DWORD*)(dwImageBase + pExportDirArr[i].AddressOfFunctions);
        for (int j = 0; j < pExportDirArr[i].NumberOfFunctions; ++j)
        {
            printf("export fun entry addr: %08x\n", pFuncAddrArr[j]);
        }

        DWORD* pFuncNameArr = (DWORD*)(dwImageBase + pExportDirArr[i].AddressOfNames);
        WORD* pFuncNameOridArr = (WORD*)(dwImageBase + pExportDirArr[i].AddressOfNameOrdinals);
        for (int k = 0; k < pExportDirArr[i].NumberOfNames; ++k)
        {
            char* pFuncName = (char*)(dwImageBase + pFuncNameArr[k]);
            printf("export fun name: %s\n", pFuncName);
            printf("export fun name ordinal: %d\n", pFuncNameOridArr[k] + pExportDirArr[i].Base);
        }

        int ss = 0;
        ss++;
    }
}

