// FileCopy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string>

#define BUF_SIZE    (64 * 1024)

#define CK_READ     100
#define CK_WRITE    101

// CIoReq扩展OVERLAPPED，因为每个op用于一次读写操作，op是与读写buf相关联的，所以，可以在这里实现读写
class CIoReq : public OVERLAPPED
{
public:
    CIoReq()
    {
        ZeroMemory(this, sizeof(CIoReq));
    }

    ~CIoReq()
    {

    }

    BOOL AllocBuffer(size_t nBufSize)
    {
        m_nBufSize = nBufSize;
        m_pvData = VirtualAlloc(NULL, m_nBufSize, MEM_COMMIT, PAGE_READWRITE);
        return (m_pvData != NULL);
    }

    BOOL Read(HANDLE hDevice, PLARGE_INTEGER pliOffset = NULL)
    {
        if (pliOffset)
        {
            Offset = pliOffset->LowPart;
            OffsetHigh = pliOffset->HighPart;
        }

        BOOL bRet = ::ReadFile(hDevice, m_pvData, m_nBufSize, NULL, this);
        DWORD dwLastErr = ::GetLastError();
        if (bRet || dwLastErr == ERROR_IO_PENDING)
        {
            // 同步操作成功，或者添加异步io成功
            bRet = TRUE;
        }
        else
        {
            // see last error.
        }
        return bRet;
    }

    BOOL Write(HANDLE hDevice, PLARGE_INTEGER pliOffset = NULL)
    {
        // 如不指定offset，则从上次读的地方开始写
        if (pliOffset)
        {
            Offset = pliOffset->LowPart;
            OffsetHigh = pliOffset->HighPart;
        }

        BOOL bRet = ::WriteFile(hDevice, m_pvData, m_nBufSize, NULL, this);
        DWORD dwLastErr = ::GetLastError();
        if (bRet || dwLastErr == ERROR_IO_PENDING)
        {
            // 同步操作成功，或者添加异步io成功
            bRet = TRUE;
        }
        else
        {
            // see last error.
        }
        return bRet;
    }

private:
    size_t  m_nBufSize;
    PVOID   m_pvData;
};

