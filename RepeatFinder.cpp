#include "RepeatFinder.h"
// Qt
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include <QDirIterator>
// C++ STL
#include <algorithm>
#include <assert.h>

#define COMPARISONS_COUNT_TRACE     0
#define HASH_CACHE_TRACE            0

namespace{
#if COMPARISONS_COUNT_TRACE
static int comparisonsCount = 0;
#endif

bool lessSize(const El &el1, const El &el2)
{
#if COMPARISONS_COUNT_TRACE
    comparisonsCount++;
    qDebug() << __FUNCTION__ << comparisonsCount;
#endif
    return el1.size < el2.size;
}

bool equalSize(El const &c1, El const &c2)
{
#if COMPARISONS_COUNT_TRACE
    comparisonsCount++;
    qDebug() << __FUNCTION__ << comparisonsCount;
#endif
    return c1.size == c2.size;
};

class EqualSizeFunctor
{
    const El&el;
public:
    EqualSizeFunctor(const El&el) :el(el){};
    bool operator()(El const &c)
    {
#if COMPARISONS_COUNT_TRACE
        comparisonsCount++;
        qDebug() << __FUNCTION__ << comparisonsCount;
#endif
        return c.size == el.size;
    }
};

}

QByteArray RepeatFinder::fileHash(const QString &path)
{
    QByteArray ret;
    QFile f(path);
    auto size = f.size();
    if(size > cacheMinimum && !(ret = cache.value(path)).isNull())
    {
#if HASH_CACHE_TRACE
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!" << path;
#endif
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
#if HASH_CACHE_TRACE
            qDebug() << __FUNCTION__ << __LINE__ << "????????" << path;
#endif
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
#if HASH_CACHE_TRACE
        qDebug() << __FUNCTION__ << __LINE__ << "!!!!!!!" << el.path;
#endif
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
#if HASH_CACHE_TRACE
                qDebug() << __FUNCTION__ << __LINE__ << "????????" << el.path;
#endif
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

    dirTree.reserve(dirTree.size() + fileTree.size());
    dirTree.insert(dirTree.end(), fileTree.begin(), fileTree.end());
    return dirTree;
}

EqualsTree RepeatFinder::findFile(const QString path)
{
    return buildFileEqualsTree(path, fileVector);
}

EqualsTree RepeatFinder::diffFolder(const QString path)
{
    RepeatFinder other;
    other.makeIndexation(path);

    EqualsTree dirDiff = diff(dirVector, other.dirVector);
    EqualsTree fileDiff = diff(fileVector, other.fileVector);

    dirDiff.reserve(dirDiff.size() + fileDiff.size());
    dirDiff.insert(dirDiff.cend(), fileDiff.begin(), fileDiff.end());

    return dirDiff;
}

EqualsTree RepeatFinder::diff(std::vector<El> v1, std::vector<El> v2)
{
    EqualsTree tree;
    tree.reserve(2048);
    ElConstIterator it = v1.cbegin();
    ElConstIterator itOther = v2.cbegin();
    ElConstIterator end = v1.cend();

    while((it = std::adjacent_find(it, end, equalSize)) != end)
    {
        const El &el = *it;
        //        auto range = std::equal_range(it, end, el, lessSize);
        std::pair<ElConstIterator, ElConstIterator> range(it, it);
        //        range.second = std::upper_bound(it, end, el, lessSize);
        range.second = std::find_if_not(it + 1, end, EqualSizeFunctor(el));

        //        if(range.second != range.first)
        {
            auto rangeSize = range.second - range.first;
            //        qDebug()<<__FUNCTION__ << "rangeSize" << rangeSize << rangeOtherSize;
            auto rangeOther = std::equal_range(itOther, v2.cend(), el, lessSize);
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
                            node.copies.push_back(copy.path);
                            copyFound = true;
                        }
                    }
                }
                if(!copyFound)
                {
                    node.originalPath = original.path;
                    tree.push_back(node);
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

EqualsTree RepeatFinder::buildFileEqualsTree(const QString path, const std::vector<El> &v)
{
    EqualsTree tree;
    qint32 entrySize = QFile(path).size();
    El el{path, entrySize};
    buildEqualsTreeForElement(el, v, tree);
    return tree;
}

/// Builds sorted by size list of files in dir and subdirs.
void RepeatFinder::buildFilesList(const QString &path, std::vector<El> &fileVector, std::vector<El> &dirVector)
{
    QDir dir(path);
    add(dir, fileVector, dirVector);

    std::sort(std::begin(fileVector), std::end(fileVector), lessSize);
    std::sort(std::begin(dirVector), std::end(dirVector), lessSize);
#if COMPARISONS_COUNT_TRACE
    comparisonsCount = 0;
#endif
}

void RepeatFinder::buildEqualsTreeForElement(const El &el, const std::vector<El> &inputEntriesVector, EqualsTree &tree)
{
    auto range = std::equal_range(inputEntriesVector.cbegin(), inputEntriesVector.cend(), el, lessSize);
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
                    node.copies.push_back(copy.path);
                }
            }
        }
        if(!node.copies.empty())
        {
            node.originalPath = original.path;
            tree.push_back(node);
        }
    }
}

EqualsTree RepeatFinder::buildEqualsTree(const std::vector<El> &inputEntriesVector)
{
    EqualsTree tree;
    tree.reserve(2048);
    ElConstIterator it = inputEntriesVector.cbegin();

    auto end = inputEntriesVector.cend();

    while((it = std::adjacent_find(it, end, equalSize)) != end)
    {
        const El &el = *it;
        //auto range = std::equal_range(it, end, el, lessSize);
        std::pair<ElConstIterator, ElConstIterator> range(it, it);
        //        range.second = std::upper_bound(it, end, el, lessSize);
        range.second = std::find_if_not(it +1, end, EqualSizeFunctor(el));

        auto rangeSize = range.second - range.first;
        //        qDebug()<<__FUNCTION__ << "rangeSize" << rangeSize;
        //if(rangeSize > 1)
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
                                    node.copies.push_back(copy.path);
                                    indexed[copyIndex] = true;
                                }
                            }
                        }
                    }
                    if(!node.copies.empty())
                    {
                        node.originalPath = original.path;
                        tree.push_back(node);
                    }
                }
            }
        }
        it = range.second;
    }

    return tree;
}

qint32 RepeatFinder::addFile(const QString path, std::vector<El> &fileVector)
{
    qint32 entrySize = QFile(path).size();
    El el{path, entrySize};

    fileVector.push_back(el);
    return entrySize;
}

qint32 RepeatFinder::add(const QDir &dir, std::vector<El> &fileVector, std::vector<El> &dirVector)
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
