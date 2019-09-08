#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Sokuban");
    this->setWindowIcon(QIcon(":/img/res/box.png"));

    origin = QPoint(0, 20);
    gameMode = READY;

    wallImg = QImage(":/img/res/wall.png");
    boxImg = QImage(":/img/res/box.png");
    boxTImg = QImage(":/img/res/box_t.png");
    manImg = QImage(":/img/res/man.png");
    tarImg = QImage(":/img/res/target.png");

    connect(&sokoban, &Sokoban::sendUpdate, this, &MainWindow::receUpdate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::readMapFromFile(MapVec &mapData) {
    QFile file(mapFilePath);
    if(!file.open(QIODevice::ReadOnly)) return false;
    QTextStream *out = new QTextStream(&file);
    QStringList tempOption = out->readAll().split("\r\n");

    QStringList dataList = tempOption.at(0).split(",");
    mapHeight = dataList.at(0).toInt();
    mapWidth = dataList.at(1).toInt();
    for (int i = 1; i < tempOption.count(); i++)
    {
        vector<int> lineMap;
        QStringList dataList = tempOption.at(i).split(",");
        for (int j = 0; j < mapWidth; j++) {
            lineMap.push_back(dataList.at(j).toInt());
        }
        mapData.push_back(lineMap);
    }
    file.close();
    return true;
}

void MainWindow::loadGameMap() {
    MapVec mapData;
    if (readMapFromFile(mapData)) {
        sokoban.loadMap(mapData, mapHeight, mapWidth);
        this->setMaximumSize(mapWidth*blockWidth, mapHeight*blockWidth);
        this->setMinimumSize(mapWidth*blockWidth, mapHeight*blockWidth);
        gameMode = PLAYING;
        update();
    } else {
        QMessageBox::information(this, "", "Read Map Failed!");
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // 地面背景绘制
    painter.setBrush(QBrush(QColor("#ffffff")));
    painter.drawRect(QRect(origin.rx(), origin.ry(),mapWidth*blockWidth, mapHeight*blockWidth));

    if (gameMode == PLAYING || gameMode == WIN) {
        // 绘制地图元素
        for (int row = 0; row < mapHeight; row++) {
            for (int col = 0; col < mapWidth; col++) {
                int type = sokoban.gameMap[row][col];
                drawBlock(painter, row, col, type);
            }
        }
    }
}

void MainWindow::drawBlock(QPainter &painter, int row, int col, int type) {
    QRect rect(origin.rx() + col*blockWidth, origin.ry() + row*blockWidth, blockWidth, blockWidth);
    switch (type) {
    case -1: painter.drawImage(rect, wallImg);break; // 墙壁
    case 0: break; // 地面
    case 1: painter.drawImage(rect, boxImg); break; // 箱子
    case 2: painter.drawImage(rect, manImg); break; // 人
    case 3: painter.drawImage(rect, tarImg); break; // 目标
    case 4: painter.drawImage(rect, manImg); break;
    case 5: painter.drawImage(rect, boxTImg); break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Up: sokoban.actionByMan(1); break;
    case Qt::Key_Down: sokoban.actionByMan(2); break;
    case Qt::Key_Left: sokoban.actionByMan(3); break;
    case Qt::Key_Right: sokoban.actionByMan(4); break;
    default: break;
    }

    update();

    if (sokoban.win) {
        QMessageBox::information(this, "", "You Win, press Reset to continue!");
        gameMode = WIN;
    }
}

void MainWindow::on_actionmap1_triggered() {
    mapFilePath = ":/map/maps/map1.txt";
    loadGameMap();
}

void MainWindow::on_actionmap2_triggered() {
    mapFilePath = ":/map/maps/map2.txt";
    loadGameMap();
}

void MainWindow::on_actionai_triggered() {
    sokoban.actionByAI();

    if (sokoban.win) {
        QMessageBox::information(this, "", "You Win, press Reset to continue!");
        gameMode = WIN;
    }
}

void MainWindow::receUpdate() {
    repaint();
}

void MainWindow::on_actionload_triggered() {
    mapFilePath = QFileDialog::getOpenFileName(this,"请选择地图文件",".","*.txt");
    loadGameMap();
}

void MainWindow::on_actionreset_triggered() {
    loadGameMap();
}

void MainWindow::on_actionexit_triggered() {
    this->close();
}

