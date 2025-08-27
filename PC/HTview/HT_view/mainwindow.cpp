#include "mainwindow.h"
#include "qcustomplot.h"
#include "csvparser.h"
#include "csvworker.h"
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include <QDateTime>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <algorithm>

// НОВЫЕ INCLUDES для печати и копирования
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QClipboard>
#include <QApplication>
#include <QBuffer>
#include <QImageWriter>
#include <QPageLayout>      // ДОБАВИТЬ для Qt 6
#include <QPageSize>        // ДОБАВИТЬ для Qt 6 
#include <QRegularExpression> // ДОБАВИТЬ для Qt 6

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), workerThread(nullptr), csvWorker(nullptr), progressDialog(nullptr)
{
    // Инициализация настроек (в AppData)
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath); // создать папку если не существует
    settings = new QSettings(configPath + "/HT_view.ini", QSettings::IniFormat, this);
    
    // Инициализация массива действий для recent files
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);
    }
    
    setupUI();
    setupMenus();
    loadSettings();
    updateRecentFilesMenu();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
}

void MainWindow::loadSettings()
{
    // Загрузить размер и позицию окна
    restoreGeometry(settings->value("geometry").toByteArray());
    restoreState(settings->value("windowState").toByteArray());
    
    // ЗАГРУЗИТЬ состояние черно-белой печати
    bool blackWhitePrint = settings->value("blackWhitePrint", false).toBool();
    if (blackWhitePrintAction) {
        blackWhitePrintAction->setChecked(blackWhitePrint);
    }
}

void MainWindow::saveSettings()
{
    // Сохранить размер и позицию окна
    settings->setValue("geometry", saveGeometry());
    settings->setValue("windowState", saveState());
    
    // СОХРАНИТЬ состояние черно-белой печати
    if (blackWhitePrintAction) {
        settings->setValue("blackWhitePrint", blackWhitePrintAction->isChecked());
    }
}

QString MainWindow::getLastDirectory()
{
    return settings->value("lastDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
}

void MainWindow::setLastDirectory(const QString& dirPath)
{
    settings->setValue("lastDirectory", dirPath);
}

void MainWindow::addToRecentFiles(const QString& filePath)
{
    QStringList recentFiles = settings->value("recentFiles").toStringList();
    
    // Удалить файл из списка если он уже есть
    recentFiles.removeAll(filePath);
    
    // Добавить в начало списка
    recentFiles.prepend(filePath);
    
    // Ограничить размер списка
    while (recentFiles.size() > MaxRecentFiles) {
        recentFiles.removeLast();
    }
    
    // Удалить несуществующие файлы
    QStringList validFiles;
    for (const QString& file : recentFiles) {
        if (QFile::exists(file)) {
            validFiles.append(file);
        }
    }
    
    settings->setValue("recentFiles", validFiles);
}

void MainWindow::updateRecentFilesMenu()
{
    QStringList recentFiles = settings->value("recentFiles").toStringList();
    
    int numRecentFiles = qMin(recentFiles.size(), MaxRecentFiles);
    
    for (int i = 0; i < numRecentFiles; ++i) {
        QString strippedName = QFileInfo(recentFiles[i]).fileName();
        QString text = QString("&%1 %2").arg(i + 1).arg(strippedName);
        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(recentFiles[i]);
        recentFileActions[i]->setVisible(true);
    }
    
    for (int i = numRecentFiles; i < MaxRecentFiles; ++i) {
        recentFileActions[i]->setVisible(false);
    }
    
    recentFilesSeparator->setVisible(numRecentFiles > 0);
}

QString MainWindow::shortenPath(const QString& fullPath, int maxLength)
{
    if (fullPath.length() <= maxLength) {
        return fullPath;
    }
    
    QFileInfo fileInfo(fullPath);
    QString fileName = fileInfo.fileName();
    QString dirPath = fileInfo.absolutePath();
    
    // Если даже имя файла слишком длинное
    if (fileName.length() > maxLength - 10) {
        return "..." + fileName.right(maxLength - 3);
    }
    
    // Сократить путь к каталогу
    QString shortened = "..." + dirPath.right(maxLength - fileName.length() - 4) + "/" + fileName;
    return shortened;
}

void MainWindow::updateWindowTitle(const QString& filePath)
{
    QString title = "HT View - Temperature/Humidity Plotter";
    
    if (!filePath.isEmpty()) {
        // Имя файла ПОСЛЕ названия приложения
        title = QString("%1 - %2").arg(title).arg(QFileInfo(filePath).fileName());
        setWindowTitle(title);
        
        // Показать полный путь в статус баре ПОСТОЯННО
        statusBar()->showMessage(filePath, 0); // 0 = показывать постоянно
    } else {
        setWindowTitle(title);
        statusBar()->clearMessage();
    }
}

void MainWindow::setupMenus() {
    // Меню File
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *openAction = fileMenu->addAction("&Open CSV...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    // Recent Files submenu
    recentFilesMenu = fileMenu->addMenu("Recent &Files");
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        recentFilesMenu->addAction(recentFileActions[i]);
    }
    recentFilesSeparator = recentFilesMenu->addSeparator();
    
    updateRecentFilesMenu();
    
    fileMenu->addSeparator();
    
    // ЧЕКБОКС для черно-белой печати
    blackWhitePrintAction = fileMenu->addAction("Print in &Black && White");
    blackWhitePrintAction->setCheckable(true);
    blackWhitePrintAction->setChecked(false);  // По умолчанию цветная печать
    
    fileMenu->addSeparator();
    
    // УПРОЩЕННЫЕ ПУНКТЫ ПЕЧАТИ - без диалогов выбора цвета
    QAction *printPreviewAction = fileMenu->addAction("Print Pre&view...");
    printPreviewAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(printPreviewAction, &QAction::triggered, this, &MainWindow::printPreview);
    
    QAction *printAction = fileMenu->addAction("&Print...");
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::printChart);
    
    fileMenu->addSeparator();
    
    QAction *exportAction = fileMenu->addAction("&Export Chart...");
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportChart);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Меню Edit
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    
    QAction *copyAction = editMenu->addAction("&Copy Chart");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyChartToClipboard);
    
    // Меню Help
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About HT View", 
            "Temperature & Humidity Data Viewer\n\n"
            "Navigation:\n"
            "• D/W/M - Day/Week/Month scale\n"
            "• Arrow keys - navigate timeline\n"
            "• Mouse wheel - zoom time axis\n"
            "• Shift+Mouse - show cursor with values\n\n"
            "Version 1.0");
    });
}

