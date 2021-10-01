#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QPluginLoader>
#include <QString>

#include "opencv2/opencv.hpp"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , fileMenu(nullptr)
    , viewMenu(nullptr)
    , currentImage(nullptr)
{
    initUI();
    loadPlugins();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");
    editMenu = menuBar()->addMenu("&Edit");
    helpMenu = menuBar()->addMenu("&Help");

    // setup toolbar
    fileToolBar = addToolBar("File");
    editToolBar = addToolBar("Edit");

    // main area for image display
    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    setCentralWidget(imageView);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("No image loaded");

    createActions();
}

void MainWindow::createActions()
{
    // create actions, add them to menus
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    saveAsAction = new QAction("&Save as", this);
    fileMenu->addAction(saveAsAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    aboutAction = new QAction("About", this);
    helpMenu->addAction(aboutAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));

    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(showAboutInfo()));

    setupShortcuts();

    // operation actions
    toGrayscaleAction = new QAction("to Grayscale", this);
    thresholdAction = new QAction("Threshold", this);
    editToolBar->addAction(toGrayscaleAction);
    editToolBar->addAction(thresholdAction);
    connect(toGrayscaleAction, SIGNAL(triggered(bool)), this, SLOT(toGrayscaleImage()));
    connect(thresholdAction, SIGNAL(triggered(bool)), this, SLOT(thresholdImage()));
}

void MainWindow::openImage()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList filePaths;
    if (dialog.exec()) {
        filePaths = dialog.selectedFiles();
        showImage(filePaths.at(0));
    }
}

void MainWindow::showImage(QString path)
{
    imageScene->clear();
    imageView->resetMatrix();
    QPixmap image(path);
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width())
        .arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
    currentImagePath = path;
}

void MainWindow::saveAs()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "Noting to save.");
        return;
    }
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image As ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        if(QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0))) {
            currentImage->pixmap().save(fileNames.at(0));
        } else {
            QMessageBox::information(this, "Information", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::setupShortcuts()
{
}

void MainWindow::showAboutInfo()
{
    QString infoStr = "This demo shows the processing flow for a computer vision application.\n";
    infoStr.append("The application in mind is counting objects, like food grains. You can run it step by step.\n\n");
    infoStr.append("Author: Fernando S. Pacheco - 2021\n\n");
    infoStr.append("Based on examples from the book Qt 5 and OpenCV 4 Computer Vision Projects by Zhuo Qingliang");
    QMessageBox::about(this, "About", infoStr);    
}

void MainWindow::loadPlugins()
{
    QDir pluginsDir(QApplication::instance()->applicationDirPath() + "/plugins");
    QStringList nameFilters;
    nameFilters << "*.so" << "*.dylib" << "*.dll";
    QFileInfoList plugins = pluginsDir.entryInfoList(
        nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
    foreach(QFileInfo plugin, plugins) {
        QPluginLoader pluginLoader(plugin.absoluteFilePath(), this);
        EditorPluginInterface *plugin_ptr = dynamic_cast<EditorPluginInterface*>(pluginLoader.instance());
        if(plugin_ptr) {
            QAction *action = new QAction(plugin_ptr->name());
            editMenu->addAction(action);
            editToolBar->addAction(action);
            editPlugins[plugin_ptr->name()] = plugin_ptr;
            connect(action, SIGNAL(triggered(bool)), this, SLOT(pluginPerform()));
            // pluginLoader.unload();
        } else {
            qDebug() << "bad plugin: " << plugin.absoluteFilePath();
        }
    }
}

void MainWindow::pluginPerform()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }

    QAction *active_action = qobject_cast<QAction*>(sender());
    EditorPluginInterface *plugin_ptr = editPlugins[active_action->text()];
    if(!plugin_ptr) {
        QMessageBox::information(this, "Information", "No plugin is found.");
        return;
    }

    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine());

    plugin_ptr->edit(mat, mat);

    QImage image_edited(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(image_edited);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
    QString status = QString("(editted image), %1x%2")
        .arg(pixmap.width()).arg(pixmap.height());
    mainStatusLabel->setText(status);
}

void MainWindow::toGrayscaleImage()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_Grayscale8);
    pixmap = QPixmap::fromImage(image);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
    QString status = QString("(grayscale image), %1x%2")
        .arg(pixmap.width()).arg(pixmap.height());
    mainStatusLabel->setText(status);
}

void MainWindow::thresholdImage()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine());

    cv::Mat tmp;
    cv::threshold(mat, tmp, 128, 255, cv::THRESH_BINARY);
    mat = tmp;

    QImage image_thresholded(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(image_thresholded);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
    QString status = QString("(image with threshold applied), %1x%2")
        .arg(pixmap.width()).arg(pixmap.height());
    mainStatusLabel->setText(status);
}