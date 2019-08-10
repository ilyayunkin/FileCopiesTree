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
    QLabel *statusLabel = nullptr;
    QPushButton *deleteButton = nullptr;

    void selectDir(bool checked);
    void showTree(const EqualsTree&tree);
    QIcon getIcon(const QString& path);

    void selectionChanged();
    void itemClicked(QTreeWidgetItem *item, int column);
    void itemSelected(QTreeWidgetItem *item);
    void itemDoubleClicked(QTreeWidgetItem *item, int column);

    void deleteButtonClicked(bool triggered);
    void keyPressEvent(QKeyEvent *event);
    void deleteSelected();
    void deleteItem(QTreeWidgetItem *item);

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
