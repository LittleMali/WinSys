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
        printf("��ִ���ļ���Ŀ��CPUΪ Intel i386\n");
        break;
    case 0X162:
        printf("��ִ���ļ���Ŀ��CPUΪ MIPS R3000\n");
        break;
    case 0x166:
        printf("��ִ���ļ���Ŀ��CPUΪ MIPS R4000\n");
        break;
    case 0x184:
        printf("��ִ���ļ���Ŀ��CPUΪ Alpha AXP\n");
        break;
    case 0x1F0:
        printf("��ִ���ļ���Ŀ��CPUΪ Power PC\n");
        break;
    default:printf("��ִ���ļ���Ŀ��CPUδ֪\n");

    }
    printf("�ļ���������Ŀ:%d\n", pFileHeader->NumberOfSections);
    //createTime = gmtime((time_t *)&pFileHeader->TimeDateStamp);
    //printf("�ļ��Ĵ���ʱ��GMT is: %s\n", asctime(createTime));
    printf("IAMGE_OPTIONAL_HEADER32�ṹ��С:%d\n", pFileHeader->SizeOfOptionalHeader);
    if (pFileHeader->Characteristics == 0x1000)
        printf("���ļ�Ϊϵͳ�ļ�\n");
    if (pFileHeader->Characteristics == 0x2000)
        printf("���ļ�ΪDLL�ļ�\n");
}

void PrintOptionAlHeader(PIMAGE_OPTIONAL_HEADER pNt)
{
    printf("\n�����������汾��:%d,�ΰ汾��:%d\n", pNt->MajorLinkerVersion, pNt->MinorLinkerVersion);
    printf("���к��д���������ܴ�С%ld\n", pNt->SizeOfCode);
    printf("���г�ʼ������������ܴ�С%ld\n", pNt->SizeOfInitializedData);
    printf("����δ��ʼ������������ܴ�С%ld\n", pNt->SizeOfUninitializedData);
    printf("����ִ�����RAV(AddressOfEntryPoint):%X\n", pNt->AddressOfEntryPoint);
    printf("����������ʼRVA(BaseOfCode):%X\n", pNt->BaseOfCode);
    printf("����������ʼRVA(BaseOfData):%X\n", pNt->BaseOfData);
    printf("����Ĭ��װ�����ַ(ImageBase):%X\n", pNt->ImageBase);
    printf("����ϵͳ���汾��%d\n", pNt->MajorOperatingSystemVersion);
    printf("����ϵͳ�ΰ汾��%d\n", pNt->MinorOperatingSystemVersion);
    printf("����Ҫ�ĵ���ϵͳ���汾��%d\n", pNt->MajorSubsystemVersion);
    printf("����Ҫ�ĵ���ϵͳ�ΰ汾��%d\n", pNt->MinorSubsystemVersion);
    printf("ӳ��װ���ڴ����ܳߴ�(SizeOfIamge):%d\n", pNt->SizeOfImage);
    printf("DOSͷ,PEͷ,������ܴ�С%d\n", pNt->SizeOfHeaders);
    printf("ӳ��Ч���%ld\n", pNt->CheckSum);
    printf("�ļ���ϵͳ%d\n", pNt->Subsystem);
    printf("����Ŀ¼�������%d\n", pNt->NumberOfRvaAndSizes);

}

void PrintSectionInfo()
{
    PIMAGE_FILE_HEADER       pFH = NULL;
    PIMAGE_SECTION_HEADER   pSH = NULL;
    pFH = GetFileHeader(MapFile.ImageBase);
    if (!pFH)
        return;
    pSH = GetFirstSectionHeader(MapFile.ImageBase);
    printf("\nPE�ļ���������Ϣ:\n");
    for (int i = 0; i < pFH->NumberOfSections; i++)
    {
        printf("����:%s\t", pSH->Name);
        printf("VirtualSize:%x\t", pSH->Misc);
        printf("VirtualAddress:%x\t", pSH->VirtualAddress);
        printf("SizeOfRawData:%x\t", pSH->SizeOfRawData);
        printf("SizeOfRawData:%x\t", pSH->SizeOfRawData);
        printf("�ļ�����(Characteristics):%X\n", pSH->Characteristics);
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
        printf("����ģ��:%s\n", pszDllName);
        IMAGE_THUNK_DATA * pThunk = (IMAGE_THUNK_DATA *)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pImport->OriginalFirstThunk, NULL));
        int n = 0;
        while (pThunk->u1.Function)
        {
            char * pszFunName = (char *)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pThunk->u1.AddressOfData, NULL));
            PDWORD lpAddr = (DWORD*)(::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pImport->FirstThunk, NULL)) + n;//((BYTE *)MapFile.ImageBase+pImport->FirstThunk)+n;
            printf("�Ӵ�ģ�鵼��ĺ���:%-25s,", pszFunName + 2);
            printf("������ַ:%X\n", lpAddr);
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

    printf("ģ�����ʵ����%s\n", (char *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->Name, NULL));
    int n;
    n = pExport->NumberOfFunctions;
    printf("ģ���к�������:%d\n", n);
    DWORD *AddressF = (DWORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfFunctions, NULL);
    DWORD *AddressName = (DWORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfNames, NULL);
    WORD *AddressNameO = (WORD *)::ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, pExport->AddressOfNameOrdinals, NULL);
    for (int i = 0; i < n; i++)
    {
        printf("����������:%-30s,��ַ:%-4x,˳�к�:%d\n", (char *)ImageRvaToVa(GetNtHeaders(MapFile.ImageBase), MapFile.ImageBase, *AddressName, NULL), *AddressF, *AddressNameO);
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
        printf("���ļ�ʧ��\n");
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
