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
    EqualsTree findRepeats(const QString &path);
private:
    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector);
    EqualsTree findFileRepeats(const QVector<El> &fileVector, QVector<El> &dirVector);
    void add(QVector<El> &fileVector, QVector<El> &dirVector, const QDir &dir);
};

qint64 dirSize(const QString &dirPath);

#endif // REPEATFINDER_H
