#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <string>

#include <shobjidl.h>
#include <shlobj.h>

#include <QIcon>
#include <QImage>
#include <QPixmap>

#include <QtWinExtras/QtWin>

class ThumbnailProvider
{
public:
    static HBITMAP GetThumbnail(std::wstring File);
    static QIcon GetThumbnail(QString path)
    {
        return QIcon(QtWin::fromHBITMAP(GetThumbnail(path.replace('/', '\\').toStdWString())));
    }
};

#endif // THUMBNAILPROVIDER_H