BOOL FileCopy(LPCTSTR pszFileSrc, LPCTSTR pszFileDst)
{
    HANDLE hFileSrc = INVALID_HANDLE_VALUE, hFileDst = INVALID_HANDLE_VALUE;
    LARGE_INTEGER liFileSizeSrc = { 0 }, liFileSizeDst = { 0 };

    BOOL bRet = FALSE;
    do 
    {
        hFileSrc = CreateFile(pszFileSrc, GENERIC_READ, FILE_SHARE_READ, NULL
            , OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);

        if (hFileSrc == INVALID_HANDLE_VALUE)
            break;
        
        GetFileSizeEx(hFileSrc, &liFileSizeSrc);

        bRet = TRUE;
    } while (FALSE);

    if (!bRet)
    {
        if (INVALID_HANDLE_VALUE != hFileSrc)
        {
            CloseHandle(hFileSrc);
            hFileSrc = NULL;
        }

        return FALSE;
    }

    bRet = FALSE;
    do 
    {
        hFileDst = CreateFile(pszFileDst, GENERIC_WRITE, FILE_SHARE_WRITE, NULL
            , CREATE_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);

        if (hFileDst == INVALID_HANDLE_VALUE)
            break;

        // NO_BUFFERING的文件按buf对齐
        liFileSizeDst.QuadPart = (liFileSizeSrc.QuadPart / BUF_SIZE * BUF_SIZE) + (liFileSizeSrc.QuadPart % BUF_SIZE ? BUF_SIZE : 0);
        SetFilePointerEx(hFileDst, liFileSizeDst, NULL, FILE_BEGIN);
        SetEndOfFile(hFileDst);

        bRet = TRUE;
    } while (FALSE);

    if (!bRet)
    {
        if (INVALID_HANDLE_VALUE != hFileDst)
        {
            CloseHandle(hFileDst);
            hFileDst = NULL;
        }

        return FALSE;
    }

    bRet = FALSE;
    HANDLE hIocp = NULL;
    do 
    {
        // create iocp
        hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (hIocp == NULL)
            break;

        // associate iocp
        if (CreateIoCompletionPort(hFileSrc, hIocp, CK_READ, 0) != hIocp)
            break;

        if (CreateIoCompletionPort(hFileDst, hIocp, CK_WRITE, 0) != hIocp)
            break;

        bRet = TRUE;
    } while (FALSE);

    if (!bRet)
    {
        if (hIocp != NULL)
        {
            CloseHandle(hIocp);
            hIocp = NULL;
        }

        return FALSE;
    }

    // get proccessors
    SYSTEM_INFO si = { 0 };
    GetSystemInfo(&si);
    int iProcessors = si.dwNumberOfProcessors * 2 + 2;
    
    CIoReq* pArrIoReq = new CIoReq [iProcessors];
    if (!pArrIoReq)
        return FALSE;

    LARGE_INTEGER liNextReadOffset = { 0 };
    int nReadProgress = 0, nWriteProgress = 0;

    for (int i = 0; i < iProcessors; ++i)
    {
        if (pArrIoReq[i].AllocBuffer(BUF_SIZE))
        {
            // 模拟写入完成，触发读操作
            PostQueuedCompletionStatus(hIocp, 0, CK_WRITE, &pArrIoReq[i]);
            nWriteProgress++;
        }
        else
        {
            // 失败了没关系，这个op不使用罢了
        }
    }

    BOOL bFinished = TRUE;
    bRet = FALSE;

    while (nWriteProgress > 0 || nReadProgress > 0)
    {
        ULONG_PTR opKey = 0;
        DWORD dwNumBytes = 0, dwLastErr = 0;
        CIoReq* pIoReq = NULL;

        bRet = GetQueuedCompletionStatus(hIocp, &dwNumBytes, &opKey, (OVERLAPPED**)&pIoReq, INFINITE);
        dwLastErr = ::GetLastError();
        if (!bRet || pIoReq == NULL)
        {
            // error occurs, quit file copy.
            bFinished = FALSE;
            break;
        }

        switch (opKey)
        {
        case CK_WRITE:
        {
            nWriteProgress--;
            if (liNextReadOffset.QuadPart < liFileSizeSrc.QuadPart)
            {
                if (pIoReq->Read(hFileSrc, &liNextReadOffset))
                {
                    nReadProgress++;
                    liNextReadOffset.QuadPart += BUF_SIZE;
                }
                else
                {
                    bFinished = FALSE;
                }
            }
        }
        break;

        case CK_READ:
        {
            nReadProgress--;
            if (pIoReq->Write(hFileDst))
            {
                nWriteProgress++;
            }
            else
            {
                bFinished = FALSE;
            }
        }
        break;

        default:
            break;
        }

        if (!bFinished)
            break;
    }

    if (pArrIoReq)
    {
        delete[] pArrIoReq;
        pArrIoReq = nullptr;
    }

    if (hFileSrc != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFileSrc);
        hFileSrc = NULL;
    }

    if (hFileDst != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFileDst);
        hFileDst = NULL;
    }

    if (!bFinished)
    {
        return FALSE;
    }

    // copy file finished
    HANDLE hFileDstTmp = CreateFile(pszFileDst, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFileDstTmp != INVALID_HANDLE_VALUE)
    {
        SetFilePointerEx(hFileDstTmp, liFileSizeSrc, NULL, FILE_BEGIN);
        SetEndOfFile(hFileDstTmp);

        CloseHandle(hFileDstTmp);
        hFileDstTmp = NULL;
    }

    return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
    LPCTSTR lpSrc = L"C:\\ToDo\\src.zip";
    LPCTSTR lpDst = L"C:\\ToDo\\dst.zip";

    BOOL bret = FileCopy(lpSrc, lpDst);

	return 0;
}

