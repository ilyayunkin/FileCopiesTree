#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector>
#include <QDebug>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QSplitter>
#include <QHeaderView>
#include <QTextCodec>
#include <QKeyEvent>

#include <assert.h>

#include "ThumbnailProvider.h"
#include "ThumbnailedIconProvider.h"
#include "DirSize.h"

#include <QDateTime>

FolderAnalysisWidget::FolderAnalysisWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *treeLayout = new QVBoxLayout(this);
    treeLayout->setMargin(0);
    {
        QPushButton *indexButton = new QPushButton(tr("Index dir"));
        connect(indexButton, &QPushButton::clicked, this, &FolderAnalysisWidget::indexDir);
        treeLayout->addWidget(indexButton);
    }
    {
        findCopiesButton = new QPushButton(tr("Find copies"));
        connect(findCopiesButton, &QPushButton::clicked, this, &FolderAnalysisWidget::findCopies);
        treeLayout->addWidget(findCopiesButton);
        findCopiesButton->setEnabled(false);
    }
    {
        findFileButton = new QPushButton(tr("Find file"));
        connect(findFileButton, &QPushButton::clicked, this, &FolderAnalysisWidget::findSpecifiedFile);
        treeLayout->addWidget(findFileButton);
        findFileButton->setEnabled(false);
    }
    {
        diffButton = new QPushButton(tr("Diff to backup"));
        diffButton->setToolTip(tr("Finds files that aren't in the backup"));
        connect(diffButton, &QPushButton::clicked, this, &FolderAnalysisWidget::diffFolder);
        treeLayout->addWidget(diffButton);
        diffButton->setEnabled(false);
    }
    {
        treeWidget = new QTreeWidget;
        treeWidget->header()->hide();
        treeLayout->addWidget(treeWidget);

        connect(treeWidget, &QTreeWidget::itemActivated,
                this, &FolderAnalysisWidget::itemDoubleClicked);
        connect(treeWidget, &QTreeWidget::itemClicked,
                this, &FolderAnalysisWidget::itemClicked);
        connect(treeWidget, &QTreeWidget::itemSelectionChanged,
                this, &FolderAnalysisWidget::selectionChanged);
    }
    {
        deleteButton = new QPushButton("delete");
        treeLayout->addWidget(deleteButton);
        deleteButton->setEnabled(false);

        connect(deleteButton, &QPushButton::clicked,
                this, &FolderAnalysisWidget::deleteButtonClicked);
    }
    {
        statusLabel = new QLabel;
        treeLayout->addWidget(statusLabel);
    }
}

void FolderAnalysisWidget::indexDir(bool checked)
{
    Q_UNUSED(checked);
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select dir"));

    if(!dirPath.isEmpty()){
        QDateTime begin = QDateTime::currentDateTime();

        repeatFinder = QSharedPointer<RepeatFinder>(new RepeatFinder());
        repeatFinder->makeIndexation(dirPath);
        findCopiesButton->setEnabled(true);
        findFileButton->setEnabled(true);
        diffButton->setEnabled(true);

        QDateTime end = QDateTime::currentDateTime();
        auto secs = begin.secsTo(end);
        qDebug() << "It took" << secs << "sec";
        qDebug() << begin;
        qDebug() << end;
        statusLabel->setText(QString("Indexed in %1 seconds").arg(secs));
    }
}

void FolderAnalysisWidget::findCopies(bool checked)
{
    Q_UNUSED(checked);

    assert(!repeatFinder.isNull());
    QDateTime begin = QDateTime::currentDateTime();
    QDateTime showBegin;
    int size = 0;
    {
        EqualsTree tree = repeatFinder->findCopies();
        size = tree.size();
        showBegin = QDateTime::currentDateTime();
        showTree(tree);
    }
    QDateTime end = QDateTime::currentDateTime();
    auto secs = begin.secsTo(end);
    auto secsShow = showBegin.secsTo(end);
    qDebug() << "It took" << secs << "sec" << QString("(%1 secons fow GUI)").arg(secsShow);
    qDebug() << begin;
    qDebug() << showBegin;
    qDebug() << end;
    qDebug() << "size" << size;
    statusLabel->setText(QString("%1 copies found in %2 seconds (%3 secons fow GUI)").arg(size).arg(secs).arg(secsShow));
}