void MainWindow::openFile() {
    QString lastDir = getLastDirectory();
    QString filePath = QFileDialog::getOpenFileName(this, 
        "Open CSV File", lastDir, "CSV Files (*.csv);;All Files (*)");
    
    if (filePath.isEmpty()) return;
    
    // Сохранить каталог для следующего раза
    setLastDirectory(QFileInfo(filePath).absolutePath());
    
    // Добавить в историю
    addToRecentFiles(filePath);
    updateRecentFilesMenu();
    
    // Сохранить текущий файл
    currentFilePath = filePath;
    updateWindowTitle(filePath);
    
    progressDialog = new QProgressDialog("Loading CSV file...", "Cancel", 0, 100, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    progressDialog->setValue(10);
    
    QTimer::singleShot(50, [this, filePath]() {
        progressDialog->setValue(30);
        progressDialog->setLabelText("Parsing CSV...");
        QCoreApplication::processEvents();
        
        CSVParser parser;
        QString pattern = loadParserPattern();
        parser.setPattern(pattern);
        QVector<DataPoint> data = parser.parseFile(filePath);
        
        progressDialog->setValue(70);
        QCoreApplication::processEvents();
        
        if (data.isEmpty()) {
            progressDialog->close();
            QMessageBox::warning(this, "Error", "No data found");
            return;
        }
        
        progressDialog->setLabelText("Smoothing data...");
        QVector<DataPoint> smoothedData = smoothData(data, 3);
        
        progressDialog->setValue(90);
        progressDialog->setLabelText("Creating plot...");
        QCoreApplication::processEvents();
        
        plotData(smoothedData);
        progressDialog->setValue(100);
        progressDialog->close();
        progressDialog = nullptr;
    });
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString filePath = action->data().toString();
        
        if (!QFile::exists(filePath)) {
            QMessageBox::warning(this, "Error", 
                QString("File not found:\n%1").arg(filePath));
            
            // Удалить из истории несуществующий файл
            QStringList recentFiles = settings->value("recentFiles").toStringList();
            recentFiles.removeAll(filePath);
            settings->setValue("recentFiles", recentFiles);
            updateRecentFilesMenu();
            return;
        }
        
        // Сохранить каталог
        setLastDirectory(QFileInfo(filePath).absolutePath());
        
        // Переместить файл на верх истории
        addToRecentFiles(filePath);
        updateRecentFilesMenu();
        
        currentFilePath = filePath;
        updateWindowTitle(filePath);
        
        // Загрузить файл (тот же код что в openFile)
        progressDialog = new QProgressDialog("Loading CSV file...", "Cancel", 0, 100, this);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->show();
        progressDialog->setValue(10);
        
        QTimer::singleShot(50, [this, filePath]() {
            progressDialog->setValue(30);
            progressDialog->setLabelText("Parsing CSV...");
            QCoreApplication::processEvents();
            
            CSVParser parser;
            QString pattern = loadParserPattern();
            parser.setPattern(pattern);
            QVector<DataPoint> data = parser.parseFile(filePath);
            
            progressDialog->setValue(70);
            QCoreApplication::processEvents();
            
            if (data.isEmpty()) {
                progressDialog->close();
                QMessageBox::warning(this, "Error", "No data found");
                return;
            }
            
            progressDialog->setLabelText("Smoothing data...");
            QVector<DataPoint> smoothedData = smoothData(data, 3);
            
            progressDialog->setValue(90);
            progressDialog->setLabelText("Creating plot...");
            QCoreApplication::processEvents();
            
            plotData(smoothedData);
            progressDialog->setValue(100);
            progressDialog->close();
            progressDialog = nullptr;
        });
    }
}

void MainWindow::onParsingProgress(int percentage) {
    if (progressDialog) {
        progressDialog->setValue(percentage);
    }
}

void MainWindow::onParsingStatus(const QString& status) {
    if (progressDialog) {
        progressDialog->setLabelText(status);
    }
}

void MainWindow::onParsingFinished(const QVector<DataPoint>& data) {
    if (progressDialog) {
        progressDialog->setLabelText("Smoothing data...");
        progressDialog->setValue(70);
    }
    
    // Сглаживание в главном потоке (быстрое)
    QVector<DataPoint> smoothedData = smoothData(data, 3); // явно указываем 3
    
    if (progressDialog) {
        progressDialog->setLabelText("Creating plot...");
        progressDialog->setValue(85);
    }
    
    plotData(smoothedData);
    
    if (progressDialog) {
        progressDialog->setValue(100);
        progressDialog->close();
        progressDialog = nullptr;
    }
    
    workerThread = nullptr; // будет удален автоматически
}

void MainWindow::onParsingError(const QString& message) {
    if (progressDialog) {
        progressDialog->close();
        progressDialog = nullptr;
    }
    
    QMessageBox::warning(this, "Error", message);
    
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void MainWindow::setupUI() {
    customPlot = new QCustomPlot(this);
    setCentralWidget(customPlot);
    
    // Настройка осей
    customPlot->xAxis->setLabel("Date/Time");
    customPlot->yAxis->setLabel("Temperature (°C)");
    customPlot->yAxis2->setLabel("Humidity (%)");
    customPlot->yAxis2->setVisible(true);
    
    // График температуры (левая ось)
    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
    QPen tempPen(Qt::red);
    tempPen.setWidth(2);
    tempPen.setStyle(Qt::SolidLine);
    customPlot->graph(0)->setPen(tempPen);
    customPlot->graph(0)->setName("Temperature");
    
    // НАСТРОЙКА ВЫДЕЛЕНИЯ для температуры
    QPen tempSelectionPen(Qt::red);  // ТОТ ЖЕ ЦВЕТ
    tempSelectionPen.setWidth(3);    // НО ТОЛЩЕ
    customPlot->graph(0)->setSelectionDecorator(new QCPSelectionDecorator);
    customPlot->graph(0)->selectionDecorator()->setPen(tempSelectionPen);
    
    // График влажности (правая ось)
    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
    QPen humPen(Qt::blue);
    humPen.setWidth(2);
    humPen.setStyle(Qt::SolidLine);
    customPlot->graph(1)->setPen(humPen);
    customPlot->graph(1)->setName("Humidity");
    
    // НАСТРОЙКА ВЫДЕЛЕНИЯ для влажности
    QPen humSelectionPen(Qt::blue);  // ТОТ ЖЕ ЦВЕТ
    humSelectionPen.setWidth(3);     // НО ТОЛЩЕ
    customPlot->graph(1)->setSelectionDecorator(new QCPSelectionDecorator);
    customPlot->graph(1)->selectionDecorator()->setPen(humSelectionPen);
    
    // НАСТРОЙКА ИНТЕРАКТИВНОСТИ
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    
    // Зум только по X (время), скроллинг по X
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    
    // Улучшенная легенда
    customPlot->legend->setVisible(true);
    customPlot->legend->setBrush(QBrush(QColor(255,255,255,200)));
    customPlot->legend->setBorderPen(QPen(Qt::black));
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
    
    // Сетка
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setVisible(true);
    customPlot->yAxis2->grid()->setVisible(false);
    
    // Настройка курсора (скрыт по умолчанию)
    crossHairV = new QCPItemLine(customPlot);
    crossHairH = new QCPItemLine(customPlot);
    valueLabel = new QCPItemText(customPlot);
    
    crossHairV->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
    crossHairH->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
    valueLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
    valueLabel->setPadding(QMargins(5, 5, 5, 5));
    valueLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
    valueLabel->setPen(QPen(Qt::black));
    
    hideCrosshair();
    showCursor = false;
    currentTimeScale = Day;
    
    // Подключить события
    connect(customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMove);
    
    // ДОБАВИТЬ: отслеживание изменений диапазона осей
    connect(customPlot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this, &MainWindow::onXAxisRangeChanged);
    
    // Перехват событий клавиатуры и колеса
    customPlot->installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);
    
    setupTimeNavigation();
    
    // Создать статус-бар С ОБНОВЛЕННОЙ СПРАВКОЙ
    statusBar()->showMessage("Ready | D/W/M-scale, Arrows-navigate, Shift+arrows-smooth pan/zoom, T/H-toggle graphs, Home/End-start/end, PgUp/PgDn-periods, Shift+Home-reset");
    
    resize(1200, 800);
}

QString MainWindow::loadParserPattern() {
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    qDebug() << "Looking for config at:" << configPath;
    
    QFile configFile(configPath);
    if (!configFile.exists()) {
        qDebug() << "Config file not found, using default pattern";
        return "DateTime('ddd, MMM dd yyyy HH:mm');%f;%f;%s";
    }
    
    QSettings configSettings(configPath, QSettings::IniFormat);
    configSettings.beginGroup("PARSER");
    
    QString activePattern = configSettings.value("ActivePattern", "DateTime").toString();
    QString pattern;
    
    if (activePattern == "DateTime") {
        // Попробуем прочитать значение напрямую
        pattern = configSettings.value("DateTimePattern").toString();
        qDebug() << "Raw DateTimePattern value:" << pattern;
        
        // Если пусто — используем default
        if (pattern.isEmpty()) {
            pattern = "DateTime('ddd, MMM dd yyyy HH:mm');%f;%f;%s";
            qDebug() << "Using default DateTime pattern";
        }
    } else {
        pattern = configSettings.value("TimestampPattern", "%d;%f;%f").toString();
    }
    
    configSettings.endGroup();
    qDebug() << "Final pattern:" << pattern;
    return pattern;
}

