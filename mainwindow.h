#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QMap>

#include "cvdemo_plugin_interface.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private:
    void initUI();
    void createActions();
    void showImage(QString);
    void setupShortcuts();
    // plugin
    void loadPlugins();

private slots:
    void openImage();
    void saveAs();
    void showAboutInfo();
    void pluginPerform();
    // operations
    void toGrayscaleImage();
    void thresholdImage();

private:
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *viewToolBar;
    QToolBar *editToolBar;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    QAction *openAction;
    QAction *saveAsAction;
    QAction *exitAction;

    QAction *aboutAction;

    QAction *toGrayscaleAction;
    QAction *thresholdAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    QMap<QString, EditorPluginInterface*> editPlugins;
};

#endif // MAINWINDOW_H
