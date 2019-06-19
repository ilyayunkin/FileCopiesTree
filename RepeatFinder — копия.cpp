#include "RepeatFinder.h"
// Qt
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
// C++ STL
#include <algorithm>
#include <assert.h>

static QByteArray hash(const QString &path)
{
    QByteArray ret;
    QFile f(path);
    if(f.open(QIODevice::ReadOnly)){
        QCryptographicHash h(QCryptographicHash::Md5);
        h.addData(&f);
        ret = h.result();
        qDebug() << path << ret;
    }
    return ret;
}

RepeatFinder::RepeatFinder()
{
}

EqualsTree RepeatFinder::findRepeats(const QString &path)
{
    QVector<El> fileVector;
    QVector<El> dirVector;
    fileVector.reserve(2048);
    dirVector.reserve(2048);

    buildFilesList(path, fileVector, dirVector);
    EqualsTree fileCopiesTree = findFileRepeats(fileVector, dirVector);

    fileVector.clear();

    return fileCopiesTree;
}

/// Builds sorted by size list of files in dir and subdirs.
void RepeatFinder::buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector)
{

    QDir dir(path);
    add(fileVector, dirVector, dir);

    std::sort(std::begin(fileVector), std::end(fileVector));
    std::sort(std::begin(dirVector), std::end(dirVector));
}

EqualsTree RepeatFinder::findFileRepeats(const QVector<El> &fileVector, QVector<El> &dirVector)
{
    EqualsTree tree;
    tree.reserve(2048);
    {
        ElConstIterator it = fileVector.cbegin();
        auto end = fileVector.cend();

        while(it != end && (it + 1) != fileVector.cend()){
            auto el = *it;
            auto range = std::equal_range(it + 1, fileVector.cend(), el);
            EqualNode node;
            node.originalPath = el.path;

            if((range.first != end) && ((range.second - range.first) > 1)){
                auto findEquals = [&node, el](const El copy)
                {
                    if((el.path != copy.path) &&
                            (hash(el.path) == hash(copy.path))){
                        node.copies.append(copy.path);
                    }
                };
                std::for_each(range.first, range.second, findEquals);

                it = range.second;
            } else {
                ++it;
            }

            if(!node.copies.isEmpty()){
                tree.append(node);
            }
        }
    }
    {
        ElConstIterator it = dirVector.cbegin();
        auto end = dirVector.cend();

        while(it != end && (it + 1) != dirVector.cend()){
            auto el = *it;
            auto range = std::equal_range(it + 1, end, el);
            EqualNode node;
            node.originalPath = el.path;

            if((range.first != end) && ((range.second - range.first) > 1)){
                auto findEquals = [&node, el](const El copy)
                {
                    if((el.path != copy.path) /*&&
                            (hash(el.path) == hash(copy.path))*/){
                        node.copies.append(copy.path);
                    }
                };
                std::for_each(range.first, range.second, findEquals);

                it = range.second;
            } else {
                ++it;
            }

            if(!node.copies.isEmpty()){
                tree.append(node);
            }
        }
    }

    return tree;
}

qint64 dirSize(const QString &dirPath)
{
    qint64 size = 0;
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

    qDebug() << __FUNCTION__ << __LINE__ << dirPath << size;
    return size;
}

void RepeatFinder::add(QVector<El> &fileVector, QVector<El> &dirVector, const QDir &dir)
{
    assert(dir.exists());

    auto entrylist = dir.entryInfoList();
    for(auto &entry: entrylist){
        if(entry.isFile()){
            qDebug() << __FUNCTION__ << __LINE__ << entry.absoluteFilePath();
            El el{entry.absoluteFilePath(), entry.size()};
            fileVector.push_back(el);
        }else if(entry.isDir() &&
                 entry.fileName() != "." &&
                 entry.fileName() != ".."){
            QDir d(entry.absoluteFilePath());
            if(d.absolutePath() != dir.absolutePath()){
                qDebug() << __FUNCTION__ << __LINE__ << entry.absoluteFilePath();
                El el{entry.absoluteFilePath(), dirSize(entry.absolutePath())};
                dirVector.push_back(el);
                add(fileVector, dirVector, d);
            }
        }
    }
}