void MainWindow::optimizePlotting() {
    // Включить аппаратное ускорение и оптимизации
    customPlot->setNotAntialiasedElements(QCP::aeAll);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    
    // Оптимизация для больших данных
    customPlot->graph(0)->setAdaptiveSampling(true);
    customPlot->graph(1)->setAdaptiveSampling(true);
}

QVector<DataPoint> MainWindow::smoothData(const QVector<DataPoint>& data, int windowSize) {
    if (data.size() < windowSize || windowSize < 2) return data;
    
    QVector<DataPoint> smoothed;
    smoothed.reserve(data.size()); // резервируем память
    
    int half = windowSize / 2;
    
    for (int i = 0; i < data.size(); ++i) {
        double tempSum = 0, humSum = 0;
        int count = 0;
        
        // Окно сглаживания
        for (int j = qMax(0, i - half); j <= qMin(data.size() - 1, i + half); ++j) {
            tempSum += data[j].temperature;
            humSum += data[j].humidity;
            count++;
        }
        
        DataPoint smoothPoint = data[i];
        smoothPoint.temperature = tempSum / count;
        smoothPoint.humidity = humSum / count;
        smoothed.append(smoothPoint);
    }
    
    return smoothed;
}

void MainWindow::plotData(const QVector<DataPoint>& data) {
    originalData = data; // СОХРАНИТЬ оригинальные данные
    
    QVector<double> timeData, tempData, humData;
    
    for (const DataPoint& point : data) {
        timeData.append(point.timestamp.toSecsSinceEpoch());
        tempData.append(point.temperature);
        humData.append(point.humidity);
    }
    
    customPlot->graph(0)->setData(timeData, tempData);
    customPlot->graph(1)->setData(timeData, humData);
    
    // === 1. НАСТРОЙКА ШКАЛЫ ВРЕМЕНИ (круглые интервалы, дата при смене) ===

if (!timeData.isEmpty()) {
    QSharedPointer<QCPAxisTickerText> timeTicker(new QCPAxisTickerText);
    
    double timeRange = timeData.last() - timeData.first();
    int timeStepSeconds;
    
    // Круглые интервалы времени
    if (timeRange <= 3600) { // меньше часа - каждые 5 минут
        timeStepSeconds = 300; // 5 минут
    } else if (timeRange <= 21600) { // меньше 6 часов - каждые 30 минут
        timeStepSeconds = 1800; // 30 минут
    } else if (timeRange <= 86400) { // меньше суток - каждый час
        timeStepSeconds = 3600; // 1 час
    } else if (timeRange <= 604800) { // неделя - каждые 4 часа
        timeStepSeconds = 14400; // 4 часа
    } else { // больше недели - каждые сутки
        timeStepSeconds = 86400; // 1 день
    }
    
    // Выровнять начальное время на круглый интервал
    QDateTime startDT = QDateTime::fromSecsSinceEpoch(timeData.first());
    QDateTime alignedStart;
    
    if (timeStepSeconds < 3600) { // минуты
        int minutes = (startDT.time().minute() / (timeStepSeconds/60)) * (timeStepSeconds/60);
        alignedStart = QDateTime(startDT.date(), QTime(startDT.time().hour(), minutes, 0));
    } else if (timeStepSeconds < 86400) { // часы
        int hours = (startDT.time().hour() / (timeStepSeconds/3600)) * (timeStepSeconds/3600);
        alignedStart = QDateTime(startDT.date(), QTime(hours, 0, 0));
    } else { // дни
        alignedStart = QDateTime(startDT.date(), QTime(0, 0, 0));
    }
    
    // Генерируем позиции тиков с выровненного времени
    QString lastDateStr = "";
    for (qint64 alignedTime = alignedStart.toSecsSinceEpoch(); 
         alignedTime <= timeData.last(); 
         alignedTime += timeStepSeconds) {
        
        if (alignedTime < timeData.first()) continue;
        
        QDateTime dt = QDateTime::fromSecsSinceEpoch(alignedTime);
        QString currentDateStr = dt.toString("dd.MM.yyyy");
        QString timeStr = dt.toString("hh:mm");
        
        QString label;
        if (currentDateStr != lastDateStr) {
            // Дата изменилась - показываем дату и время
            if (timeRange <= 86400) {
                label = timeStr; // Для одного дня - только время
            } else {
                label = QString("%1\n%2").arg(currentDateStr).arg(timeStr);
            }
            lastDateStr = currentDateStr;
        } else {
            // Дата та же - только время
            label = timeStr;
        }
        
        timeTicker->addTick(alignedTime, label);
    }
    
    customPlot->xAxis->setTicker(timeTicker);
} else {
    // Fallback для пустых данных
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("hh:mm");
    customPlot->xAxis->setTicker(dateTicker);
}

customPlot->xAxis->setTickLabelRotation(-180);
customPlot->xAxis->setSubTicks(true);
customPlot->xAxis->setTickLength(0, 6);
customPlot->xAxis->setSubTickLength(0, 3);
    
    // === 2. НАСТРОЙКА ШКАЛЫ ТЕМПЕРАТУРЫ - УПРОЩЕННО ===
    // Отключить автосетку - будем рисовать вручную через updateTemperatureGrid()
    customPlot->yAxis->grid()->setVisible(false);
    customPlot->yAxis->setTickLabels(true);
    customPlot->yAxis->setSubTicks(false);
    customPlot->yAxis->setTickLength(8, 4);

    // === 3. НАСТРОЙКА ШКАЛЫ ВЛАЖНОСТИ ===
    QSharedPointer<QCPAxisTickerFixed> humTicker(new QCPAxisTickerFixed);
    humTicker->setTickStep(5.0);
    customPlot->yAxis2->setTicker(humTicker);
    customPlot->yAxis2->setTickLabels(true);
    customPlot->yAxis2->setSubTicks(true);
    customPlot->yAxis2->setTickLength(8, 4);
    customPlot->yAxis2->setSubTickLength(4, 2);
    
    // Улучшение производительности
    optimizePlotting();
    
    // === 4. МАСШТАБИРОВАНИЕ С ОТСТУПАМИ ===
    customPlot->rescaleAxes();
    
    if (!tempData.isEmpty()) {
        double tempMin = *std::min_element(tempData.begin(), tempData.end());
        double tempMax = *std::max_element(tempData.begin(), tempData.end());
        double tempRange = tempMax - tempMin;
        double tempMargin = qMax(0.5, tempRange * 0.05);
        
        customPlot->yAxis->setRange(tempMin - tempMargin, tempMax + tempMargin);
    }
    
    if (!humData.isEmpty()) {
        double humMin = *std::min_element(humData.begin(), humData.end());
        double humMax = *std::max_element(humData.begin(), humData.end());
        double humRange = humMax - humMin;
        double humMargin = qMax(2.0, humRange * 0.05);
        
        customPlot->yAxis2->setRange(humMin - humMargin, humMax + humMargin);
    }
    
    // === 5. ИНИЦИАЛИЗИРОВАТЬ сетку температуры ===
    updateTemperatureGrid();
    
    customPlot->replot();
    
    // Показать статистику в статус-баре
    if (!data.isEmpty()) {
        QString statsText = QString("Records: %1 | Period: %2 to %3")
            .arg(data.size())
            .arg(data.first().timestamp.toString("dd.MM.yyyy hh:mm"))
            .arg(data.last().timestamp.toString("dd.MM.yyyy hh:mm"));
        statusBar()->showMessage(statsText, 0);
    }
    
    qDebug() << "Plotted" << data.size() << "data points";
}

