// detours_demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "detours\detours.h"

typedef int(__cdecl *NewTargetFunc)(int a, int b, int c);
typedef int(__cdecl *OldTargetFunc)(int a, int b);

OldTargetFunc g_pfnTarget = nullptr;
OldTargetFunc g_pfnFakeTarget = nullptr;

int __cdecl TargetFunc(int a, int b, int c)
{
    int s = a + b + c;
    return s;
}

int __cdecl Fake_Target(int aa, int bb)
{
    int ss = aa - bb;
    //int orig_s = g_pfnTarget(aa, bb);
    return ss;
}

int __cdecl add_ccall(int a, int b)
{
    return a + b;
}

int __stdcall add_stdcall(int a, int b)
{
    return a + b;
}

int add_default(int a, int b)
{
    return a + b;
}

int main()
{
    HANDLE hProc = ::GetModuleHandle(nullptr);
    g_pfnTarget = (OldTargetFunc)&TargetFunc;
    g_pfnFakeTarget = (OldTargetFunc)&Fake_Target;

    DetourTransactionBegin();
    DetourUpdateThread(::GetCurrentThread());
    DetourAttach((PVOID*)&g_pfnTarget, (LPVOID)g_pfnFakeTarget);
    DetourTransactionCommit();

    int i = 0;

    i = 1;
    TargetFunc(1, 2, 3);
    i = 2;

    return 0;
}

