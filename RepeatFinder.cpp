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
        qDebug() << __FUNCTION__ << path << ret;
    }
    return ret;
}

RepeatFinder::RepeatFinder()
{
}

/// Builds sorted by size list of files in dir and subdirs.
void RepeatFinder::buildFilesList(const QString &path)
{
    v.clear();
    v.reserve(2048);
    QDir dir(path);
    add(dir);

    std::sort(std::begin(v), std::end(v));
}

EqualsTree RepeatFinder::buildEqualsTree()
{
    EqualsTree tree;
    tree.reserve(2048);
    ElIterator it = v.begin();

    auto end = v.end();
    while((it != end) && ((it + 1) != end)){
        const auto &el = *it;
        auto range = std::equal_range(it, end, el);
        {
            ElVector copiesVector;
            std::copy(range.first, range.second, std::back_inserter(copiesVector));
            auto originalIt = copiesVector.begin();
            while(originalIt != copiesVector.end()){
                const auto &original = *originalIt;
                {
                    EqualNode node;
                    node.originalPath = original.path;
                    {
                        auto copyIt = copiesVector.begin();
                        while(copyIt != copiesVector.end()){
                            auto copy = *copyIt;
                            if((original.path != copy.path) &&
                                    (hash(original.path) == hash(copy.path))){
                                node.copies.append(copy.path);
                                copyIt = copiesVector.erase(copyIt);
                            }else{
                                ++copyIt;
                            }
                        }
                    }
                    if(!node.copies.isEmpty()){
                        tree.append(node);
                    }
                }
                ++originalIt;
            }
        }
        it = range.second;
    }

    return tree;
}

void RepeatFinder::add(const QDir &dir)
{
    assert(dir.exists());

    auto entrylist = dir.entryInfoList();
    for(auto &entry: entrylist){
        if(entry.isFile()){
            El el{entry.absoluteFilePath(), entry.size()};
            v.push_back(el);
        }else if(entry.isDir() &&
                 entry.fileName() != "." &&
                 entry.fileName() != ".."){
            QDir d(entry.absoluteFilePath());
            if(d.absolutePath() != dir.absolutePath()){
                add(d);
            }
        }
    }
}