void MainWindow::updateTemperatureGrid() {
    if (originalData.isEmpty()) return;
    
    // УДАЛИТЬ все старые линии сетки (если есть)
    for (int i = customPlot->itemCount() - 1; i >= 0; --i) {
        QCPItemLine* line = qobject_cast<QCPItemLine*>(customPlot->item(i));
        if (line && line != crossHairV && line != crossHairH) {
            customPlot->removeItem(line);
        }
    }
    
    // ПОЛУЧИТЬ текущий диапазон температуры
    QCPRange tempRange = customPlot->yAxis->range();
    double tempMin = tempRange.lower;
    double tempMax = tempRange.upper;
    
    // ПОЛУЧИТЬ диапазон времени для линий сетки
    QCPRange xRange = customPlot->xAxis->range();
    
    // === ОБНОВИТЬ ТИКЕР ТЕМПЕРАТУРЫ ===
    QSharedPointer<QCPAxisTickerText> tempTicker(new QCPAxisTickerText);
    
    // Добавляем риски через 0.5° в ТЕКУЩЕМ диапазоне
    for (double temp = qFloor(tempMin * 2) / 2.0; temp <= qCeil(tempMax * 2) / 2.0; temp += 0.5) {
        if (qAbs(temp - qRound(temp)) < 0.01) {
            // Целый градус - с подписью
            tempTicker->addTick(temp, QString::number(qRound(temp)));
        } else {
            // 0.5° - без подписи
            tempTicker->addTick(temp, "");
        }
    }
    
    customPlot->yAxis->setTicker(tempTicker);
    
    // === ДОБАВИТЬ НОВЫЕ ЛИНИИ СЕТКИ только на целых градусах ===
    for (double temp = qFloor(tempMin); temp <= qCeil(tempMax); temp += 1.0) {
        if (temp >= tempMin && temp <= tempMax) {
            QCPItemLine* gridLine = new QCPItemLine(customPlot);
            gridLine->start->setCoords(xRange.lower, temp);
            gridLine->end->setCoords(xRange.upper, temp);
            gridLine->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
        }
    }
}

