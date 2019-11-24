#pragma once

/*
* @brief 安装hook
* @param pTargetFunc  -- hook目标函数地址的函数指针
* @param pDetourFunc  -- 替换函数地址
*/
BOOL InstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc);

/*
* @brief 罐装hook
* @param pTargetFunc  -- hook目标函数地址的函数指针
* @param pDetourFunc  -- 替换函数地址
*/
BOOL UnInstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc);
