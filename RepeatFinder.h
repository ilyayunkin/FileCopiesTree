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
typedef QVector<El>::const_iterator ElConstIterator;

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

    EqualsTree findCopies(const QString &path);
private:
    quint64 add(const QDir &dir, QVector<El> &fileVector, QVector<El> &dirVector);
    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector);
    EqualsTree buildEqualsTree(const QVector<El> &v);
};

#endif // REPEATFINDER_H
