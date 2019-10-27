#pragma once

typedef struct _MAP_FILE_STRUCT
{
    HANDLE hFile;
    HANDLE hMapping;
    LPVOID ImageBase;
} MAP_FILE_STRUCT, *PMAP_FILE_STRUCT;

extern MAP_FILE_STRUCT MapFile;

extern BOOL LoadFile(LPTSTR lpFilename, PMAP_FILE_STRUCT pstMapFile);
extern void UnLoadFile(PMAP_FILE_STRUCT pstMapFile);
extern BOOL IsPEFile(LPVOID ImageBase);
extern PIMAGE_NT_HEADERS GetNtHeaders(LPVOID ImageBase);
extern PIMAGE_FILE_HEADER   GetFileHeader(LPVOID ImageBase);
extern PIMAGE_OPTIONAL_HEADER GetOptionalHeader(LPVOID ImageBase);
extern PIMAGE_SECTION_HEADER GetFirstSectionHeader(LPVOID ImageBase);
extern LPVOID RvaToPtr(PIMAGE_NT_HEADERS pNtH, LPVOID ImageBase, DWORD dwRVA);
extern LPVOID GetDirectoryEntryToData(LPVOID ImageBase, USHORT DirectoryEntry);
extern void PrintFileHeader(PIMAGE_FILE_HEADER pFileHeader);
extern void PrintOptionAlHeader(PIMAGE_OPTIONAL_HEADER pNt);
extern void PrintSectionInfo();
extern void printIAT();
extern void printET();
extern void test();
