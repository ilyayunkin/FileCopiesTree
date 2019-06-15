#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

#include "RepeatFinder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTreeWidget *treeWidget = nullptr;
    QLabel *imgLabel = nullptr;
    QLabel *infoLabel = nullptr;
    QPushButton *deleteButton = nullptr;
    QString selectedPath;

    void selectDir(bool checked);
    void showTree(const EqualsTree&tree);
    QIcon getIcon(const QString& path);

    void  itemClicked(QTreeWidgetItem *item, int column);
    void itemDoubleClicked(QTreeWidgetItem *item, int column);

    void deleteFile(bool triggered);

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
