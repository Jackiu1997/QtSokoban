#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QPoint>
#include <QImage>

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "sokoban.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum GameMode { READY, PLAYING, WIN } gameMode;

    QString mapFilePath;

    int mapWidth;
    int mapHeight;
    Sokoban sokoban;

    int blockWidth = 70;
    QPoint origin;

    QImage wallImg;
    QImage boxImg;
    QImage boxTImg;
    QImage tarImg;
    QImage manImg;

    bool readMapFromFile(MapVec &mapData);
    void loadGameMap();

    Ui::MainWindow *ui;
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void drawBlock(QPainter &painter, int row, int col, int type);


private slots:
    void receUpdate();
    void on_actionmap1_triggered();
    void on_actionmap2_triggered();
    void on_actionai_triggered();
    void on_actionload_triggered();
    void on_actionexit_triggered();
    void on_actionreset_triggered();
};

#endif // MAINWINDOW_H
