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
#include <QStack>

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

    void undo();

    void setThreshold();

    // operations
    void toGrayscaleImage();
    void thresholdImage();
    void connectedCompImage();

private:
    int thresholdValue = 128;

    QStack<QPixmap> undoStack;

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

    QAction *undoAction;

    QAction *setThresholdAction;

    QAction *toGrayscaleAction;
    QAction *thresholdAction;
    QAction *connectedCompAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;

    QMap<QString, EditorPluginInterface*> editPlugins;
};

#endif // MAINWINDOW_H
