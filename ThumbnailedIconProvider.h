#ifndef THUMBNAILEDICONPROVIDER_H
#define THUMBNAILEDICONPROVIDER_H

#include <QIcon>
#include <QFileInfo>

class ThumbnailedIconProvider
{
public:
    QIcon icon(const QFileInfo &info, bool basicIconOnly = false) const;
};

#endif // THUMBNAILEDICONPROVIDER_H
