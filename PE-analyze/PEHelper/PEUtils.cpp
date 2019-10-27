#include "stdafx.h"
#include "PEUtils.h"

MAP_FILE_STRUCT MapFile;

BOOL LoadFile(LPTSTR lpFilename, PMAP_FILE_STRUCT pstMapFile)
{

    HANDLE hFile;
    HANDLE hMapping;
    LPVOID ImageBase;

    memset(pstMapFile, 0, sizeof(MAP_FILE_STRUCT));

    hFile = CreateFile(lpFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 0);

    if (!hFile)
        return FALSE;

    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping)
    {
        CloseHandle(hFile);
        return FALSE;
    }
    ImageBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!ImageBase)
    {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return FALSE;
    }
    pstMapFile->hFile = hFile;
    pstMapFile->hMapping = hMapping;
    pstMapFile->ImageBase = ImageBase;
    return TRUE;
}

void UnLoadFile(PMAP_FILE_STRUCT pstMapFile)
{
    if (pstMapFile->ImageBase)
        UnmapViewOfFile(pstMapFile->ImageBase);

    if (pstMapFile->hMapping)
        CloseHandle(pstMapFile->hMapping);

    if (pstMapFile->hFile)
        CloseHandle(pstMapFile->hFile);

}


BOOL IsPEFile(LPVOID ImageBase)
{
    PIMAGE_DOS_HEADER pDH = NULL;
    PIMAGE_NT_HEADERS pNtH = NULL;

    if (!ImageBase)
        return FALSE;

    pDH = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDH->e_magic != IMAGE_DOS_SIGNATURE)
        return FALSE;

    pNtH = (PIMAGE_NT_HEADERS32)((DWORD)pDH + pDH->e_lfanew);
    if (pNtH->Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    return TRUE;

}

PIMAGE_NT_HEADERS GetNtHeaders(LPVOID ImageBase)
{

    if (!IsPEFile(ImageBase))
        return NULL;
    PIMAGE_NT_HEADERS pNtH;
    PIMAGE_DOS_HEADER pDH;
    pDH = (PIMAGE_DOS_HEADER)ImageBase;
    pNtH = (PIMAGE_NT_HEADERS)((DWORD)pDH + pDH->e_lfanew);

    return pNtH;

}

PIMAGE_FILE_HEADER   GetFileHeader(LPVOID ImageBase)
{
    PIMAGE_NT_HEADERS pNtH;
    PIMAGE_FILE_HEADER pFH = NULL;
    pNtH = GetNtHeaders(ImageBase);
    pFH = &pNtH->FileHeader;
    return pFH;
}

PIMAGE_OPTIONAL_HEADER GetOptionalHeader(LPVOID ImageBase)
{
    PIMAGE_OPTIONAL_HEADER pOH = NULL;

    pOH = &(GetNtHeaders(ImageBase))->OptionalHeader;
    return pOH;
}

PIMAGE_SECTION_HEADER GetFirstSectionHeader(LPVOID ImageBase)
{
    PIMAGE_NT_HEADERS     pNtH = NULL;
    PIMAGE_SECTION_HEADER pSH = NULL;

    pNtH = GetNtHeaders(ImageBase);
    pSH = IMAGE_FIRST_SECTION(pNtH);
    return pSH;
}

LPVOID RvaToPtr(PIMAGE_NT_HEADERS pNtH, LPVOID ImageBase, DWORD dwRVA)
{
    return ::ImageRvaToVa(pNtH, ImageBase, dwRVA, NULL);

}

LPVOID GetDirectoryEntryToData(LPVOID ImageBase, USHORT DirectoryEntry)
{
    DWORD dwDataStartRVA;
    LPVOID pDirData = NULL;
    PIMAGE_NT_HEADERS     pNtH = NULL;
    PIMAGE_OPTIONAL_HEADER pOH = NULL;

    pNtH = GetNtHeaders(ImageBase);
    if (!pNtH)
        return NULL;
    pOH = GetOptionalHeader(ImageBase);
    if (!pOH)
        return NULL;
    dwDataStartRVA = pOH->DataDirectory[DirectoryEntry].VirtualAddress;
    if (!dwDataStartRVA)
        return NULL;

    pDirData = RvaToPtr(pNtH, ImageBase, dwDataStartRVA);
    if (!pDirData)
        return NULL;
    return pDirData;
}

void PrintFileHeader(PIMAGE_FILE_HEADER pFileHeader)
{
    tm *createTime;
    switch (pFileHeader->Machine)
    {
    case 0X14C:
        printf("可执行文件的目标CPU为 Intel i386\n");
        break;
    case 0X162:
        printf("可执行文件的目标CPU为 MIPS R3000\n");
        break;
    case 0x166:
        printf("可执行文件的目标CPU为 MIPS R4000\n");
        break;
    case 0x184:
        printf("可执行文件的目标CPU为 Alpha AXP\n");
        break;
    case 0x1F0:
        printf("可执行文件的目标CPU为 Power PC\n");
        break;
    default:printf("可执行文件的目标CPU未知\n");

    }
    printf("文件的区块数目:%d\n", pFileHeader->NumberOfSections);
    //createTime = gmtime((time_t *)&pFileHeader->TimeDateStamp);
    //printf("文件的创建时间GMT is: %s\n", asctime(createTime));
    printf("IAMGE_OPTIONAL_HEADER32结构大小:%d\n", pFileHeader->SizeOfOptionalHeader);
    if (pFileHeader->Characteristics == 0x1000)
        printf("该文件为系统文件\n");
    if (pFileHeader->Characteristics == 0x2000)
        printf("该文件为DLL文件\n");
}

