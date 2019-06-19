#include "ThumbnailProvider.h"

// C++ STL
#include <string>
// Qt
#include <QPixmap>
#include <QDebug>
// Qt Win extras
#include <QtWinExtras/QtWin>
// WINAPI
#include <qt_windows.h>
#include <setupapi.h>
#include <shobjidl.h>
#include <shlobj.h>

///I used http://www.cplusplus.com/forum/windows/100661/
static HBITMAP Thumbnail(std::wstring File, long width = 300, long heigth = 300)
{
    std::wstring Folder,FileName;
    int Pos = File.find_last_of(L"\\");
    Folder = File.substr(0,Pos);
    FileName = File.substr(Pos+1);

    IShellFolder* pDesktop = NULL;
    IShellFolder* pSub = NULL;
    IExtractImage* pIExtract = NULL;
    LPITEMIDLIST pidl = NULL;

    HRESULT hr;
    hr = SHGetDesktopFolder(&pDesktop);
    if(FAILED(hr))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
        return NULL;
    }
    hr = pDesktop->ParseDisplayName(NULL, NULL, (LPWSTR)Folder.c_str(), NULL, &pidl, NULL);
    qDebug() << QString().fromWCharArray(Folder.c_str());
    if(FAILED(hr))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
        return NULL;
    }
    hr = pDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&pSub);
    if(FAILED(hr))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
        return NULL;
    }
    hr = pSub->ParseDisplayName(NULL, NULL, (LPWSTR)FileName.c_str(), NULL, &pidl, NULL);
    qDebug() << QString().fromWCharArray(FileName.c_str());
    if(FAILED(hr))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
        return NULL;
    }
    hr = pSub ->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_IExtractImage, NULL, (void**)& pIExtract);
    if(FAILED(hr))
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
        return NULL;
    }

    SIZE size{width, heigth};

    DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY;

    HBITMAP hThumbnail = NULL;

    // Set up the options for the image
    OLECHAR pathBuffer[MAX_PATH];
    hr = pIExtract->GetLocation(pathBuffer, MAX_PATH, NULL, &size,32, &dwFlags);

    // Get the image
    hr = pIExtract ->Extract(&hThumbnail);

    pDesktop->Release();
    pSub->Release();
    pIExtract->Release();

    return hThumbnail;
}

QPixmap ThumbnailProvider::GetThumbnail(QString path, long width, long heigth)
{
    return QtWin::fromHBITMAP(Thumbnail(path.replace('/', '\\').toStdWString(), width, heigth));
}
