#include "ThumbnailProvider.h"

// C++ STL
#include <string>
#include <memory>
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

namespace
{
template<class T>
class Releaser
{
public:
    void operator()(T *obj)
    {
        qDebug() << __FUNCTION__ << obj;
        obj->Release();
    }
};

typedef std::shared_ptr<HBITMAP__> HbitmapPtr;

///I used http://www.cplusplus.com/forum/windows/100661/
static HbitmapPtr Thumbnail(const std::wstring &File,
                            const long width = 300,
                            const long heigth = 300)
{
    int Pos = File.find_last_of(L"\\");

    std::shared_ptr<IShellFolder> desktop;
    {
        IShellFolder* pDesktop = NULL;
        const HRESULT hr = SHGetDesktopFolder(&pDesktop);
        if(FAILED(hr))
        {
            qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
            return NULL;
        }
        desktop = std::shared_ptr<IShellFolder>(pDesktop, Releaser<IShellFolder>());
    }

    LPITEMIDLIST pidl = NULL;
    {
        const std::wstring Folder = File.substr(0,Pos);
        const HRESULT hr = desktop->ParseDisplayName(NULL, NULL, (LPWSTR)Folder.c_str(), NULL, &pidl, NULL);
        qDebug() << __FUNCTION__ << __LINE__ << QString().fromWCharArray(Folder.c_str());
        if(FAILED(hr))
        {
            qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
            return NULL;
        }
    }

    std::shared_ptr<IShellFolder> sub;
    {
        IShellFolder* pSub = NULL;
        const HRESULT hr = desktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&pSub);
        if(FAILED(hr))
        {
            qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
            return NULL;
        }
        sub = std::shared_ptr<IShellFolder>(pSub, Releaser<IShellFolder>());
    }
    {
        const std::wstring FileName = File.substr(Pos+1);
        const HRESULT hr = sub->ParseDisplayName(NULL, NULL, (LPWSTR)FileName.c_str(), NULL, &pidl, NULL);
        qDebug() << __FUNCTION__ << __LINE__ << QString().fromWCharArray(FileName.c_str());
        if(FAILED(hr))
        {
            qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
            return NULL;
        }
    }

    std::shared_ptr<IExtractImage> extract;
    {
        IExtractImage* pIExtract = NULL;
        const HRESULT hr = sub ->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_IExtractImage, NULL, (void**)& pIExtract);
        if(FAILED(hr))
        {
            qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!!!!!!";
            return NULL;
        }
        extract = std::shared_ptr<IExtractImage>(pIExtract,  Releaser<IExtractImage>());
    }

    {// Set up the options for the image
        SIZE size{width, heigth};
        DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY;
        OLECHAR pathBuffer[MAX_PATH];
        const HRESULT hr = extract->GetLocation(pathBuffer, MAX_PATH, NULL, &size,32, &dwFlags);
        (void)hr;
    }

    HBITMAP hThumbnail = NULL;
    {// Get the image
        const HRESULT hr = extract ->Extract(&hThumbnail);
        (void)hr;
    }

    HbitmapPtr ptr(hThumbnail, DeleteObject);
    return ptr;
}
}

QPixmap ThumbnailProvider::GetThumbnail(const QString &path, const long width, const long heigth)
{
    HbitmapPtr hBmp = Thumbnail(QString(path).replace('/', '\\').toStdWString(), width, heigth);
    QPixmap ret = QtWin::fromHBITMAP(hBmp.get());
    return ret;
}
