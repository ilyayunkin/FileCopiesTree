#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QIcon>

class ThumbnailProvider
{
public:
    static QIcon GetThumbnail(QString path);
};

#endif // THUMBNAILPROVIDER_H
