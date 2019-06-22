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

#include <assert.h>

#include "ThumbnailProvider.h"
#include "ThumbnailedIconProvider.h"
#include "DirSize.h"

#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new QWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    {
        QSplitter *splitter = new QSplitter;
        mainLayout->addWidget(splitter);
        {
            QWidget *w = new QWidget;
            QVBoxLayout *treeLayout = new QVBoxLayout(w);
            treeLayout->setMargin(0);
            splitter->addWidget(w);
            {
                QPushButton *pb = new QPushButton(tr("Select dir"));
                connect(pb, &QPushButton::clicked, this, &MainWindow::selectDir);
                treeLayout->addWidget(pb);
            }
            {
                treeWidget = new QTreeWidget;
                treeWidget->header()->hide();
                treeLayout->addWidget(treeWidget);

                connect(treeWidget, &QTreeWidget::itemDoubleClicked,
                        this, &MainWindow::itemDoubleClicked);
                connect(treeWidget, &QTreeWidget::itemClicked,
                        this, &MainWindow::itemClicked);
            }
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
            {
                deleteButton = new QPushButton("delete");
                imgLayout->addWidget(deleteButton);
                deleteButton->hide();

                connect(deleteButton, &QPushButton::clicked,
                        this, &MainWindow::deleteFile);
            }
        }
        splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    }
    {
        statusLabel = new QLabel;
        mainLayout->addWidget(statusLabel);
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::selectDir(bool checked)
{
    Q_UNUSED(checked);
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select dir"));

    if(!dirPath.isEmpty()){
        QDateTime begin = QDateTime::currentDateTime();

            RepeatFinder finder;
            EqualsTree tree = finder.findCopies(dirPath);

            showTree(tree);
        QDateTime end = QDateTime::currentDateTime();
        auto secs = begin.secsTo(end);
        qDebug() << "It took" << secs << "sec";
        qDebug() << begin;
        qDebug() << end;
        qDebug() << "size" << tree.size();
        imgLabel->clear();
        statusLabel->setText(QString("%1 copies found in %2 seconds").arg(tree.size()).arg(secs));
    }
}

void MainWindow::showTree(const EqualsTree& tree)
{
    infoLabel->clear();
    statusLabel->clear();
    treeWidget->clear();
    selectedPath.clear();
    deleteButton->hide();
    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for(const EqualNode &node : tree){
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node.originalPath));

        item->setIcon(0, ThumbnailedIconProvider().icon(node.originalPath));
        items.append(item);
        for(const QString &copy :node.copies){
            QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, QStringList(copy));
            item1->setIcon(0, ThumbnailedIconProvider().icon(copy));
            item->addChild(item1);
        }
    }
    treeWidget->insertTopLevelItems(0, items);
}

void  MainWindow::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = item->text(0);
    selectedPath = path;
    deleteButton->show();
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

void  MainWindow::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = item->text(0);

    QUrl url = QUrl::fromLocalFile(path);
    QDesktopServices::openUrl(url);
}

void MainWindow::deleteFile(bool triggered)
{
    Q_UNUSED(triggered)
    bool confirm =
            QMessageBox::question(this,
                                  tr("Confirm deleting"),
                                  QString(tr("Do you really want to delete %1?")).arg(selectedPath)) ==
            QMessageBox::Yes;

    if(confirm){
        qDebug() << selectedPath << " deletion confirmed";
        bool removed = QDir().remove(selectedPath);

        if(removed){
            QMessageBox::information(this,
                                     tr("File is removed"),
                                     QString(tr("File %1 is removed")).arg(selectedPath));
            qDebug() << selectedPath << " removed";
        }else{
            QMessageBox::critical(this,
                                  tr("File is removed"),
                                  QString(tr("File %1 is NOT removed")).arg(selectedPath));
            qDebug() << selectedPath << " NOT removed!!!!!!!!";
        }
    }
}
