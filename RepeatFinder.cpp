#include "RepeatFinder.h"
// Qt
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include <QDirIterator>
// C++ STL
#include <algorithm>
#include <assert.h>

class Compare
{
public:
    bool operator()(const El &el1, const El &el2)
    {
        static int i = 0;
        i++;
        qDebug() << __FUNCTION__ << i;
        return el1 < el2;
    }
};

QByteArray RepeatFinder::fileHash(const QString &path)
{
    QByteArray ret;
    QFile f(path);
    auto size = f.size();
    if(size > cacheMinimum && !(ret = cache.value(path)).isNull())
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!" << path;
    }else
    {
        if(f.open(QIODevice::ReadOnly)){
            QCryptographicHash h(QCryptographicHash::Md5);
            h.addData(&f);
            ret = h.result();
//            qDebug() << __FUNCTION__ << path << ret;
        }

        if(size > cacheMinimum)
        {
            cache.insert(path, ret);
            qDebug() << __FUNCTION__ << __LINE__ << "????????" << path;
        }
    }

    return ret;
}

QByteArray RepeatFinder::dirHash(const QString &path)
{
    QCryptographicHash h(QCryptographicHash::Md5);
//    qDebug() << __FUNCTION__ << __LINE__;

    QDir::Filters dirFilters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
    QDirIterator dirIterator(path, dirFilters);
    while(dirIterator.hasNext())
    {
        auto entry = QFileInfo(dirIterator.next());
        if(entry.isFile()){
            h.addData(fileHash(entry.absoluteFilePath()));
//            qDebug() << __FUNCTION__ << __LINE__;
        }else if(entry.isDir()){
            h.addData(dirHash(entry.absoluteFilePath()));
//            qDebug() << __FUNCTION__ << __LINE__;
        }
    }
    QByteArray ret = h.result();
//    qDebug() << __FUNCTION__ << path << ret;
    return ret;
}

QByteArray RepeatFinder::hash(const El &el)
{
    QByteArray ret;

    if(el.size > cacheMinimum && !(ret = cache.value(el.path)).isNull())
    {
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!" << el.path;
    }else
    {
        QFileInfo entry(el.path);
        if(entry.isFile())
        {
            ret = fileHash(el.path);
        }else if(entry.isDir())
        {
            assert(entry.fileName() != "." && entry.fileName() != "..");
            ret = dirHash(el.path);
            if(el.size > cacheMinimum)
            {
                qDebug() << __FUNCTION__ << __LINE__ << "????????" << el.path;
                cache.insert(el.path, ret);
            }
        }
    }
//    qDebug() << __FUNCTION__ << el.path << ret;
    return ret;
}

RepeatFinder::RepeatFinder()
{
}

void RepeatFinder::makeIndexation(const QString &path)
{
    fileVector.reserve(2048);
    dirVector.reserve(2048);
    buildFilesList(path, fileVector, dirVector);
}

EqualsTree RepeatFinder::findCopies()
{
    EqualsTree fileTree = buildEqualsTree(fileVector);
    EqualsTree dirTree = buildEqualsTree(dirVector);

    return dirTree + fileTree;
}

EqualsTree RepeatFinder::findFile(const QString path)
{
    return buildFileEqualsTree(path, fileVector);
}

EqualsTree RepeatFinder::diffFolder(const QString path)
{
    RepeatFinder other;
    other.makeIndexation(path);
    return diff(dirVector, other.dirVector) +
            diff(fileVector, other.fileVector);
}

EqualsTree RepeatFinder::diff(QVector<El> v1, QVector<El> v2)
{
    EqualsTree tree;
    tree.reserve(2048);
    ElConstIterator it = v1.cbegin();
    ElConstIterator itOther = v2.cbegin();
    ElConstIterator end = v1.cend();

    while(it != end)
    {
        const El &el = *it;
//        auto range = std::equal_range(it, end, el/*, Compare()*/);
        std::pair<ElConstIterator, ElConstIterator> range(it, it);
        range.second = std::upper_bound(it, end, el/*, Compare()*/);

        if(range.second != range.first)
        {
            auto rangeSize = range.second - range.first;
            //        qDebug()<<__FUNCTION__ << "rangeSize" << rangeSize << rangeOtherSize;
            auto rangeOther = std::equal_range(itOther, v2.cend(), el/*, Compare()*/);
            auto rangeOtherSize = rangeOther.second - rangeOther.first;
            QByteArray hashes[rangeSize];
            QByteArray hashesOther[rangeSize];

            for(int originalIndex = 0; originalIndex < rangeSize; ++originalIndex)
            {
                bool copyFound = false;
                const El &original = *(range.first + originalIndex);
                EqualNode node;
                for(int copyIndex = 0; (copyIndex < rangeOtherSize) && ! copyFound; ++copyIndex)
                {
                    const auto &copy = *(rangeOther.first + copyIndex);
                    if((original.path != copy.path) &&
                            (original.path.left(copy.path.length() + 1) != (copy.path + '/')) &&
                            (copy.path.left(original.path.length() + 1) != (original.path + '/')))
                    {
                        if(hashes[originalIndex].isNull())
                        {
                            hashes[originalIndex] = hash(original);
                        }
                        if(hashesOther[copyIndex].isNull())
                        {
                            hashesOther[copyIndex] = hash(copy);
                        }
//                        qDebug() << __FUNCTION__ << original.path << original.size<< "AND" << copy.path << copy. size;
                        if(hashes[originalIndex] == hashesOther[copyIndex])
                        {
                            node.copies.append(copy.path);
                            copyFound = true;
                        }
                    }
                }
                if(!copyFound)
                {
                    node.originalPath = original.path;
                    tree.append(node);
                }
            }
            if(rangeOtherSize > 0)
            {
                itOther = rangeOther.second;
            }
        }
        it = range.second;
    }

    return tree;

}

