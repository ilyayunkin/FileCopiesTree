#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QVector>
#include <QDebug>
#include <QFileIconProvider>

#include <assert.h>

#include "ThumbnailProvider.h"

QIcon MainWindow::getIcon(const QString& path)
{
    QIcon icon = ThumbnailProvider::GetThumbnail(path);
    if(icon.isNull()){
        QFileIconProvider provider;
        icon = QIcon(provider.icon(path));
    }
    return icon;
}

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

    if(!dirPath.isEmpty()){
        RepeatFinder finder;
        finder.buildFilesList(dirPath);

        EqualsTree tree = finder.buildEqualsTree();
        showTree(tree);
    }
}

void MainWindow::showTree(const EqualsTree& tree)
{
    treeWidget->clear();
    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for(const EqualNode &node : tree){
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node.originalPath));

        item->setIcon(0, getIcon(node.originalPath));
        items.append(item);
        for(const QString &copy :node.copies){
            QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, QStringList(copy));
            item1->setIcon(0, getIcon(copy));
            item->addChild(item1);
        }
    }
    treeWidget->insertTopLevelItems(0, items);
}
