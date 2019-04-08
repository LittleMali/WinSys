#ifndef __TSTRING_QS_H__
#define __TSTRING_QS_H__
#pragma once

#include <string>

using std::string;
using std::wstring;

#ifdef _UNICODE
    #ifndef tstring
        #define tstring            std::wstring
    #endif
#else
    #ifndef tstring
        #define tstring            std::string
    #endif
#endif 


#endif // __TSTRING_QS_H__