EqualsTree RepeatFinder::buildFileEqualsTree(const QString path, const QVector<El> &v)
{
    EqualsTree tree;
    qint32 entrySize = QFile(path).size();
    El el{path, entrySize};
    buildEqualsTreeForElement(el, v, tree);
    return tree;
}

/// Builds sorted by size list of files in dir and subdirs.
void RepeatFinder::buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector)
{
    QDir dir(path);
    add(dir, fileVector, dirVector);

    std::sort(std::begin(fileVector), std::end(fileVector));
    std::sort(std::begin(dirVector), std::end(dirVector));
}

void RepeatFinder::buildEqualsTreeForElement(const El &el, const QVector<El> &inputEntriesVector, EqualsTree &tree)
{
    auto range = std::equal_range(inputEntriesVector.cbegin(), inputEntriesVector.cend(), el/*, Compare()*/);
    if(range.second != range.first)
    {
        auto rangeSize = range.second - range.first;
    //    qDebug()<<__FUNCTION__ << "rangeSize" << rangeSize;
        const El &original = el;
        QByteArray originalHash = hash(el);
        EqualNode node;
        for(int copyIndex = 0; copyIndex < rangeSize; ++copyIndex)
        {
            const auto &copy = *(range.first + copyIndex);
            if((original.path != copy.path) &&
                    (original.path.left(copy.path.length() + 1) != (copy.path + '/')) &&
                    (copy.path.left(original.path.length() + 1) != (original.path + '/')))
            {
                QByteArray copyHash = hash(copy);

//                qDebug() << __FUNCTION__ << original.path << original.size<< "AND" << copy.path << copy.size;
                if(originalHash == copyHash)
                {
                    node.copies.append(copy.path);
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

EqualsTree RepeatFinder::buildEqualsTree(const QVector<El> &inputEntriesVector)
{
    EqualsTree tree;
    tree.reserve(2048);
    ElConstIterator it = inputEntriesVector.cbegin();

    auto end = inputEntriesVector.cend();
    while((it != end) && ((it + 1) != end))
    {
        const El &el = *it;
        //auto range = std::equal_range(it, end, el/*, Compare()*/);
        std::pair<ElConstIterator, ElConstIterator> range(it, it);
        range.second = std::upper_bound(it, end, el/*, Compare()*/);
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
                    const El &original = *(range.first + originalIndex);
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
                                    hashes[originalIndex] = hash(original);
                                }
                                if(hashes[copyIndex].isNull())
                                {
                                    hashes[copyIndex] = hash(copy);
                                }
//                                qDebug() << __FUNCTION__ << original.path << original.size<< "AND" << copy.path << copy. size;
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

qint32 RepeatFinder::addFile(const QString path, QVector<El> &fileVector)
{
    qint32 entrySize = QFile(path).size();
    El el{path, entrySize};

    fileVector.push_back(el);
    return entrySize;
}

qint32 RepeatFinder::add(const QDir &dir, QVector<El> &fileVector, QVector<El> &dirVector)
{
    qint32 size = 0;
    assert(dir.exists());

    QDir::Filters dirFilters = QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
    QDirIterator dirIterator(dir.absolutePath(), dirFilters);
    while(dirIterator.hasNext())
    {
        auto entry = QFileInfo(dirIterator.next());
        if(entry.isFile()){
            size+= addFile(entry.absoluteFilePath(), fileVector);
        }else if(entry.isDir()){
            QDir d(entry.absoluteFilePath());

            if(d.absolutePath() != dir.absolutePath()){
                qint32 entrySize = add(d, fileVector, dirVector);
                size+= entrySize;
            }
        }
    }

    El el{dir.absolutePath(), size};
    dirVector.push_back(el);

    return size;
}
