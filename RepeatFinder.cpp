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
        auto el = *it;
        auto range = std::equal_range(it + 1, end, el);
        EqualNode node;
        node.originalPath = el.path;

        auto copyIt = range.first;
        while(copyIt != range.second){
            auto copy = *copyIt;
            if((el.path != copy.path) &&
                    (hash(el.path) == hash(copy.path))){
                node.copies.append(copy.path);
            }
            ++copyIt;
        }
        it = range.second;

        if(!node.copies.isEmpty()){
            tree.append(node);
        }
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
