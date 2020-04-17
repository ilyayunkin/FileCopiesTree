#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QPixmap>

class ThumbnailProvider
{
public:
    static QPixmap GetThumbnail(const QString &path,
                                const long width = 300,
                                const long heigth = 300);
};

#endif // THUMBNAILPROVIDER_H
