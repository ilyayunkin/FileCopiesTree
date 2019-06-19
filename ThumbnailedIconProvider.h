#ifndef THUMBNAILEDICONPROVIDER_H
#define THUMBNAILEDICONPROVIDER_H

#include <QFileIconProvider>

class ThumbnailedIconProvider : public QFileIconProvider
{
public:
    QIcon icon(const QFileInfo &info) const override;
};

#endif // THUMBNAILEDICONPROVIDER_H
