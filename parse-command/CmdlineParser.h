///////////////////////////////////////////////////////////////////////////////
//
//  FileName :  CmdlineParser.h
//  Version  :  1.0
//  Creator  :  robotwang
//  Date     :  2010-11-18 22:32:37
//  Comment  :  命令行参数解析
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __CMDLINEPARSER_H__
#define __CMDLINEPARSER_H__
#pragma once

#include <tchar.h>
#include <list>
#include "qsdefine.h"

//////////////////////////////////////////////////////////////////////////

struct CCmdlineParamNode
{
    tstring     strName;
    tstring     strValue;
};
typedef std::list<CCmdlineParamNode> CCmdlineParamList;

//////////////////////////////////////////////////////////////////////////

class CCmdlineParser
{
public:
    CCmdlineParser();

public:
    // 解析
    void    Parse();
    void    Parse(const wchar_t *pCmdline);
    void    Parse(int argc, TCHAR **argv);

    // 取得结果
    BOOL    GetParamValue(const TCHAR *pName, tstring &strValue);
    BOOL    GetParamValue(const TCHAR *pName, int &nValue);
    BOOL    GetParamValue(const TCHAR *pName, __int64 &nValue);
    BOOL    GetParamValue(const TCHAR *pName);

    const CCmdlineParamList &GetParamList() const;

private:
    CCmdlineParamList   m_ParamList;
};

inline void CCmdlineParser::Parse()
{
    Parse(__argc, __targv);
}

inline const CCmdlineParamList &CCmdlineParser::GetParamList() const
{
    return m_ParamList;
}

#endif // __CMDLINEPARSER_H__