void PrintOptionAlHeader(PIMAGE_OPTIONAL_HEADER pNt)
{
    printf("\n连接器的主版本号:%d,次版本号:%d\n", pNt->MajorLinkerVersion, pNt->MinorLinkerVersion);
    printf("所有含有代码区块的总大小%ld\n", pNt->SizeOfCode);
    printf("所有初始化数据区块的总大小%ld\n", pNt->SizeOfInitializedData);
    printf("所有未初始化数据区块的总大小%ld\n", pNt->SizeOfUninitializedData);
    printf("程序执行入口RAV(AddressOfEntryPoint):%X\n", pNt->AddressOfEntryPoint);
    printf("代码区块起始RVA(BaseOfCode):%X\n", pNt->BaseOfCode);
    printf("数据区块起始RVA(BaseOfData):%X\n", pNt->BaseOfData);
    printf("数据默认装入基地址(ImageBase):%X\n", pNt->ImageBase);
    printf("操作系统主版本号%d\n", pNt->MajorOperatingSystemVersion);
    printf("操作系统次版本号%d\n", pNt->MinorOperatingSystemVersion);
    printf("所需要的的子系统主版本号%d\n", pNt->MajorSubsystemVersion);
    printf("所需要的的子系统次版本号%d\n", pNt->MinorSubsystemVersion);
    printf("映像装入内存后的总尺寸(SizeOfIamge):%d\n", pNt->SizeOfImage);
    printf("DOS头,PE头,区块表总大小%d\n", pNt->SizeOfHeaders);
    printf("映像效验和%ld\n", pNt->CheckSum);
    printf("文件子系统%d\n", pNt->Subsystem);
    printf("数据目录表的项数%d\n", pNt->NumberOfRvaAndSizes);

}

void PrintSectionInfo()
{
    PIMAGE_FILE_HEADER       pFH = NULL;
    PIMAGE_SECTION_HEADER   pSH = NULL;
    pFH = GetFileHeader(MapFile.ImageBase);
    if (!pFH)
        return;
    pSH = GetFirstSectionHeader(MapFile.ImageBase);
    printf("\nPE文件的区块信息:\n");
    for (int i = 0; i < pFH->NumberOfSections; i++)
    {
        printf("段名:%s\t", pSH->Name);
        printf("VirtualSize:%x\t", pSH->Misc);
        printf("VirtualAddress:%x\t", pSH->VirtualAddress);
        printf("SizeOfRawData:%x\t", pSH->SizeOfRawData);
        printf("SizeOfRawData:%x\t", pSH->SizeOfRawData);
        printf("文件属性(Characteristics):%X\n", pSH->Characteristics);
        pSH++;
    }
}

void printIAT()
{
    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)GetDirectoryEntryToData(MapFile.ImageBase, IMAGE_DIRECTORY_ENTRY_IMPORT);
    //PIMAGE_IMPORT_DESCRIPTOR pImport=(PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)MapFile.ImageBase+GetOptionalHeader(MapFile.ImageBase)->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (pImport->FirstThunk)
    {
        char *pszDllName = (char *)RvaToPtr(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pImport->Name);//(char *)((BYTE *)MapFile.ImageBase+pImport->Name);
        printf("输入模块:%s\n", pszDllName);
        IMAGE_THUNK_DATA * pThunk = (IMAGE_THUNK_DATA *)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pImport->OriginalFirstThunk, NULL));
        int n = 0;
        while (pThunk->u1.Function)
        {
            char * pszFunName = (char *)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pThunk->u1.AddressOfData, NULL));
            PDWORD lpAddr = (DWORD*)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pImport->FirstThunk, NULL)) + n;//((BYTE *)MapFile.ImageBase+pImport->FirstThunk)+n;
            printf("从此模块导入的函数:%-25s,", pszFunName + 2);
            printf("函数地址:%X\n", lpAddr);
            n++;
            pThunk++;
        }
        pImport++;
    }
}

void printET()
{
    IMAGE_EXPORT_DIRECTORY * pExport;
    pExport = (IMAGE_EXPORT_DIRECTORY *)GetDirectoryEntryToData(MapFile.ImageBase, IMAGE_DIRECTORY_ENTRY_EXPORT);

    printf("模块的真实名称%s\n", (char *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->Name, NULL));
    int n;
    n = pExport->NumberOfFunctions;
    printf("模块中函数个数:%d\n", n);
    DWORD *AddressF = (DWORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfFunctions, NULL);
    DWORD *AddressName = (DWORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfNames, NULL);
    WORD *AddressNameO = (WORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfNameOrdinals, NULL);
    for (int i = 0; i < n; i++)
    {
        printf("函数的名称:%-30s,地址:%-4x,顺列号:%d\n", (char *)ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, *AddressName, NULL), *AddressF, *AddressNameO);
        AddressName++;
        AddressF++;
        AddressNameO++;
    }

}

void test()
{
    //TCHAR fileName[MAX_PATH];
    //scanf("%s", fileName);
    LPTSTR fileName=L"d:\\Main.exe";
    LoadFile(fileName, &MapFile);
    if (!IsPEFile(MapFile.ImageBase))
    {
        printf("打开文件失败\n");
        return;
    }
    PrintFileHeader(GetFileHeader(MapFile.ImageBase));
    PrintOptionAlHeader(GetOptionalHeader(MapFile.ImageBase));
    PrintSectionInfo();
    printIAT();
    printET();
    getchar();
    UnLoadFile(&MapFile);
}
