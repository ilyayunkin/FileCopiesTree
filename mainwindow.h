#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

#include "RepeatFinder.h"

class FolderAnalysisWidget : public QWidget
{
    Q_OBJECT
    QTreeWidget *treeWidget = nullptr;
    QLabel *statusLabel = nullptr;
    QPushButton *deleteButton;

    void selectDir(bool checked);
    void showTree(const EqualsTree&tree);
    void selectionChanged();
    void itemClicked(QTreeWidgetItem *item, int column);
    void itemSelected(QTreeWidgetItem *item);
    void itemDoubleClicked(QTreeWidgetItem *item, int column);
    void deleteButtonClicked(bool triggered);
    void deleteSelected();
    void deleteItem(QTreeWidgetItem *item);
    void keyPressEvent(QKeyEvent *event);

public:
    FolderAnalysisWidget(QWidget *parent = nullptr);

signals:
    void selectedPath(QString path);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QLabel *imgLabel = nullptr;
    QLabel *infoLabel = nullptr;
    FolderAnalysisWidget *folderAnalysisWidget;
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void clearView();
    void showThumbnail(QString path);
};

#endif // MAINWINDOW_H
