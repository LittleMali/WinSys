#include "stdafx.h"
#include <windows.h>

#include "detours.h"
#include "detours_util.h"

BOOL InstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc) 
{
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(ppTargetFunc, pDetourFunc);
  return NO_ERROR == DetourTransactionCommit();
}

BOOL UnInstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc)
{
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourDetach(ppTargetFunc, pDetourFunc);
  return NO_ERROR == DetourTransactionCommit();
}