void FolderAnalysisWidget::findSpecifiedFile(bool checked)
{
    Q_UNUSED(checked);

    assert(!repeatFinder.isNull());

    QString dirPath = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if(!dirPath.isEmpty()){
        QDateTime begin = QDateTime::currentDateTime();
        QDateTime showBegin;
        int size = 0;
        {
            EqualsTree tree = repeatFinder->findFile(dirPath);
            size = tree.size();
            showBegin = QDateTime::currentDateTime();
            showTree(tree);
        }
        QDateTime end = QDateTime::currentDateTime();
        auto secs = begin.secsTo(end);
        auto secsShow = showBegin.secsTo(end);
        qDebug() << "It took" << secs << "sec" << QString("(%1 secons fow GUI)").arg(secsShow);
        qDebug() << begin;
        qDebug() << showBegin;
        qDebug() << end;
        qDebug() << "size" << size;
        statusLabel->setText(QString("%1 copies found in %2 seconds (%3 secons fow GUI)").arg(size).arg(secs).arg(secsShow));
    }
}

void FolderAnalysisWidget::diffFolder(bool checked)
{
    Q_UNUSED(checked);

    assert(!repeatFinder.isNull());

    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select a dir"));

    if(!dirPath.isEmpty()){
        QDateTime begin = QDateTime::currentDateTime();
        QDateTime showBegin;
        int size = 0;
        {
            EqualsTree tree = repeatFinder->diffFolder(dirPath);
            size = tree.size();
            showBegin = QDateTime::currentDateTime();
            showTree(tree);
        }
        QDateTime end = QDateTime::currentDateTime();
        auto secs = begin.secsTo(end);
        auto secsShow = showBegin.secsTo(end);
        qDebug() << "It took" << secs << "sec" << QString("(%1 secons fow GUI)").arg(secsShow);
        qDebug() << begin;
        qDebug() << showBegin;
        qDebug() << end;
        qDebug() << "size" << size;
        statusLabel->setText(QString("%1 copies found in %2 seconds (%3 secons fow GUI)").arg(size).arg(secs).arg(secsShow));
    }
}

void FolderAnalysisWidget::showTree(const EqualsTree& tree)
{
    statusLabel->clear();
    treeWidget->clear();
    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for(const EqualNode &node : tree){
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node.originalPath));

        item->setIcon(0, ThumbnailedIconProvider().icon(node.originalPath, true));
        items.append(item);
        for(const QString &copy :node.copies){
            QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, QStringList(copy));
            item1->setIcon(0, ThumbnailedIconProvider().icon(copy, true));
            item->addChild(item1);
        }
    }
    treeWidget->insertTopLevelItems(0, items);
}

void  FolderAnalysisWidget::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    itemSelected(item);
}

void  FolderAnalysisWidget::itemSelected(QTreeWidgetItem *item)
{
    QString path = item->text(0);
    deleteButton->show();
    emit selectedPath(path);
    deleteButton->setEnabled(true);
}

void FolderAnalysisWidget::selectionChanged()
{
    auto selected = treeWidget->selectedItems();
    if(!selected.isEmpty())
    {
        QTreeWidgetItem *item = selected.takeFirst();
        itemSelected(item);
    }
}

void  FolderAnalysisWidget::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = item->text(0);

    QUrl url = QUrl::fromLocalFile(path);
    QDesktopServices::openUrl(url);
}

void FolderAnalysisWidget::deleteSelected()
{
    auto selected = treeWidget->selectedItems();
    if(!selected.isEmpty())
    {
        QTreeWidgetItem *item = selected.takeFirst();
        deleteItem(item);
    }
}

