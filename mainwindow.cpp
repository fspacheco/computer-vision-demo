#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QPluginLoader>
#include <QString>
#include <QInputDialog>

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
    
    undoAction = new QAction("Undo", this);
    setThresholdAction = new QAction("Set Threshold Value", this);
    editMenu->addAction(undoAction);
    editMenu->addAction(setThresholdAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));

    connect(undoAction, SIGNAL(triggered(bool)), this, SLOT(undo()));

    connect(setThresholdAction, SIGNAL(triggered(bool)), this, SLOT(setThreshold()));

    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(showAboutInfo()));

    setupShortcuts();

    // operation actions
    toGrayscaleAction = new QAction("to Grayscale", this);
    thresholdAction = new QAction("Threshold", this);
    connectedCompAction = new QAction("Connect.Comp", this);
    editToolBar->addAction(toGrayscaleAction);
    editToolBar->addAction(thresholdAction);
    editToolBar->addAction(connectedCompAction);
    connect(toGrayscaleAction, SIGNAL(triggered(bool)), this, SLOT(toGrayscaleImage()));
    connect(thresholdAction, SIGNAL(triggered(bool)), this, SLOT(thresholdImage()));
    connect(connectedCompAction, SIGNAL(triggered(bool)), this, SLOT(connectedCompImage()));
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
    if (!undoStack.isEmpty()) { //not for the first image
        undoStack.push(image);
    }
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width())
        .arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
    currentImagePath = path;
}

void MainWindow::undo()
{
    if (!undoStack.isEmpty()) {
        QPixmap pixmap = undoStack.pop();
        imageScene->clear();
        imageView->resetMatrix();
        currentImage = imageScene->addPixmap(pixmap);
        imageScene->update();
        imageView->setSceneRect(pixmap.rect());
        QString status = QString("Undo");
        mainStatusLabel->setText(status);
    }
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
    QList<QKeySequence> shortcuts;
    shortcuts << Qt::CTRL+Qt::Key_Z;
    undoAction->setShortcuts(shortcuts);
}

void MainWindow::showAboutInfo()
{
    QString infoStr = "This demo shows the processing flow for a computer vision application.\n";
    infoStr.append("The application in mind is counting objects, like food grains. You can run it step by step.\n\n");
    infoStr.append("Author: Fernando S. Pacheco - 2021\n\n");
    infoStr.append("GUI interface based on code from the book Qt 5 and OpenCV 4 Computer Vision Projects by Zhuo Qingliang");
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
    undoStack.push(pixmap);
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
    undoStack.push(pixmap);
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
    undoStack.push(pixmap);
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine());

    cv::Mat tmp;
    cv::threshold(mat, tmp, thresholdValue, 255, cv::THRESH_BINARY);
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
    QString status = QString("(image with threshold %1 applied), %2x%3")
        .arg(thresholdValue).arg(pixmap.width()).arg(pixmap.height());
    mainStatusLabel->setText(status);
}

void MainWindow::connectedCompImage()
{
    if (currentImage == nullptr) {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }
    QPixmap pixmap = currentImage->pixmap();
    undoStack.push(pixmap);
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    // matC3: RGB image to draw circles
    cv::Mat matC3 = cv::Mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());

    // imageC1, matC1: single channel image (required by connecteComponentesWithStats)
    QImage imageC1 = pixmap.toImage();
    imageC1 = image.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat matC1 = cv::Mat(imageC1.height(), imageC1.width(), CV_8UC1, imageC1.bits(), imageC1.bytesPerLine());

    cv::Mat labelImage, stats, centroids;

    int nLabels = connectedComponentsWithStats(matC1, labelImage, stats, centroids);

    cv::Point center;
    for (int i=0;i<nLabels;i++) {
        //std::cout << stats.at<int>(i,cv::CC_STAT_LEFT) << std::endl;
        center.x = (int)centroids.at<double>(i,0);
        center.y = (int)centroids.at<double>(i,1);
        cv::circle(matC3, center, 10, cv::Scalar(255, 0, 0), -1);
    }

    QImage image_labeled(matC3.data, matC3.cols, matC3.rows, matC3.step, QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(image_labeled);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
    QString status = QString("(");
    status.append(QString::number(nLabels));
    status.append(" blobs found)");
    mainStatusLabel->setText(status);
}

void MainWindow::setThreshold()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("Threshold value"),
                                 tr("0-255"), thresholdValue, 0, 255, 1, &ok);
    if (ok) {
        //integerLabel->setText(tr("%1%").arg(i));
        std::cout << "value:" << i << std::endl;
        thresholdValue = i;
    }
}
