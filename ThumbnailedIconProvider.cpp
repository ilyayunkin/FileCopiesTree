#include "ThumbnailedIconProvider.h"

#include <QFileIconProvider>

#include "ThumbnailProvider.h"

QIcon ThumbnailedIconProvider::icon(const QFileInfo &info, bool basicIconOnly) const
{
    QIcon icon;
    if(basicIconOnly ||
            ((icon = QIcon(ThumbnailProvider::GetThumbnail(info.absoluteFilePath()))).isNull())){
        icon = QIcon(QFileIconProvider().icon(info));
    }
    return icon;
}