void FolderAnalysisWidget::deleteItem(QTreeWidgetItem *item)
{
    const QString path = item->text(0);
    bool confirm =
            QMessageBox::question(this,
                                  tr("Confirm deleting"),
                                  QString(tr("Do you really want to delete %1?")).arg(path)) ==
            QMessageBox::Yes;

    if(confirm){
        qDebug() << path << " deletion confirmed";
        bool removed;
        if(QFileInfo(path).isDir()){
            removed = QDir(path).removeRecursively();
        }else{
            removed = QDir().remove(path);
        }

        if(removed){
            QMessageBox::information(this,
                                     tr("File is removed"),
                                     QString(tr("File %1 is removed")).arg(path));
            qDebug() << path << " removed";
            QTreeWidgetItem *parent = item->parent();
            {/// Deletion of an item from widget
                delete item;
            }
            if(parent != nullptr)
            {/// Deletion parent item if it hasn't childs (copies of the file)
                if(parent->childCount() == 0)
                {
                    delete parent;
                }
            }
        }else{
            QMessageBox::critical(this,
                                  tr("File isn't removed"),
                                  QString(tr("File %1 is NOT removed")).arg(path));
            qDebug() << path << " NOT removed!!!!!!!!";

            bool confirmPermissions =
                    QMessageBox::question(this, "Change permissions?", "Do you want to permit deleting?") ==
                    QMessageBox::Yes;
            if(confirmPermissions)
            {
                QFile file(path);
                bool permissionsChanged = file.setPermissions(QFile::ReadOther | QFile::WriteOther);
                if(permissionsChanged)
                {
                    QMessageBox::information(this, "Permissions changed", "Try to delete the file again.");
                    qDebug() << path << " permitted";
                }else{
                    QMessageBox::critical(this,
                                          tr("Permessions aren't changed"),
                                          QString(tr("Permessions for file %1 are NOT changed")).arg(path));
                    qDebug() << path << "Permessions for file are NOT changed";
                }
            }
        }
    }
}

void FolderAnalysisWidget::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    if(key == Qt::Key_Delete){
        deleteSelected();
    }
    event->accept();
}

void FolderAnalysisWidget::deleteButtonClicked(bool triggered)
{
    Q_UNUSED(triggered)
    deleteSelected();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new QWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    {
        QSplitter *splitter = new QSplitter;
        mainLayout->addWidget(splitter);
        {
            folderAnalysisWidget = new FolderAnalysisWidget;
            connect(folderAnalysisWidget, FolderAnalysisWidget::selectedPath,
                    this, MainWindow::showThumbnail);
            splitter->addWidget(folderAnalysisWidget);
        }
        {
            QWidget *w = new QWidget;
            QVBoxLayout *imgLayout = new QVBoxLayout(w);
            imgLayout->setMargin(0);
            splitter->addWidget(w);
            {
                imgLabel = new QLabel;
                imgLayout->addWidget(imgLabel);
            }
            {
                infoLabel = new QLabel;
                infoLabel->setWordWrap(true);
                imgLayout->addWidget(infoLabel);
            }
        }
        splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::clearView()
{
    infoLabel->clear();
    imgLabel->clear();
}

void MainWindow::showThumbnail(QString path)
{
    imgLabel->setPixmap(ThumbnailedIconProvider().icon(path).pixmap(300, 300));
    QString sizeString;
    {
        auto size = dirSize(path);
        auto gb = size / 1024 / 1024 / 1024;
        size = size % (1024 * 1024 * 1024);
        auto mb = size / 1024 / 1024;
        size = size % (1024 * 1024);
        auto kb = size / 1024;
        size = size % (1024);
        auto b = size;

        if(gb) sizeString += QString("%1 Gbytes ").arg(gb);
        if(mb) sizeString += QString("%1 Mbytes ").arg(mb);
        if(kb) sizeString += QString("%1 kbytes ").arg(kb);
        if(b) sizeString += QString("%1 bytes ").arg(b);
    }

    QString info = QString(tr("Path:%1\r\n")).arg(path)+
            QString("Size:%1").arg(sizeString);
    infoLabel->setText(info);
}
