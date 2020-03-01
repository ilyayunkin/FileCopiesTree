#include "RepeatFinder.h"
// Qt
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
// C++ STL
#include <algorithm>
#include <assert.h>

#include "DirSize.h"

static QByteArray fileHash(const QString &path)
{
    QByteArray ret;
    QFile f(path);
    if(f.open(QIODevice::ReadOnly)){
        QCryptographicHash h(QCryptographicHash::Md5);
        h.addData(&f);
        ret = h.result();
        qDebug() << __FUNCTION__ << path << ret;
    }
    return ret;
}

static QByteArray dirHash(const QString &path)
{
    QCryptographicHash h(QCryptographicHash::Md5);
    qDebug() << __FUNCTION__ << __LINE__;
    QDir::Filters dirFilters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
    auto entrylist = QDir(path).entryInfoList(dirFilters);
    for(auto &entry: entrylist){
        if(entry.isFile()){
            h.addData(fileHash(entry.absoluteFilePath()));
            qDebug() << __FUNCTION__ << __LINE__;
        }else if(entry.isDir()){
            h.addData(dirHash(entry.absoluteFilePath()));
            qDebug() << __FUNCTION__ << __LINE__;
        }
    }
    QByteArray ret = h.result();
    qDebug() << __FUNCTION__ << path << ret;
    return ret;
}

static QByteArray hash(const QString &path)
{
    QByteArray ret;
    QFileInfo entry(path);
    if(entry.isFile()){
        ret = fileHash(path);
    }else if(entry.isDir() &&
             entry.fileName() != "." &&
             entry.fileName() != ".."){
        ret = dirHash(path);
    }
    qDebug() << __FUNCTION__ << path << ret;
    return ret;
}

RepeatFinder::RepeatFinder()
{
}

EqualsTree RepeatFinder::findCopies(const QString &path)
{
    QVector<El> fileVector;
    fileVector.reserve(2048);
    QVector<El> dirVector;
    dirVector.reserve(2048);
    buildFilesList(path, fileVector, dirVector);
    EqualsTree fileTree = buildEqualsTree(fileVector);
    EqualsTree dirTree = buildEqualsTree(dirVector);

    return dirTree + fileTree;
}

/// Builds sorted by size list of files in dir and subdirs.
void RepeatFinder::buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector)
{
    QDir dir(path);
    add(dir, fileVector, dirVector);

    std::sort(std::begin(fileVector), std::end(fileVector));
    std::sort(std::begin(dirVector), std::end(dirVector));
}

EqualsTree RepeatFinder::buildEqualsTree(const QVector<El> &v)
{
    EqualsTree tree;
    tree.reserve(2048);
    ElConstIterator it = v.cbegin();

    auto end = v.cend();
    while((it != end) && ((it + 1) != end))
    {
        const auto &el = *it;
        auto range = std::equal_range(it, end, el);
        auto rangeSize = range.second - range.first;
        qDebug()<<__FUNCTION__ << "rangeSize" << rangeSize;
        if(rangeSize > 1)
        {
            bool indexed[rangeSize] = { 0 };
            QByteArray hashes[rangeSize];
            auto lastIndex = rangeSize / 2 + rangeSize % 2;

            for(int originalIndex = 0; originalIndex <= lastIndex; ++originalIndex)
            {
                if(!indexed[originalIndex])
                {
                    const auto &original = *(range.first + originalIndex);
                    EqualNode node;
                    for(int copyIndex = originalIndex + 1; copyIndex < rangeSize; ++copyIndex)
                    {
                        assert(copyIndex != originalIndex);
                        if(!indexed[copyIndex])
                        {
                            const auto &copy = *(range.first + copyIndex);
                            if((original.path != copy.path) &&
                                    (original.path.left(copy.path.length() + 1) != (copy.path + '/')) &&
                                    (copy.path.left(original.path.length() + 1) != (original.path + '/')))
                            {
                                if(hashes[originalIndex].isNull())
                                {
                                    hashes[originalIndex] = hash(original.path);
                                }
                                if(hashes[copyIndex].isNull())
                                {
                                    hashes[copyIndex] = hash(copy.path);
                                }
                                qDebug() << __FUNCTION__ << original.path << original.size<< "AND" << copy.path << copy. size;
                                if(hashes[originalIndex] == hashes[copyIndex])
                                {
                                    node.copies.append(copy.path);
                                    indexed[copyIndex] = true;
                                }
                            }
                        }
                    }
                    if(!node.copies.isEmpty())
                    {
                        node.originalPath = original.path;
                        tree.append(node);
                    }
                }
            }
        }
        it = range.second;
    }

    return tree;
}

quint64 RepeatFinder::add(const QDir &dir, QVector<El> &fileVector, QVector<El> &dirVector)
{
    qint64 size = 0;
    assert(dir.exists());

    QDir::Filters dirFilters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
    auto entrylist = dir.entryInfoList(dirFilters);
    for(auto &entry: entrylist){
        if(entry.isFile()){
            qint64 entrySize = dirSize(entry.absoluteFilePath());
            El el{entry.absoluteFilePath(), entrySize};

            size+= entrySize;
            fileVector.push_back(el);
        }else if(entry.isDir()){
            QDir d(entry.absoluteFilePath());

            if(d.absolutePath() != dir.absolutePath()){
               qint64 entrySize = add(d, fileVector, dirVector);
               size+= entrySize;
            }
        }
    }

    El el{dir.absolutePath(), size};
    dirVector.push_back(el);

    return size;
}
