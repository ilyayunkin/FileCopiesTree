#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QVector>
#include <QDebug>
#include <QCryptographicHash>

#include <algorithm>
#include <assert.h>

#include "ThumbnailProvider.h"

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

QByteArray hash(const QString &path)
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

class Finder
{
public:

    Finder()
    {
    }

    /// Builds sorted by size list of files in dir and subdirs.
    void buildFilesList(const QString &path)
    {
        v.clear();
        v.reserve(2048);
        QDir dir(path);
        add(dir);

        std::sort(std::begin(v), std::end(v));
    }

    EqualsTree buildEqualsTree()
    {
        EqualsTree tree;
        tree.reserve(2048);
        ElIterator it = v.begin();

        while(it != v.end()){
            auto el = *it;
            auto range = std::equal_range(it, std::end(v), el);
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

    const QVector<El> &getSortedFileList() const
    {
        return v;
    }
private:
    QVector<El>v;
    void add(const QDir &dir)
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
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new QWidget);
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
        //        {
        //            te = new QTextEdit;
        //            mainLayout->addWidget(te);
        //        }
        {
            QPushButton *pb = new QPushButton(tr("Select dir"));
            connect(pb, &QPushButton::clicked, this, &MainWindow::selectDir);
            mainLayout->addWidget(pb);
        }
        {
            treeWidget = new QTreeWidget;
            mainLayout->addWidget(treeWidget);
        }
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::selectDir(bool checked)
{
    Q_UNUSED(checked);
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select dir"));

    treeWidget->setColumnCount(1);
    if(!dirPath.isEmpty()){
        Finder finder;
        finder.buildFilesList(dirPath);
        //        for(auto el : v){

        EqualsTree tree = finder.buildEqualsTree();
        QList<QTreeWidgetItem *> items;
        for(const EqualNode &node : tree){
            //            te->append(node.originalPath);
            //            for(const QString &copy :node.copies){
            //                te->append(QString("    ") + copy);
            //            }

            {
                QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node.originalPath));

                item->setIcon(0, ThumbnailProvider::GetThumbnail(node.originalPath));
                items.append(item);
                for(const QString &copy :node.copies){
                    QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, QStringList(copy));
                    item1->setIcon(0, ThumbnailProvider::GetThumbnail(copy));
                    item->addChild(item1);
                }
            }
        }
        treeWidget->insertTopLevelItems(0, items);
    }
}
