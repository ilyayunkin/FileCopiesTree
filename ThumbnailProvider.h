#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QPixmap>

class ThumbnailProvider
{
public:
    static QPixmap GetThumbnail(QString path, long width = 300, long heigth = 300);
};

#endif // THUMBNAILPROVIDER_H
