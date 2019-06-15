#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QVector>
#include <QDebug>
#include <QFileIconProvider>
#include <QSplitter>
#include <QHeaderView>
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
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget());
    QSplitter *splitter = new QSplitter;
    mainLayout->addWidget(splitter);
    {
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
                connect(treeWidget, &QTreeWidget::itemClicked,
                        this, &MainWindow::itemClicked);
            }
        }
        {
            QWidget *w = new QWidget;
            QVBoxLayout *imgLayout = new QVBoxLayout(w);
            imgLayout->setMargin(0);
            splitter->addWidget(w);
            imgLabel = new QLabel;
            imgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            imgLayout->addWidget(imgLabel);
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
    imgLabel->clear();
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

void  MainWindow::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = item->text(0);
    imgLabel->setPixmap(getIcon(path).pixmap(300, 300));
}
