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
    auto entrylist = QDir(path).entryInfoList();
    for(auto &entry: entrylist){
        if(entry.isFile()){
            h.addData(fileHash(entry.absoluteFilePath()));
            qDebug() << __FUNCTION__ << __LINE__;
        }else if(entry.isDir() &&
                 entry.fileName() != "." &&
                 entry.fileName() != ".."){
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
        {
            bool indexed[rangeSize] = { 0 };
            auto lastIndex = rangeSize / 2 + rangeSize % 2;

            for(int originalIndex = 0; originalIndex <= lastIndex; ++originalIndex)
            {
                if(!indexed[originalIndex])
                {
                    const auto &original = *(range.first + originalIndex);
                    EqualNode node;
                    for(int copyIndex = originalIndex; copyIndex < rangeSize; ++copyIndex)
                    {
                        if(!indexed[copyIndex])
                        {
                            const auto &copy = *(range.first + copyIndex);
                            if((original.path != copy.path) &&
                                    (original.path.left(copy.path.length() + 1) != (copy.path + '/')) &&
                                    (copy.path.left(original.path.length() + 1) != (original.path + '/')) &&
                                    (hash(original.path) == hash(copy.path)))
                            {
                                node.copies.append(copy.path);
                                indexed[copyIndex] = true;
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

void RepeatFinder::add(const QDir &dir, QVector<El> &fileVector, QVector<El> &dirVector)
{
    assert(dir.exists());

    auto entrylist = dir.entryInfoList();
    for(auto &entry: entrylist){
        if(entry.isFile()){
            El el{entry.absoluteFilePath(), dirSize(entry.absoluteFilePath())};
            fileVector.push_back(el);
        }else if(entry.isDir() &&
                 entry.fileName() != "." &&
                 entry.fileName() != ".."){
            QDir d(entry.absoluteFilePath());
            El el{entry.absoluteFilePath(), dirSize(entry.absoluteFilePath())};
            dirVector.push_back(el);
            if(d.absolutePath() != dir.absolutePath()){
                add(d, fileVector, dirVector);
            }
        }
    }
}
