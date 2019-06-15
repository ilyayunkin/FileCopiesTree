#ifndef REPEATFINDER_H
#define REPEATFINDER_H

#include <QVector>
#include <QDir>

struct El
{
    QString path;
    qint64 size;

    bool operator <(const struct El &other) const
    {
        return size < other.size;
    }
};
typedef QVector<El> ElVector;
typedef QVector<El>::iterator ElIterator;

struct EqualNode
{
    QString originalPath;
    QVector<QString> copies;
};
typedef QVector<EqualNode>EqualsTree;
typedef EqualsTree::iterator EqualsIterator;

class RepeatFinder
{
public:

    RepeatFinder();

    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path);
    EqualsTree buildEqualsTree();
private:
    QVector<El>v;
    void add(const QDir &dir);
};

#endif // REPEATFINDER_H
