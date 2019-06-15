#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QIcon>
#include <QVector>
#include <QDebug>

#include <assert.h>

#include "ThumbnailProvider.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new QWidget);
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
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
        RepeatFinder finder;
        finder.buildFilesList(dirPath);
        //        for(auto el : v){

        EqualsTree tree = finder.buildEqualsTree();
        QList<QTreeWidgetItem *> items;
        for(const EqualNode &node : tree){
            QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node.originalPath));

            item->setIcon(0, ThumbnailProvider::GetThumbnail(node.originalPath));
            items.append(item);
            for(const QString &copy :node.copies){
                QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, QStringList(copy));
                item1->setIcon(0, ThumbnailProvider::GetThumbnail(copy));
                item->addChild(item1);
            }
        }
        treeWidget->insertTopLevelItems(0, items);
    }
}
