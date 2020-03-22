#include "DirSize.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

qint32 dirSize(const QString &dirPath)
{
    QFileInfo info(dirPath);
    qint32 size = 0;
    if(info.isDir())
    {
        QDir dir(dirPath);
        //calculate total size of current directories' files
        QDir::Filters fileFilters = QDir::Files|QDir::System|QDir::Hidden;
        for(QString filePath : dir.entryList(fileFilters)) {
            QFileInfo fi(dir, filePath);
            size+= fi.size();
        }
        //add size of child directories recursively
        QDir::Filters dirFilters = QDir::Dirs|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
        auto entrylist = dir.entryInfoList(dirFilters);
        for(auto &entry: entrylist){
            size+= dirSize(entry.absoluteFilePath());
        }
    }else if(info.isFile()){
        size = QFile(dirPath).size();
    }

    qDebug() << __FUNCTION__ << __LINE__ << dirPath << size;
    return size;
}
