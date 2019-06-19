#include "ThumbnailedIconProvider.h"

#include "ThumbnailProvider.h"

QIcon ThumbnailedIconProvider::icon(const QFileInfo &info) const
{
    QIcon icon = ThumbnailProvider::GetThumbnail(info.absoluteFilePath());
    if(icon.isNull()){
        icon = QIcon(QFileIconProvider::icon(info));
    }
    return icon;
}
