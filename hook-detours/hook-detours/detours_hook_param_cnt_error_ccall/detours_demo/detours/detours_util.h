#pragma once

/*
* @brief ��װhook
* @param pTargetFunc  -- hookĿ�꺯����ַ�ĺ���ָ��
* @param pDetourFunc  -- �滻������ַ
*/
BOOL InstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc);

/*
* @brief ��װhook
* @param pTargetFunc  -- hookĿ�꺯����ַ�ĺ���ָ��
* @param pDetourFunc  -- �滻������ַ
*/
BOOL UnInstallHook_Detours(PVOID* ppTargetFunc, PVOID pDetourFunc);
