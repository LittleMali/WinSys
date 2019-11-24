// detours_demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "detours\detours.h"

int TargetFunc(int a, int b);

typedef int(*PFNTargetFunc)(int a, int b);

PFNTargetFunc g_pfnTarget = nullptr;
PFNTargetFunc g_pfnFakeTarget = nullptr;

int TargetFunc(int a, int b)
{
    int s = a + b;
    return s;
}

int Fake_Target(int aa, int bb)
{
    int ss = aa - bb;
    int orig_s = g_pfnTarget(aa, bb);
    return ss;
}

int main()
{
    HANDLE hProc = ::GetModuleHandle(nullptr);
    g_pfnTarget = (PFNTargetFunc)&TargetFunc;
    g_pfnFakeTarget = (PFNTargetFunc)&Fake_Target;

    DetourTransactionBegin();
    DetourUpdateThread(::GetCurrentThread());
    DetourAttach((PVOID*)&g_pfnTarget, (LPVOID)g_pfnFakeTarget);
    DetourTransactionCommit();

    TargetFunc(1, 2);

    return 0;
}