void MainWindow::zoomVertical(double factor) {
    if (originalData.isEmpty()) return;
    
    // Зум по температуре
    QCPRange tempRange = customPlot->yAxis->range();
    double tempCenter = (tempRange.lower + tempRange.upper) / 2.0;
    double newTempSize = tempRange.size() * factor;
    customPlot->yAxis->setRange(tempCenter - newTempSize/2, tempCenter + newTempSize/2);
    
    // Зум по влажности
    QCPRange humRange = customPlot->yAxis2->range();
    double humCenter = (humRange.lower + humRange.upper) / 2.0;
    double newHumSize = humRange.size() * factor;
    customPlot->yAxis2->setRange(humCenter - newHumSize/2, humCenter + newHumSize/2);
    
    // ДОБАВИТЬ: обновить сетку температуры для нового диапазона
    updateTemperatureGrid();
    
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::panVertical(int direction) {
    if (originalData.isEmpty()) return;
    
    // Сдвиг по температуре (10% от диапазона)
    QCPRange tempRange = customPlot->yAxis->range();
    double tempStep = tempRange.size() * 0.1 * direction;
    customPlot->yAxis->setRange(tempRange.lower + tempStep, tempRange.upper + tempStep);
    
    // Сдвиг по влажности (10% от диапазона)
    QCPRange humRange = customPlot->yAxis2->range();
    double humStep = humRange.size() * 0.1 * direction;
    customPlot->yAxis2->setRange(humRange.lower + humStep, humRange.upper + humStep);
    
    // ДОБАВИТЬ: обновить сетку температуры для нового диапазона
    updateTemperatureGrid();
    
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::panHorizontal(int direction) {
    if (originalData.isEmpty()) return;
    
    // Плавный сдвиг по времени (10% от диапазона)
    QCPRange xRange = customPlot->xAxis->range();
    double step = xRange.size() * 0.1 * direction;
    
    customPlot->xAxis->setRange(xRange.lower + step, xRange.upper + step);
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::resetAllZoom() {
    if (originalData.isEmpty()) return;
    
    // Полный сброс - показать все данные
    customPlot->rescaleAxes();
    
    // Добавить небольшие отступы как в plotData()
    QVector<double> tempData, humData;
    for (const DataPoint& point : originalData) {
        tempData.append(point.temperature);
        humData.append(point.humidity);
    }
    
    if (!tempData.isEmpty()) {
        double tempMin = *std::min_element(tempData.begin(), tempData.end());
        double tempMax = *std::max_element(tempData.begin(), tempData.end());
        double tempRange = tempMax - tempMin;
        double tempMargin = qMax(0.5, tempRange * 0.05);
        
        customPlot->yAxis->setRange(tempMin - tempMargin, tempMax + tempMargin);
    }
    
    if (!humData.isEmpty()) {
        double humMin = *std::min_element(humData.begin(), humData.end());
        double humMax = *std::max_element(humData.begin(), humData.end());
        double humRange = humMax - humMin;
        double humMargin = qMax(2.0, humRange * 0.05);
        
        customPlot->yAxis2->setRange(humMin - humMargin, humMax + humMargin);
    }
    
    // ДОБАВИТЬ: обновить сетку температуры
    updateTemperatureGrid();
    
    customPlot->replot();
    updateStatusStats();
    
    statusBar()->showMessage("All zoom reset", 2000);
}

void MainWindow::goToStart() {
    if (originalData.isEmpty()) return;
    
    double firstTime = originalData.first().timestamp.toSecsSinceEpoch();
    double rangeSize = customPlot->xAxis->range().size();
    
    customPlot->xAxis->setRange(firstTime, firstTime + rangeSize);
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::goToEnd() {
    if (originalData.isEmpty()) return;
    
    double lastTime = originalData.last().timestamp.toSecsSinceEpoch();
    double rangeSize = customPlot->xAxis->range().size();
    
    customPlot->xAxis->setRange(lastTime - rangeSize, lastTime);
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::jumpToPeriodBoundary(int direction) {
    if (originalData.isEmpty()) return;
    
    // ГРАНИЦЫ ДАННЫХ
    double dataStart = originalData.first().timestamp.toSecsSinceEpoch();
    double dataEnd = originalData.last().timestamp.toSecsSinceEpoch();
    double rangeSize = customPlot->xAxis->range().size();
    
    double currentCenter = (customPlot->xAxis->range().lower + customPlot->xAxis->range().upper) / 2.0;
    QDateTime currentDT = QDateTime::fromSecsSinceEpoch(currentCenter);
    QDateTime targetDT;
    
    switch (currentTimeScale) {
    case Day: {
        // Переход к началу/концу суток
        if (direction > 0) {
            // ВПЕРЕД - Следующие сутки
            targetDT = QDateTime(currentDT.date().addDays(1), QTime(0, 0, 0));
        } else {
            // НАЗАД - Предыдущие сутки  
            targetDT = QDateTime(currentDT.date(), QTime(0, 0, 0));
            // Если мы уже в начале суток, переходим к предыдущим
            if (targetDT >= currentDT || qAbs(targetDT.toSecsSinceEpoch() - currentCenter) < 3600) {
                targetDT = QDateTime(currentDT.date().addDays(-1), QTime(0, 0, 0));
            }
        }
        break;
    }
    case Week: {
        // Переход к началу недели (понедельник)
        int daysToMonday = currentDT.date().dayOfWeek() - 1; // 1=Monday
        QDate mondayDate = currentDT.date().addDays(-daysToMonday);
        
        if (direction > 0) {
            // ВПЕРЕД - Следующая неделя
            targetDT = QDateTime(mondayDate.addDays(7), QTime(0, 0, 0));
        } else {
            // НАЗАД - Предыдущая неделя
            targetDT = QDateTime(mondayDate, QTime(0, 0, 0));
            // Если мы уже в начале недели, переходим к предыдущей
            if (targetDT >= currentDT || qAbs(targetDT.toSecsSinceEpoch() - currentCenter) < 86400) {
                targetDT = QDateTime(mondayDate.addDays(-7), QTime(0, 0, 0));
            }
        }
        break;
    }
    case Month: {
        // Переход к началу месяца
        QDate firstOfMonth(currentDT.date().year(), currentDT.date().month(), 1);
        
        if (direction > 0) {
            // ВПЕРЕД - Следующий месяц
            targetDT = QDateTime(firstOfMonth.addMonths(1), QTime(0, 0, 0));
        } else {
            // НАЗАД - Предыдущий месяц
            targetDT = QDateTime(firstOfMonth, QTime(0, 0, 0));
            // Если мы уже в начале месяца, переходим к предыдущему
            if (targetDT >= currentDT || qAbs(targetDT.toSecsSinceEpoch() - currentCenter) < 86400) {
                targetDT = QDateTime(firstOfMonth.addMonths(-1), QTime(0, 0, 0));
            }
        }
        break;
    }
    }
    
    // НОВАЯ ЛОГИКА: проверка границ данных и корректировка
    double targetTime = targetDT.toSecsSinceEpoch();
    double newRangeStart, newRangeEnd;
    
    if (direction > 0) {
        // ВПЕРЕД: целевое время как НАЧАЛО диапазона
        newRangeStart = targetTime;
        newRangeEnd = targetTime + rangeSize;
        
        // ОГРАНИЧЕНИЕ: если конец выходит за данные, сдвинуть назад
        if (newRangeEnd > dataEnd) {
            newRangeEnd = dataEnd;
            newRangeStart = dataEnd - rangeSize;
            // Если даже начало выходит за начало данных, показать все данные
            if (newRangeStart < dataStart) {
                newRangeStart = dataStart;
                newRangeEnd = dataEnd;
            }
        }
    } else {
        // НАЗАД: целевое время как КОНЕЦ диапазона  
        newRangeEnd = targetTime;
        newRangeStart = targetTime - rangeSize;
        
        // ОГРАНИЧЕНИЕ: если начало выходит за данные, сдвинуть вперед
        if (newRangeStart < dataStart) {
            newRangeStart = dataStart;
            newRangeEnd = dataStart + rangeSize;
            // Если даже конец выходит за конец данных, показать все данные
            if (newRangeEnd > dataEnd) {
                newRangeStart = dataStart;
                newRangeEnd = dataEnd;
            }
        }
    }
    
    // ПРИМЕНИТЬ новый диапазон
    customPlot->xAxis->setRange(newRangeStart, newRangeEnd);
    customPlot->replot();
    updateStatusStats();
    
    // ОТЛАДОЧНАЯ информация
    QDateTime actualStart = QDateTime::fromSecsSinceEpoch(newRangeStart);
    QDateTime actualEnd = QDateTime::fromSecsSinceEpoch(newRangeEnd);
    qDebug() << "Jump" << (direction > 0 ? "forward" : "backward") 
             << "target:" << targetDT.toString("dd.MM.yyyy hh:mm")
             << "actual range:" << actualStart.toString("dd.MM.yyyy hh:mm") 
             << "to" << actualEnd.toString("dd.MM.yyyy hh:mm");
}

void MainWindow::toggleTemperatureGraph() {
    bool visible = customPlot->graph(0)->visible();
    customPlot->graph(0)->setVisible(!visible);
    customPlot->replot();
    
    QString status = visible ? "Temperature graph hidden" : "Temperature graph shown";
    statusBar()->showMessage(status, 2000);
}

void MainWindow::toggleHumidityGraph() {
    bool visible = customPlot->graph(1)->visible();
    customPlot->graph(1)->setVisible(!visible);
    customPlot->replot();
    
    QString status = visible ? "Humidity graph hidden" : "Humidity graph shown";
    statusBar()->showMessage(status, 2000);
}

// === НЕДОСТАЮЩИЕ МЕТОДЫ ===

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == customPlot) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            bool shiftPressed = keyEvent->modifiers() & Qt::ShiftModifier;
            
            switch (keyEvent->key()) {
            // === РЕЖИМЫ МАСШТАБИРОВАНИЯ ===
            case Qt::Key_D:
                setTimeScale(Day);
                return true;
            case Qt::Key_W:
                setTimeScale(Week);
                return true;
            case Qt::Key_M:
                setTimeScale(Month);
                return true;
                
            // === НАВИГАЦИЯ ВЛЕВО/ВПРАВО ===
            case Qt::Key_Left:
                if (shiftPressed) {
                    // Shift+Left = плавное перемещение влево
                    panHorizontal(-1);
                } else {
                    // Left = быстрая навигация влево
                    navigateTime(-1);
                }
                return true;
            case Qt::Key_Right:
                if (shiftPressed) {
                    // Shift+Right = плавное перемещение вправо
                    panHorizontal(1);
                } else {
                    // Right = быстрая навигация вправо
                    navigateTime(1);
                }
                return true;
                
            // === НАВИГАЦИЯ ВВЕРХ/ВНИЗ ===
            case Qt::Key_Up:
                if (shiftPressed) {
                    // Shift+Up = Zoom In по вертикали
                    zoomVertical(0.8); // уменьшить диапазон = увеличить масштаб
                } else {
                    // Up = сдвинуть графики вверх
                    panVertical(1);
                }
                return true;
            case Qt::Key_Down:
                if (shiftPressed) {
                    // Shift+Down = Zoom Out по вертикали
                    zoomVertical(1.25); // увеличить диапазон = уменьшить масштаб
                } else {
                    // Down = сдвинуть графики вниз
                    panVertical(-1);
                }
                return true;
                
            // === ПЕРЕХОДЫ К НАЧАЛУ/КОНЦУ ===
            case Qt::Key_Home:
                if (shiftPressed) {
                    // Shift+Home = полный сброс зума
                    resetAllZoom();
                } else {
                    // Home = к началу данных
                    goToStart();
                }
                return true;
            case Qt::Key_End:
                // End = к концу данных
                goToEnd();
                return true;
                
            // === ПЕРЕХОДЫ ПО ГРАНИЦАМ ПЕРИОДОВ ===
            case Qt::Key_PageUp:
                jumpToPeriodBoundary(-1); // назад
                return true;
            case Qt::Key_PageDown:
                jumpToPeriodBoundary(1);  // вперед
                return true;
                
            // === ВКЛЮЧЕНИЕ/ВЫКЛЮЧЕНИЕ ГРАФИКОВ ===
            case Qt::Key_T:
                toggleTemperatureGraph();
                return true;
            case Qt::Key_H:
                toggleHumidityGraph();
                return true;
            }
        } else if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            
            // Плавный зум только по X
            double factor = wheelEvent->angleDelta().y() > 0 ? 0.9 : 1.1;
            customPlot->xAxis->scaleRange(factor, customPlot->xAxis->pixelToCoord(wheelEvent->position().x()));
            customPlot->replot();
            
            // Обновить статистику после зума
            updateStatusStats();
            
            return true;
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onMouseMove(QMouseEvent* event) {
    if (!originalData.isEmpty()) {
        // ИСПРАВЛЕНИЕ: убедиться что customPlot имеет фокус для клавиш
        if (!customPlot->hasFocus()) {
            customPlot->setFocus();
        }
        
        bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;
        
        if (shiftPressed && !showCursor) {
            showCursor = true;
        } else if (!shiftPressed && showCursor) {
            showCursor = false;
            hideCrosshair();
            // Показать статистику видимого диапазона вместо стандартного сообщения
            statusBar()->showMessage(getVisibleRangeStats());
        }
        
        if (showCursor) {
            double x = customPlot->xAxis->pixelToCoord(event->pos().x());
            double y = customPlot->yAxis->pixelToCoord(event->pos().y());
            
            updateCrosshair(x, y);
            
            // Показать значения в статус-баре
            QString valueText = formatValueAtPosition(x);
            statusBar()->showMessage(valueText);
        }
    }
}

void MainWindow::onXAxisRangeChanged(const QCPRange &newRange) {
    // Обновить сетку температуры при изменении диапазона времени
    updateTemperatureGrid();
    updateStatusStats();
}

void MainWindow::setupTimeNavigation() {
    currentTimeScale = Day;
}

void MainWindow::setTimeScale(TimeScale scale) {
    if (originalData.isEmpty()) return;
    
    currentTimeScale = scale;
    
    // Получить текущий центр
    double currentCenter = (customPlot->xAxis->range().lower + customPlot->xAxis->range().upper) / 2.0;
    
    // Определить новый размер диапазона
    double newRangeSize;
    switch (scale) {
    case Day:
        newRangeSize = 86400; // 24 часа в секундах
        break;
    case Week:
        newRangeSize = 604800; // 7 дней в секундах
        break;
    case Month:
        newRangeSize = 2592000; // 30 дней в секундах
        break;
    }
    
    // Применить новый диапазон
    double newStart = currentCenter - newRangeSize / 2.0;
    double newEnd = currentCenter + newRangeSize / 2.0;
    
    // Ограничить границами данных
    double dataStart = originalData.first().timestamp.toSecsSinceEpoch();
    double dataEnd = originalData.last().timestamp.toSecsSinceEpoch();
    
    if (newStart < dataStart) {
        newStart = dataStart;
        newEnd = newStart + newRangeSize;
    }
    if (newEnd > dataEnd) {
        newEnd = dataEnd;
        newStart = newEnd - newRangeSize;
    }
    if (newStart < dataStart) {
        newStart = dataStart;
    }
    
    customPlot->xAxis->setRange(newStart, newEnd);
    customPlot->replot();
    updateStatusStats();
    
    QString scaleText;
    switch (scale) {
    case Day: scaleText = "Day"; break;
    case Week: scaleText = "Week"; break;
    case Month: scaleText = "Month"; break;
    }
    statusBar()->showMessage(QString("Time scale: %1").arg(scaleText), 2000);
}

void MainWindow::navigateTime(int direction) {
    if (originalData.isEmpty()) return;
    
    // Навигация на 1/4 от текущего диапазона
    QCPRange currentRange = customPlot->xAxis->range();
    double step = currentRange.size() * 0.25 * direction;
    
    double newStart = currentRange.lower + step;
    double newEnd = currentRange.upper + step;
    
    // Ограничить границами данных
    double dataStart = originalData.first().timestamp.toSecsSinceEpoch();
    double dataEnd = originalData.last().timestamp.toSecsSinceEpoch();
    
    if (newStart < dataStart) {
        newStart = dataStart;
        newEnd = newStart + currentRange.size();
    }
    if (newEnd > dataEnd) {
        newEnd = dataEnd;
        newStart = newEnd - currentRange.size();
    }
    if (newStart < dataStart) {
        newStart = dataStart;
    }
    
    customPlot->xAxis->setRange(newStart, newEnd);
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::updateCrosshair(double x, double y) {
    // Найти ближайшую точку данных
    double minDistance = std::numeric_limits<double>::max();
    DataPoint closestPoint;
    bool found = false;
    
    for (const DataPoint& point : originalData) {
        double pointTime = point.timestamp.toSecsSinceEpoch();
        double distance = qAbs(pointTime - x);
        
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
            found = true;
        }
    }
    
    if (!found) return;
    
    double snapX = closestPoint.timestamp.toSecsSinceEpoch();
    
    // Обновить позицию курсора
    crossHairV->start->setCoords(snapX, customPlot->yAxis->range().lower);
    crossHairV->end->setCoords(snapX, customPlot->yAxis->range().upper);
    
    crossHairH->start->setCoords(customPlot->xAxis->range().lower, closestPoint.temperature);
    crossHairH->end->setCoords(customPlot->xAxis->range().upper, closestPoint.temperature);
    
    // Показать линии
    crossHairV->setVisible(true);
    crossHairH->setVisible(true);
    
    // Обновить подпись
    QString labelText = QString("Time: %1\nTemp: %2°C\nHumidity: %3%")
        .arg(closestPoint.timestamp.toString("dd.MM.yyyy hh:mm:ss"))
        .arg(closestPoint.temperature, 0, 'f', 1)
        .arg(closestPoint.humidity, 0, 'f', 1);
    
    valueLabel->setText(labelText);
    valueLabel->position->setCoords(snapX, closestPoint.temperature);
    valueLabel->setVisible(true);
    
    customPlot->replot();
}

void MainWindow::hideCrosshair() {
    crossHairV->setVisible(false);
    crossHairH->setVisible(false);
    valueLabel->setVisible(false);
    customPlot->replot();
}

QString MainWindow::formatValueAtPosition(double timePos) {
    // Найти ближайшую точку данных
    double minDistance = std::numeric_limits<double>::max();
    DataPoint closestPoint;
    bool found = false;
    
    for (const DataPoint& point : originalData) {
        double pointTime = point.timestamp.toSecsSinceEpoch();
        double distance = qAbs(pointTime - timePos);
        
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
            found = true;
        }
    }
    
    if (!found) return "No data";
    
    return QString("Time: %1 | Temp: %2°C | Humidity: %3%")
        .arg(closestPoint.timestamp.toString("dd.MM.yyyy hh:mm:ss"))
        .arg(closestPoint.temperature, 0, 'f', 1)
        .arg(closestPoint.humidity, 0, 'f', 1);
}

QString MainWindow::getVisibleRangeStats() {
    if (originalData.isEmpty()) return "No data";
    
    QCPRange xRange = customPlot->xAxis->range();
    QDateTime startTime = QDateTime::fromSecsSinceEpoch(xRange.lower);
    QDateTime endTime = QDateTime::fromSecsSinceEpoch(xRange.upper);
    
    // Подсчитать количество точек в видимом диапазоне
    int visiblePoints = 0;
    double tempSum = 0, humSum = 0;
    double tempMin = std::numeric_limits<double>::max();
    double tempMax = std::numeric_limits<double>::lowest();
    double humMin = std::numeric_limits<double>::max();
    double humMax = std::numeric_limits<double>::lowest();
    
    for (const DataPoint& point : originalData) {
        double pointTime = point.timestamp.toSecsSinceEpoch();
        if (pointTime >= xRange.lower && pointTime <= xRange.upper) {
            visiblePoints++;
            tempSum += point.temperature;
            humSum += point.humidity;
            tempMin = qMin(tempMin, point.temperature);
            tempMax = qMax(tempMax, point.temperature);
            humMin = qMin(humMin, point.humidity);
            humMax = qMax(humMax, point.humidity);
        }
    }
    
    if (visiblePoints == 0) return "No data in visible range";
    
    double tempAvg = tempSum / visiblePoints;
    double humAvg = humSum / visiblePoints;
    
    return QString("Visible: %1 points | Period: %2 - %3 | Temp: %4°C (avg), %5°C-%6°C | Hum: %7% (avg), %8%-%9%")
        .arg(visiblePoints)
        .arg(startTime.toString("dd.MM hh:mm"))
        .arg(endTime.toString("dd.MM hh:mm"))
        .arg(tempAvg, 0, 'f', 1)
        .arg(tempMin, 0, 'f', 1)
        .arg(tempMax, 0, 'f', 1)
        .arg(humAvg, 0, 'f', 1)
        .arg(humMin, 0, 'f', 1)
        .arg(humMax, 0, 'f', 1);
}

void MainWindow::updateStatusStats() {
    if (!showCursor) {
        statusBar()->showMessage(getVisibleRangeStats(), 0);
    }
}

// === МЕТОДЫ ПЕЧАТИ И ЭКСПОРТА ===

void MainWindow::printChart() {
    if (originalData.isEmpty()) {
        QMessageBox::information(this, "Print", "No data to print");
        return;
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("Print Chart");
    
    if (dialog.exec() == QDialog::Accepted) {
        bool useColor = !blackWhitePrintAction->isChecked();
        renderChartToPrinter(&printer, useColor);
        statusBar()->showMessage("Chart printed", 2000);
    }
}

void MainWindow::printPreview() {
    if (originalData.isEmpty()) {
        QMessageBox::information(this, "Print Preview", "No data to preview");
        return;
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("Print Preview");
    
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter *p) {
        bool useColor = !blackWhitePrintAction->isChecked();
        renderChartToPrinter(p, useColor);
    });
    
    preview.exec();
}

void MainWindow::copyChartToClipboard() {
    if (originalData.isEmpty()) {
        QMessageBox::information(this, "Copy", "No data to copy");
        return;
    }
    
    // Создать изображение с высоким разрешением
    QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
    
    QApplication::clipboard()->setPixmap(pixmap);
    statusBar()->showMessage("Chart copied to clipboard", 2000);
}

void MainWindow::exportChart() {
    if (originalData.isEmpty()) {
        QMessageBox::information(this, "Export", "No data to export");
        return;
    }
    
    QString lastDir = getLastDirectory();
    QString fileName = QFileInfo(currentFilePath).baseName() + "_chart";
    QString defaultPath = lastDir + "/" + fileName;
    
    QString filePath = QFileDialog::getSaveFileName(this,
        "Export Chart", defaultPath,
        "PNG Images (*.png);;JPG Images (*.jpg);;PDF Files (*.pdf);;All Files (*)");
    
    if (filePath.isEmpty()) return;
    
    // Сохранить каталог
    setLastDirectory(QFileInfo(filePath).absolutePath());
    
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    bool success = false;
    
    if (extension == "pdf") {
        // Экспорт в PDF
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageOrientation(QPageLayout::Landscape);
        
        renderChartToPrinter(&printer, true); // PDF всегда цветной
        success = true;
    } else {
        // Экспорт в растровый формат
        QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
        success = pixmap.save(filePath);
    }
    
    if (success) {
        statusBar()->showMessage(QString("Chart exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, "Export Error", "Failed to export chart");
    }
}

void MainWindow::renderChartToPrinter(QPrinter *printer, bool useColor) {
    QPainter painter(printer);
    
    // Получить размер страницы
    QRectF pageRect = painter.viewport();
    qDebug() << "Page rect:" << pageRect;
    
    // ПРОСТОЕ масштабирование
    double scale = qMin(pageRect.width() / 1000.0, pageRect.height() / 700.0);
    scale = qBound(0.8, scale, 2.5); // ОГРАНИЧИТЬ масштаб
    qDebug() << "Scale:" << scale;
    
    // Проверка на PDF экспорт
    bool isPdfExport = (printer->outputFormat() == QPrinter::PdfFormat && 
                       !printer->outputFileName().isEmpty());
    
    // Размеры для заголовка и графика
    QRectF titleRect, chartRect;
    
    if (isPdfExport) {
        // PDF без заголовка - весь лист для графика
        chartRect = pageRect;
    } else {
        // Печать с заголовком - БЕЗОПАСНЫЙ размер шрифта
        QFont titleFont = painter.font();
        int titleSize = qBound(8, int(10 * scale), 16);
        titleFont.setPointSize(titleSize);
        painter.setFont(titleFont);
        
        QFontMetrics fm(titleFont);
        int titleHeight = qBound(15, fm.height() + 15, 60);
        
        titleRect = QRectF(pageRect.x(), pageRect.y(), pageRect.width(), titleHeight);
        chartRect = QRectF(pageRect.x(), pageRect.y() + titleHeight,
                          pageRect.width(), pageRect.height() - titleHeight);
        
        // Нарисовать заголовок
        QString fileName = QFileInfo(currentFilePath).fileName();
        if (fileName.isEmpty()) fileName = "Chart";
        
        painter.drawText(titleRect, Qt::AlignCenter, fileName);
    }
    
    // === ПОЛУЧИТЬ ДАННЫЕ ===
    QCPDataContainer<QCPGraphData>::const_iterator tempBegin = customPlot->graph(0)->data()->constBegin();
    QCPDataContainer<QCPGraphData>::const_iterator tempEnd = customPlot->graph(0)->data()->constEnd();
    QCPDataContainer<QCPGraphData>::const_iterator humBegin = customPlot->graph(1)->data()->constBegin();
    QCPDataContainer<QCPGraphData>::const_iterator humEnd = customPlot->graph(1)->data()->constEnd();
    
    if (tempBegin == tempEnd) {
        painter.end();
        return;
    }
    
    // Получить диапазоны осей с экрана
    QCPRange xRange = customPlot->xAxis->range();
    QCPRange yRange = customPlot->yAxis->range();
    QCPRange y2Range = customPlot->yAxis2->range();
    
    // УВЕЛИЧЕННЫЕ отступы
    double marginLeft = qBound(60.0, 90.0 * scale, 140.0);
    double marginRight = qBound(60.0, 90.0 * scale, 140.0);
    double marginTop = qBound(15.0, 30.0 * scale, 60.0);
    double marginBottom = qBound(80.0, 140.0 * scale, 200.0);
    
    QRectF plotArea(chartRect.x() + marginLeft, chartRect.y() + marginTop,
                   chartRect.width() - marginLeft - marginRight,
                   chartRect.height() - marginTop - marginBottom);
    
    // === БЕЗОПАСНЫЕ ШРИФТЫ ===
    QFont axisFont = painter.font();
    QFont tickFont = painter.font();
    QFont legendFont = painter.font();
    
    int axisSize = qBound(6, int(9 * scale), 14);
    int tickSize = qBound(5, int(7 * scale), 11);
    int legendSize = qBound(5, int(8 * scale), 12);
    
    axisFont.setPointSize(axisSize);
    tickFont.setPointSize(tickSize);
    legendFont.setPointSize(legendSize);
    
    // === ВЫЧИСЛИТЬ ТОЧНЫЕ ПОЗИЦИИ ТЕМПЕРАТУРЫ ===
    // Получить диапазон температуры и вычислить градусные позиции
    double tempMinRange = yRange.lower;
    double tempMaxRange = yRange.upper;
    
    // Найти минимальный и максимальный целый градус в диапазоне
    int tempMinInt = qFloor(tempMinRange);
    int tempMaxInt = qCeil(tempMaxRange);
    
    // Функция для преобразования температуры в Y координату
    auto tempToY = [&](double temp) -> double {
        return plotArea.bottom() - (temp - tempMinRange) / (tempMaxRange - tempMinRange) * plotArea.height();
    };
    
    // === РИСУЕМ СЕТКУ ТЕМПЕРАТУРЫ - ТОЧНО ПО ГРАДУСАМ ===
    painter.setPen(QPen(Qt::lightGray, qBound(0.5, 1.0 * scale, 2.0), Qt::DotLine));
    
    // Горизонтальные линии сетки ТОЛЬКО на целых градусах
    for (int tempValue = tempMinInt; tempValue <= tempMaxInt; ++tempValue) {
        if (tempValue >= tempMinRange && tempValue <= tempMaxRange) {
            double y = tempToY(tempValue);
            painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
        }
    }
    
    // Вертикальные линии сетки (время)
    int numVertLines = 8;
    for (int i = 0; i <= numVertLines; ++i) {
        double x = plotArea.left() + (plotArea.width() * i / numVertLines);
        painter.drawLine(QPointF(x, plotArea.top()), QPointF(x, plotArea.bottom()));
    }
    
    // === РИСУЕМ ОСИ С РИСКАМИ ===
    painter.setPen(QPen(Qt::black, qBound(1.0, 1.5 * scale, 3.0)));
    painter.drawRect(plotArea);
    
    // РИСКИ НА ОСЯХ
    double tickLength = qBound(3.0, 6.0 * scale, 12.0);
    double subTickLength = tickLength * 0.6; // Полуградусные риски короче
    
    // Риски на нижней оси (время)
    int numXTicks = 8;
    for (int i = 0; i <= numXTicks; ++i) {
        double x = plotArea.left() + (plotArea.width() * i / numXTicks);
        painter.drawLine(QPointF(x, plotArea.bottom()), 
                        QPointF(x, plotArea.bottom() + tickLength));
    }
    
    // РИСКИ НА ЛЕВОЙ ОСИ ТЕМПЕРАТУРЫ - С ПОЛУГРАДУСНЫМИ
    painter.setPen(QPen(Qt::black, qBound(1.0, 1.5 * scale, 3.0)));
    
    // Проходим по всем 0.5° в диапазоне
    double tempStartRange = qFloor(tempMinRange * 2) / 2.0;  // Округлить до ближайшего 0.5°
    double tempEndRange = qCeil(tempMaxRange * 2) / 2.0;
    
    for (double tempValue = tempStartRange; tempValue <= tempEndRange; tempValue += 0.5) {
        if (tempValue >= tempMinRange && tempValue <= tempMaxRange) {
            double y = tempToY(tempValue);
            
            bool isWholeDegree = (qAbs(tempValue - qRound(tempValue)) < 0.01);
            double currentTickLength = isWholeDegree ? tickLength : subTickLength;
            
            painter.drawLine(QPointF(plotArea.left(), y), 
                           QPointF(plotArea.left() - currentTickLength, y));
        }
    }
    
    // Риски на правой оси (влажность) - каждые 5%
    double humMinRange = y2Range.lower;
    double humMaxRange = y2Range.upper;
    
    auto humToY = [&](double hum) -> double {
        return plotArea.bottom() - (hum - humMinRange) / (humMaxRange - humMinRange) * plotArea.height();
    };
    
    int humStartInt = qFloor(humMinRange / 5) * 5;  // Округлить до ближайших 5%
    int humEndInt = qCeil(humMaxRange / 5) * 5;
    
    for (int humValue = humStartInt; humValue <= humEndInt; humValue += 5) {
        if (humValue >= humMinRange && humValue <= humMaxRange) {
            double y = humToY(humValue);
            painter.drawLine(QPointF(plotArea.right(), y), 
                           QPointF(plotArea.right() + tickLength, y));
        }
    }
    
    // === ПОДПИСИ НА ОСЯХ ===
    painter.setFont(tickFont);
    painter.setPen(QPen(Qt::black));
    
    // ПОДПИСИ ВРЕМЕНИ
    double timeRange = xRange.size();
    bool showOnlyTime = (timeRange <= 86400);
    QFontMetrics tickFM(tickFont);
    
    for (int i = 0; i <= numXTicks; ++i) {
        double timeValue = xRange.lower + (xRange.size() * i / numXTicks);
        double x = plotArea.left() + (plotArea.width() * i / numXTicks);
        
        QDateTime dt = QDateTime::fromSecsSinceEpoch(timeValue);
        
        QString label;
        if (showOnlyTime) {
            label = dt.toString("hh:mm");
        } else {
            label = dt.toString("dd.MM\nhh:mm");
        }
        
        QRect textBounds = tickFM.boundingRect(label);
        int textWidth = textBounds.width();
        double textRectWidth = qMax(50.0, textWidth + 10.0);
        
        QRectF textRect(x - textRectWidth/2, plotArea.bottom() + tickLength + 5, 
                       textRectWidth, 35);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
    
    // ПОДПИСИ ТЕМПЕРАТУРЫ - ТОЛЬКО НА ЦЕЛЫХ ГРАДУСАХ
    for (int tempValue = tempMinInt; tempValue <= tempMaxInt; ++tempValue) {
        if (tempValue >= tempMinRange && tempValue <= tempMaxRange) {
            double y = tempToY(tempValue);
            QString tempStr = QString::number(tempValue);
            
            QRectF textRect(plotArea.left() - tickLength - 35, y - 8, 30, 16);
            painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, tempStr);
        }
    }
    
    // ПОДПИСИ ВЛАЖНОСТИ - КАЖДЫЕ 5%
    for (int humValue = humStartInt; humValue <= humEndInt; humValue += 5) {
        if (humValue >= humMinRange && humValue <= humMaxRange) {
            double y = humToY(humValue);
            QString humStr = QString::number(humValue);
            
            QRectF textRect(plotArea.right() + tickLength + 5, y - 8, 30, 16);
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, humStr);
        }
    }
    
    // === ФУНКЦИЯ преобразования координат ===
    auto mapToPlot = [&](double x, double y, const QCPRange& yAxisRange) -> QPointF {
        double px = plotArea.left() + (x - xRange.lower) / xRange.size() * plotArea.width();
        double py = plotArea.bottom() - (y - yAxisRange.lower) / yAxisRange.size() * plotArea.height();
        return QPointF(px, py);
    };
    
    // === РИСУЕМ ГРАФИКИ ===
    double lineWidth = qBound(1.5, 2.5 * scale, 4.0);
    
    // График температуры
    if (useColor) {
        painter.setPen(QPen(Qt::red, lineWidth));
    } else {
        painter.setPen(QPen(Qt::black, lineWidth));
    }
    
    QPointF prevTempPoint;
    bool firstTempPoint = true;
    
    for (auto it = tempBegin; it != tempEnd; ++it) {
        if (it->key >= xRange.lower && it->key <= xRange.upper) {
            QPointF point = mapToPlot(it->key, it->value, yRange);
            
            if (!firstTempPoint) {
                painter.drawLine(prevTempPoint, point);
            }
            prevTempPoint = point;
            firstTempPoint = false;
        }
    }
    
    // График влажности
    if (useColor) {
        painter.setPen(QPen(Qt::blue, lineWidth));
    } else {
        painter.setPen(QPen(Qt::black, lineWidth * 0.7, Qt::DashLine));
    }

    
    QPointF prevHumPoint;
    bool firstHumPoint = true;
    
    for (auto it = humBegin; it != humEnd; ++it) {
        if (it->key >= xRange.lower && it->key <= xRange.upper) {
            QPointF point = mapToPlot(it->key, it->value, y2Range);
            
            if (!firstHumPoint) {
                painter.drawLine(prevHumPoint, point);
            }
            prevHumPoint = point;
            firstHumPoint = false;
        }
    }
    
    // === ПОДПИСИ ОСЕЙ ===
    painter.setFont(axisFont);
    painter.setPen(QPen(Qt::black));
    
    // Подпись оси X
    QRectF xLabelRect(plotArea.left(), plotArea.bottom() + 50, plotArea.width(), 25);
    painter.drawText(xLabelRect, Qt::AlignCenter, "Date/Time");
    
    // ЛЕВАЯ ось Y
    QRectF tempLabelRect(10, plotArea.top(), 40, plotArea.height());
    painter.drawText(tempLabelRect, Qt::AlignCenter, "T °C");
    
    // ПРАВАЯ ось Y
    QRectF humLabelRect(plotArea.right() + 50, plotArea.top(), 40, plotArea.height());
    painter.drawText(humLabelRect, Qt::AlignCenter, "H %");
    
    // === ЛЕГЕНДА ===
    painter.setFont(legendFont);
    
    QFontMetrics legendFM(legendFont);
    QString tempText = useColor ? "Temperature" : "Temperature (solid)";
    QString humText = useColor ? "Humidity" : "Humidity (dashed)";
    
    QRect tempBounds = legendFM.boundingRect(tempText);
    QRect humBounds = legendFM.boundingRect(humText);
    int textWidth = qMax(tempBounds.width(), humBounds.width());
    
    int lineLength = qBound(15, int(20 * scale), 35);
    int legendWidth = lineLength + 10 + textWidth + 15;
    int legendHeight = (legendFM.height() + 5) * 2 + 15;
    
    QRectF legendRect(plotArea.right() - legendWidth - 10, plotArea.top() + 10, 
                     legendWidth, legendHeight);
    
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(QColor(255, 255, 255, 240)));
    painter.drawRect(legendRect);
    
    // Элементы легенды
    double legendY = legendRect.top() + 10;
    double lineStartX = legendRect.left() + 8;
    double lineEndX = lineStartX + lineLength;
    double textX = lineEndX + 6;
    
    // Температура
    if (useColor) {
        painter.setPen(QPen(Qt::red, lineWidth));
    } else {
        painter.setPen(QPen(Qt::black, lineWidth));
    }
    painter.drawLine(QPointF(lineStartX, legendY), QPointF(lineEndX, legendY));
    
    painter.setPen(QPen(Qt::black));
    painter.drawText(QPointF(textX, legendY + 4), tempText);
    
    // Влажность
    legendY += legendFM.height() + 5;
    if (useColor) {
        painter.setPen(QPen(Qt::blue, lineWidth));
    } else {
        painter.setPen(QPen(Qt::black, lineWidth * 0.7, Qt::DashLine));
    }
    painter.drawLine(QPointF(lineStartX, legendY), QPointF(lineEndX, legendY));
    
    painter.setPen(QPen(Qt::black));
    painter.drawText(QPointF(textX, legendY + 4), humText);
    
    painter.end();
    
    qDebug() << "Manual rendering completed with scale:" << scale;
}

