#ifndef REPEATFINDER_H
#define REPEATFINDER_H

#include <QVector>
#include <QDir>

struct El
{
    QString path;
    qint32 size;

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
    QVector<El> fileVector;
    QVector<El> dirVector;
    QHash<QString, QByteArray> cache;
    static const long cacheMinimum = 1024 * 1024 * 2; // 2 Mb

public:
    RepeatFinder();

    void makeIndexation(const QString &path);
    EqualsTree findCopies();
    EqualsTree findFile(const QString path);
    EqualsTree diffFolder(const QString path);
private:
    qint32 add(const QDir &dir, QVector<El> &fileVector, QVector<El> &dirVector);
    qint32 addFile(const QString path, QVector<El> &fileVector);

    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path, QVector<El> &fileVector, QVector<El> &dirVector);
    /// \brief Builds A tree where each branch represents copies of the same file.
    /// \param inputEntriesVector - a sorted by size vector of files.
    /// \returns the tree.
    EqualsTree buildEqualsTree(const QVector<El> &inputEntriesVector);
    void buildEqualsTreeForElement(const El &el,
                                   const QVector<El> &inputEntriesVector,
                                   EqualsTree &tree);
    /// \brief Builds A tree where each branch represents copies of the file by "path".
    /// \param path - the path to the file.
    /// \param v - a sorted by size vector of files.
    /// \returns the tree.
    EqualsTree buildFileEqualsTree(const QString path, const QVector<El> &v);

    EqualsTree diff(QVector<El> v1, QVector<El> v2);

    QByteArray fileHash(const QString &path);
    QByteArray dirHash(const QString &path);
    QByteArray hash(const El &el);
};

#endif // REPEATFINDER_H
