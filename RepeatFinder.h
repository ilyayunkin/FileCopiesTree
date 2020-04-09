#ifndef REPEATFINDER_H
#define REPEATFINDER_H

#include <vector>
#include <QDir>

struct El
{
    QString path;
    qint32 size;
};
typedef std::vector<El> ElVector;
typedef std::vector<El>::iterator ElIterator;
typedef std::vector<El>::const_iterator ElConstIterator;

struct EqualNode
{
    QString originalPath;
    std::vector<QString> copies;
};
typedef std::vector<EqualNode>EqualsTree;
typedef EqualsTree::iterator EqualsIterator;

class RepeatFinder
{
    std::vector<El> fileVector;
    std::vector<El> dirVector;
    QHash<QString, QByteArray> cache;
    static constexpr long cacheMinimum = 1024 * 1024 * 2; // 2 Mb

public:
    RepeatFinder();

    void makeIndexation(const QString &path);
    EqualsTree findCopies();
    EqualsTree findFile(const QString path);
    EqualsTree diffFolder(const QString path);
private:
    qint32 add(const QDir &dir, std::vector<El> &fileVector, std::vector<El> &dirVector);
    qint32 addFile(const QString path, std::vector<El> &fileVector);

    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path, std::vector<El> &fileVector, std::vector<El> &dirVector);
    /// \brief Builds A tree where each branch represents copies of the same file.
    /// \param inputEntriesVector - a sorted by size vector of files.
    /// \returns the tree.
    EqualsTree buildEqualsTree(const std::vector<El> &inputEntriesVector);
    void buildEqualsTreeForElement(const El &el,
                                   const std::vector<El> &inputEntriesVector,
                                   EqualsTree &tree);
    /// \brief Builds A tree where each branch represents copies of the file by "path".
    /// \param path - the path to the file.
    /// \param v - a sorted by size vector of files.
    /// \returns the tree.
    EqualsTree buildFileEqualsTree(const QString path, const std::vector<El> &v);

    EqualsTree diff(std::vector<El> v1, std::vector<El> v2);

    QByteArray fileHash(const QString &path);
    QByteArray dirHash(const QString &path);
    QByteArray hash(const El &el);
};

#endif // REPEATFINDER_H
