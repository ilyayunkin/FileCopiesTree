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

    while((it + 1) != v.end()){
        auto el = *it;
        auto range = std::equal_range(it + 1, std::end(v), el);
        EqualNode node;
        node.originalPath = el.path;

        if(range.first != range.second){
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
