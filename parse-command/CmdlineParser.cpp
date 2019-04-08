#include "stdafx.h"
#include "CmdlineParser.h"

CCmdlineParser::CCmdlineParser()
{
}

void CCmdlineParser::Parse(int argc, TCHAR **argv)
{
    m_ParamList.clear();

    int i;
    for(i = 1; i < argc; i++)
    {
        const TCHAR *pParam = argv[i];
        if(pParam != NULL && (pParam[0] == _T('/') || pParam[0] == _T('-')))
        {
            CCmdlineParamNode Node;
            const TCHAR *pEqual = _tcschr(pParam + 1, _T('='));
            if(pEqual != NULL)
            {
                Node.strName = tstring(pParam + 1, pEqual - pParam - 1);
                Node.strValue = pEqual + 1;
            }
            else
            {
                Node.strName = pParam + 1;
            }
            m_ParamList.push_back(Node);
        }
    }
}

void CCmdlineParser::Parse(const wchar_t *pCmdline)
{
    int nArgc = 0;
    wchar_t **pArgv = ::CommandLineToArgvW(pCmdline, &nArgc);
    Parse(nArgc, pArgv);
    ::LocalFree(pArgv);   
}

BOOL CCmdlineParser::GetParamValue(const TCHAR *pName, tstring &strValue)
{
    CCmdlineParamList::const_iterator Iter;
    for(Iter = m_ParamList.begin(); Iter != m_ParamList.end(); Iter++)
    {
        const CCmdlineParamNode &Node = *Iter;
        if(_tcsicmp(Node.strName.c_str(), pName) == 0)
        {
            strValue = Node.strValue;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CCmdlineParser::GetParamValue(const TCHAR *pName, int &nValue)
{
    tstring strValue;
    if(!GetParamValue(pName, strValue))
        return FALSE;

    nValue = _ttoi(strValue.c_str());
    return TRUE;
}

BOOL CCmdlineParser::GetParamValue(const TCHAR *pName, __int64 &nValue)
{
    tstring strValue;
    if(!GetParamValue(pName, strValue))
        return FALSE;

    nValue = _ttoi64(strValue.c_str());
    return TRUE;
}

BOOL CCmdlineParser::GetParamValue(const TCHAR *pName)
{
    tstring strValue;
    return GetParamValue(pName, strValue);
}