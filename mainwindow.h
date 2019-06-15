#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QTextEdit>
#include <QTreeWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTreeWidget *treeWidget;

    void selectDir(bool checked);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
