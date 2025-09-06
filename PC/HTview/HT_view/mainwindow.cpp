#include "mainwindow.h"
#include "qcustomplot.h"
#include "csvparser.h"
#include "csvworker.h"
#include <QDebug>

// === МАКРОС ДЛЯ ОТЛАДКИ ===
#define DEBUG_MODE 1  // 1 = включить отладку, 0 = отключить

#if DEBUG_MODE
    #define DEBUG_LOG(x) (qDebug() << x)
#else
    #define DEBUG_LOG(x) ((void)0)
#endif
// === КОНЕЦ МАКРОСА ===

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
#include <QPainter>         // ДОБАВИТЬ: для рисования на принтере

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
    
    // ИСПРАВИТЬ строку 138:
    int numRecentFiles = qMin(recentFiles.size(), static_cast<int>(MaxRecentFiles));
    
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
    
    // ДОБАВИТЬ: новый пункт для добавления файлов к графику
    QAction *addAction = fileMenu->addAction("&Add to Chart...");
    addAction->setShortcut(QKeySequence("Ctrl+A"));
    connect(addAction, &QAction::triggered, this, &MainWindow::addFileToChart);
    
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
    
    // ДОБАВИТЬ: очистка всех графиков
    QAction *clearAllAction = fileMenu->addAction("&Clear All");
    clearAllAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(clearAllAction, &QAction::triggered, this, &MainWindow::clearAllGraphs);
    
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
            "• Shift+Mouse - show cursor with values\n"
            "• P - toggle Points/Lines display\n"
            "• T/H - toggle Temperature/Humidity graphs\n\n"
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
    
    // ИСПРАВИТЬ: использовать ту же логику что в openRecentFile
    currentFilePath = filePath;
    updateWindowTitle(filePath);
    
    // Очистить все наборы данных
    dataSets.clear();
    currentDataSetIndex = -1;
    overlayMode = false;
    
    // Загрузить файл (тот же код что в openRecentFile)
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
        
        // Сохранить исходные данные
        rawData = data;
        DEBUG_LOG("Saved rawData with" << rawData.size() << "points");
        
        // Сбросить режим отображения при загрузке нового файла
        showPointsMode = false;
        
        progressDialog->setLabelText("Smoothing data...");
        
        // Применить сглаживание согласно настройкам
        QVector<DataPoint> smoothedData;
        if (vizSettings.enableSmoothing) {
            smoothedData = smoothData(data, vizSettings.smoothingWindow);
        } else {
            smoothedData = data;
        }
        
        originalData = smoothedData;
        DEBUG_LOG("Saved originalData with" << originalData.size() << "points");
        
        progressDialog->setValue(90);
        progressDialog->setLabelText("Creating plot...");
        QCoreApplication::processEvents();
        
        // ИСПРАВИТЬ: использовать старый метод plotData вместо новой системы
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
            
            // ИСПРАВИТЬ: использовать те же настройки что в openFile()
            rawData = data;
            DEBUG_LOG("Saved rawData with" << rawData.size() << "points");
            
            // Сбросить режим отображения при загрузке нового файла
            showPointsMode = false;
            
            progressDialog->setLabelText("Smoothing data...");
            
            // ИСПРАВИТЬ: применить сглаживание согласно настройкам
            QVector<DataPoint> smoothedData;
            if (vizSettings.enableSmoothing) {
                smoothedData = smoothData(data, vizSettings.smoothingWindow);
            } else {
                smoothedData = data;
            }
            
            originalData = smoothedData;
            DEBUG_LOG("Saved originalData with" << originalData.size() << "points");
            
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
        progressDialog->setLabelText("Processing data...");
        progressDialog->setValue(70);
    }
    
    // СОХРАНИТЬ исходные данные для режима точек
    rawData = data;
    
    // ИСПРАВИТЬ: использовать настройки визуализации
    QVector<DataPoint> smoothedData;
    if (vizSettings.enableSmoothing) {
        smoothedData = smoothData(data, vizSettings.smoothingWindow);
    } else {
        smoothedData = data;
    }
    originalData = smoothedData;
    
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
    
    workerThread = nullptr;
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
    customPlot->graph(0)->setName("Temperature");
    
    // График влажности (правая ось)
    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
    customPlot->graph(1)->setName("Humidity");
    
    // УЛУЧШИТЬ: настройка выделения с более заметным эффектом
    for (int i = 0; i < customPlot->graphCount(); ++i) {
        QCPSelectionDecorator *decorator = new QCPSelectionDecorator;
        decorator->setPen(QPen(Qt::yellow, 4)); // Желтая обводка при выделении
        customPlot->graph(i)->setSelectionDecorator(decorator);
    }
    
    // Настройка курсора (скрыт по умолчанию)
    crossHairV = new QCPItemLine(customPlot);
    crossHairH = new QCPItemLine(customPlot);
    valueLabel = new QCPItemText(customPlot);
    
    valueLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
    valueLabel->setPadding(QMargins(5, 5, 5, 5));
    valueLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
    valueLabel->setPen(QPen(Qt::black));
    
    // ДОБАВИТЬ: временно загрузить дефолтные настройки
    loadDefaultVisualizationSettings();
    
    // ПРИМЕНИТЬ настройки (будут перезагружены в loadParserPattern)
    applyVisualizationSettings();
    
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
    statusBar()->showMessage("Ready | D/W/M-scale, Arrows-navigate, Shift+arrows-smooth pan/zoom, T/H-toggle graphs, P-points/lines, Del-delete selected, Home/End-start/end, PgUp/PgDn-periods, Shift+Home-reset");
    
    resize(1200, 800);
}

QString MainWindow::loadParserPattern() {
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    DEBUG_LOG("Looking for config at:" << configPath);
    
    QFile configFile(configPath);
    if (!configFile.exists()) {
        DEBUG_LOG("Config file not found, using default pattern");
        loadDefaultVisualizationSettings();  // ДОБАВИТЬ: загрузить дефолтные настройки
        return "DateTime('ddd, MMM dd yyyy HH:mm');%f;%f;%s";
    }
    
    QSettings configSettings(configPath, QSettings::IniFormat);
    
    // === ЗАГРУЗИТЬ НАСТРОЙКИ ВИЗУАЛИЗАЦИИ ===
    loadVisualizationSettings(configSettings);
    
    configSettings.beginGroup("PARSER");
    
    QString activePattern = configSettings.value("ActivePattern", "DateTime").toString();
    QString pattern;
    
    if (activePattern == "DateTime") {
        pattern = configSettings.value("DateTimePattern").toString();
        DEBUG_LOG("Raw DateTimePattern value:" << pattern);
        
        if (pattern.isEmpty()) {
            pattern = "DateTime('ddd, MMM dd yyyy HH:mm');%f;%f;%s";
            DEBUG_LOG("Using default DateTime pattern");
        }
    } else {
        pattern = configSettings.value("TimestampPattern", "%d;%f;%f").toString();
    }
    
    configSettings.endGroup();
    DEBUG_LOG("Final pattern:" << pattern);
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
    DEBUG_LOG("plotData called with" << data.size() << "points");
    originalData = data;

    if (data.isEmpty()) {
        statusBar()->showMessage("No data to display");
        return;
    }

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
    updateTimeScale(); // ЗАМЕНИТЬ весь блок настройки времени на этот вызов
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
    
    // ИСПРАВИТЬ: показать детальную статистику вместо простого количества записей
    updateStatusStats(); // Использовать тот же метод что и для навигации
    
    DEBUG_LOG("Plotted" << data.size() << "data points");
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
    DEBUG_LOG("Jump" << (direction > 0 ? "forward" : "backward") 
         << "target:" << targetDT.toString("dd.MM.yyyy hh:mm")
         << "actual range:" << actualStart.toString("dd.MM.yyyy hh:mm") 
         << "to" << actualEnd.toString("dd.MM.yyyy hh:mm"));
}

void MainWindow::toggleTemperatureGraph() {
    if (customPlot->graphCount() < 1) return;
    
    // ИСПРАВИТЬ: переключать все графики температуры (четные индексы)
    bool anyVisible = false;
    
    // Проверить, есть ли хотя бы один видимый график температуры
    for (int i = 0; i < customPlot->graphCount(); i += 2) {
        if (customPlot->graph(i)->visible()) {
            anyVisible = true;
            break;
        }
    }
    
    // Переключить все графики температуры
    for (int i = 0; i < customPlot->graphCount(); i += 2) {
        customPlot->graph(i)->setVisible(!anyVisible);
    }
    
    customPlot->replot();
    
    QString status = anyVisible ? "Temperature graphs hidden" : "Temperature graphs shown";
    statusBar()->showMessage(status, 2000);
}

void MainWindow::toggleHumidityGraph() {
    if (customPlot->graphCount() < 2) return;
    
    // ИСПРАВИТЬ: переключать все графики влажности (нечетные индексы)
    bool anyVisible = false;
    
    // Проверить, есть ли хотя бы один видимый график влажности
    for (int i = 1; i < customPlot->graphCount(); i += 2) {
        if (customPlot->graph(i)->visible()) {
            anyVisible = true;
            break;
        }
    }
    
    // Переключить все графики влажности
    for (int i = 1; i < customPlot->graphCount(); i += 2) {
        customPlot->graph(i)->setVisible(!anyVisible);
    }
    
    customPlot->replot();
    
    QString status = anyVisible ? "Humidity graphs hidden" : "Humidity graphs shown";
    statusBar()->showMessage(status, 2000);
}

void MainWindow::toggleDisplayMode() {
    // ИСПРАВИТЬ: проверить оба способа хранения данных
    if (dataSets.isEmpty() && originalData.isEmpty()) {
        return;
    }
    
    // ИСПРАВИТЬ: сохранить текущий зум перед обновлением
    QCPRange xRange = customPlot->xAxis->range();
    QCPRange yRange = customPlot->yAxis->range();
    QCPRange y2Range = customPlot->yAxis2->range();
    
    showPointsMode = !showPointsMode;
    
    // ИСПРАВИТЬ: использовать правильный метод в зависимости от того, как загружены данные
    if (!dataSets.isEmpty()) {
        // Новая система множественных графиков
        updateAllGraphs();
    } else if (!originalData.isEmpty()) {
        // Старая система одного файла - ИСПРАВИТЬ: использовать правильные данные
        const QVector<DataPoint>& dataToUse = showPointsMode ? rawData : originalData;
        
        // ИСПРАВИТЬ: обновить только стили графиков, а не перестраивать весь график
        QVector<double> timeData, tempData, humData;
        for (const DataPoint& point : dataToUse) {
            timeData.append(point.timestamp.toSecsSinceEpoch());
            tempData.append(point.temperature);
            humData.append(point.humidity);
        }
        
        // Обновить данные
        customPlot->graph(0)->setData(timeData, tempData);
        customPlot->graph(1)->setData(timeData, humData);
        
        // Применить стили
        if (showPointsMode) {
            // Режим точек
            customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
            customPlot->graph(0)->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssCircle, vizSettings.temperatureColor, vizSettings.temperatureColor, vizSettings.pointSize));
            
            customPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
            customPlot->graph(1)->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssCircle, vizSettings.humidityColor, vizSettings.humidityColor, vizSettings.pointSize));
        } else {
            // Режим линий
            QPen tempPen(vizSettings.temperatureColor, vizSettings.temperatureLineWidth, Qt::SolidLine);
            customPlot->graph(0)->setPen(tempPen);
            customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
            customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
            
            QPen humPen(vizSettings.humidityColor, vizSettings.humidityLineWidth, Qt::SolidLine);
            customPlot->graph(1)->setPen(humPen);
            customPlot->graph(1)->setLineStyle(QCPGraph::lsLine);
            customPlot->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);
        }
    }
    
    // ИСПРАВИТЬ: восстановить зум после обновления
    customPlot->xAxis->setRange(xRange);
    customPlot->yAxis->setRange(yRange);
    customPlot->yAxis2->setRange(y2Range);
    
    updateTemperatureGrid();
    customPlot->replot();
    
    statusBar()->showMessage(
        QString("Display mode: %1").arg(showPointsMode ? "Points (raw data)" : "Lines (smoothed)"), 
        2000);
}

// ДОБАВИТЬ в конец файла все недостающие методы:

void MainWindow::setupTimeNavigation() {
    // Инициализация переменных навигации по времени
    currentTimeScale = Day;
    showPointsMode = false;
    showCursor = false;
}

void MainWindow::loadVisualizationSettings(QSettings& settings) {
    settings.beginGroup("VISUALIZATION");
    
    // Цвета
    vizSettings.temperatureColor = QColor(settings.value("TemperatureColor", "#FF0000").toString());
    vizSettings.humidityColor = QColor(settings.value("HumidityColor", "#0000FF").toString());
    vizSettings.gridColor = QColor(settings.value("GridColor", "#D3D3D3").toString());
    vizSettings.crosshairColor = QColor(settings.value("CrosshairColor", "#A9A9A9").toString());
    
    // Толщина линий
    vizSettings.temperatureLineWidth = settings.value("TemperatureLineWidth", 2).toInt();
    vizSettings.humidityLineWidth = settings.value("HumidityLineWidth", 2).toInt();
    vizSettings.gridLineWidth = settings.value("GridLineWidth", 1).toInt();
    vizSettings.crosshairLineWidth = settings.value("CrosshairLineWidth", 1).toInt();
    
    // Сглаживание
    vizSettings.smoothingWindow = settings.value("SmoothingWindow", 5).toInt();
    vizSettings.enableSmoothing = settings.value("EnableSmoothing", true).toBool();
    
    // Размеры и прозрачность
    vizSettings.pointSize = settings.value("PointSize", 4).toInt();
    vizSettings.gridOpacity = settings.value("GridOpacity", 255).toInt();
    vizSettings.crosshairOpacity = settings.value("CrosshairOpacity", 200).toInt();
    
    settings.endGroup();
    
    DEBUG_LOG("Loaded visualization settings from config");
}

void MainWindow::loadDefaultVisualizationSettings() {
    vizSettings.temperatureColor = Qt::red;
    vizSettings.humidityColor = Qt::blue;
    vizSettings.gridColor = Qt::lightGray;
    vizSettings.crosshairColor = Qt::darkGray;
    vizSettings.temperatureLineWidth = 2;
    vizSettings.humidityLineWidth = 2;
    vizSettings.gridLineWidth = 1;
    vizSettings.crosshairLineWidth = 1;
    vizSettings.smoothingWindow = 5;
    vizSettings.enableSmoothing = true;
    vizSettings.pointSize = 4;
    vizSettings.gridOpacity = 255;
    vizSettings.crosshairOpacity = 200;
    
    DEBUG_LOG("Loaded default visualization settings");
}

void MainWindow::applyVisualizationSettings() {
    if (!customPlot) return;
    
    // Применить цвета и стили к графикам
    if (customPlot->graphCount() >= 2) {
        // График температуры
        QPen tempPen(vizSettings.temperatureColor, vizSettings.temperatureLineWidth, Qt::SolidLine);
        customPlot->graph(0)->setPen(tempPen);
        
        // График влажности
        QPen humPen(vizSettings.humidityColor, vizSettings.humidityLineWidth, Qt::SolidLine);
        customPlot->graph(1)->setPen(humPen);
    }
    
    // Применить настройки сетки
    QPen gridPen(vizSettings.gridColor, vizSettings.gridLineWidth, Qt::DotLine);
    customPlot->xAxis->grid()->setPen(gridPen);
    customPlot->yAxis->grid()->setPen(gridPen);
    customPlot->yAxis2->grid()->setPen(gridPen);
    
    // Применить настройки курсора
    QColor crosshairColorWithAlpha = vizSettings.crosshairColor;
    crosshairColorWithAlpha.setAlpha(vizSettings.crosshairOpacity);
    QPen crosshairPen(crosshairColorWithAlpha, vizSettings.crosshairLineWidth, Qt::DashLine);
    
    if (crossHairV && crossHairH) {
        crossHairV->setPen(crosshairPen);
        crossHairH->setPen(crosshairPen);
    }
    
    DEBUG_LOG("Applied visualization settings");
}

void MainWindow::hideCrosshair() {
    if (crossHairV) crossHairV->setVisible(false);
    if (crossHairH) crossHairH->setVisible(false);
    if (valueLabel) valueLabel->setVisible(false);
    customPlot->replot();
}

void MainWindow::updateCrosshair(double x, double y) {
    if (!crossHairV || !crossHairH || !valueLabel) return;
    
    // Показать линии курсора
    crossHairV->start->setCoords(x, customPlot->yAxis->range().lower);
    crossHairV->end->setCoords(x, customPlot->yAxis->range().upper);
    crossHairH->start->setCoords(customPlot->xAxis->range().lower, y);
    crossHairH->end->setCoords(customPlot->xAxis->range().upper, y);
    
    crossHairV->setVisible(true);
    crossHairH->setVisible(true);
    
    // Показать значения в метке
    QString valueText = formatValueAtPosition(x);
    valueLabel->setText(valueText);
    valueLabel->position->setCoords(x, y);
    valueLabel->setVisible(true);
    
    customPlot->replot();
}

QString MainWindow::formatValueAtPosition(double timePos) {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(timePos);
    
    // Найти ближайшую точку данных
    if (originalData.isEmpty()) return "";
    
    DataPoint closestPoint = originalData.first();
    double minDistance = qAbs(closestPoint.timestamp.toSecsSinceEpoch() - timePos);
    
    for (const DataPoint& point : originalData) {
        double distance = qAbs(point.timestamp.toSecsSinceEpoch() - timePos);
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
        }
    }
    
    return QString("Time: %1\nTemp: %2°C\nHum: %3%")
        .arg(dt.toString("dd.MM.yyyy hh:mm:ss"))
        .arg(closestPoint.temperature, 0, 'f', 1)
        .arg(closestPoint.humidity, 0, 'f', 1);
}

void MainWindow::updateStatusStats() {
    if (originalData.isEmpty()) return;
    
    QCPRange xRange = customPlot->xAxis->range();
    QDateTime startTime = QDateTime::fromSecsSinceEpoch(xRange.lower);
    QDateTime endTime = QDateTime::fromSecsSinceEpoch(xRange.upper);
    
    // ДОБАВИТЬ: расчет статистики для видимого диапазона
    QVector<DataPoint> visibleData;
    
    // ИСПРАВИТЬ: использовать правильные данные в зависимости от системы
    const QVector<DataPoint>& dataToAnalyze = showPointsMode ? rawData : originalData;
    
    // Найти точки данных в видимом диапазоне
    for (const DataPoint& point : dataToAnalyze) {
        double pointTime = point.timestamp.toSecsSinceEpoch();
        if (pointTime >= xRange.lower && pointTime <= xRange.upper) {
            visibleData.append(point);
        }
    }
    
    QString statsText;
    
    if (visibleData.isEmpty()) {
        // Нет данных в видимом диапазоне
        statsText = QString("Viewing: %1 to %2 | No data in range")
            .arg(startTime.toString("dd.MM.yyyy hh:mm"))
            .arg(endTime.toString("dd.MM.yyyy hh:mm"));
    } else {
        // РАССЧИТАТЬ статистику для видимых данных
        double tempSum = 0, humSum = 0;
        double tempMin = visibleData.first().temperature;
        double tempMax = visibleData.first().temperature;
        double humMin = visibleData.first().humidity;
        double humMax = visibleData.first().humidity;
        
        for (const DataPoint& point : visibleData) {
            tempSum += point.temperature;
            humSum += point.humidity;
            
            tempMin = qMin(tempMin, point.temperature);
            tempMax = qMax(tempMax, point.temperature);
            humMin = qMin(humMin, point.humidity);
            humMax = qMax(humMax, point.humidity);
        }
        
        double tempAvg = tempSum / visibleData.size();
        double humAvg = humSum / visibleData.size();
        
        // ФОРМАТ: Время | Количество точек | Средние значения | Диапазоны
        statsText = QString("Viewing: %1 to %2 | Points: %3 | Temp: %4°C (avg) [%5°C...%6°C] | Hum: %7% (avg) [%8%...%9%]")
            .arg(startTime.toString("dd.MM.yyyy hh:mm"))
            .arg(endTime.toString("dd.MM.yyyy hh:mm"))
            .arg(visibleData.size())
            .arg(tempAvg, 0, 'f', 1)
            .arg(tempMin, 0, 'f', 1)
            .arg(tempMax, 0, 'f', 1)
            .arg(humAvg, 0, 'f', 1)
            .arg(humMin, 0, 'f', 1)
            .arg(humMax, 0, 'f', 1);
    }
    
    statusBar()->showMessage(statsText, 0); // 0 = показывать постоянно
}

void MainWindow::setTimeScale(TimeScale scale) {
    currentTimeScale = scale;
    
    QString scaleText;
    switch (scale) {
    case Day: scaleText = "Day scale"; break;
    case Week: scaleText = "Week scale"; break;
    case Month: scaleText = "Month scale"; break;
    }
    
    statusBar()->showMessage(scaleText, 2000);
    
    // ДОБАВИТЬ: автоматически установить подходящий масштаб времени
    if (!originalData.isEmpty()) {
        autoAdjustTimeScale();
    }
}

// ДОБАВИТЬ новый метод для автоматической настройки масштаба времени:
void MainWindow::autoAdjustTimeScale() {
    if (originalData.isEmpty()) return;
    
    // Получить текущий центр экрана
    QCPRange currentRange = customPlot->xAxis->range();
    double centerTime = (currentRange.lower + currentRange.upper) / 2.0;
    
    // Определить новый размер окна в зависимости от выбранного масштаба
    double newRangeSize;
    switch (currentTimeScale) {
    case Day:
        newRangeSize = 86400; // 24 часа
        break;
    case Week:
        newRangeSize = 604800; // 7 дней
        break;
    case Month:
        newRangeSize = 2592000; // 30 дней
        break;
    }
    
    // Установить новый диапазон с центром в текущей позиции
    double newStart = centerTime - newRangeSize / 2.0;
    double newEnd = centerTime + newRangeSize / 2.0;
    
    // Проверить границы данных
    double dataStart = originalData.first().timestamp.toSecsSinceEpoch();
    double dataEnd = originalData.last().timestamp.toSecsSinceEpoch();
    
    // Скорректировать если выходим за границы
    if (newStart < dataStart) {
        newStart = dataStart;
        newEnd = qMin(dataStart + newRangeSize, dataEnd);
    }
    if (newEnd > dataEnd) {
        newEnd = dataEnd;
        newStart = qMax(dataEnd - newRangeSize, dataStart);
    }
    
    // Применить новый диапазон
    customPlot->xAxis->setRange(newStart, newEnd);
    
    // Обновить шкалу времени
    updateTimeScale();
    
    customPlot->replot();
    updateStatusStats();
}

void MainWindow::navigateTime(int direction) {
    if (originalData.isEmpty()) return;
    
    QCPRange range = customPlot->xAxis->range();
    double step;
    
    switch (currentTimeScale) {
    case Day: step = 3600; break;      // 1 час
    case Week: step = 86400; break;    // 1 день  
    case Month: step = 604800; break;  // 1 неделя
    }
    
    step *= direction;
    customPlot->xAxis->setRange(range.lower + step, range.upper + step);
    customPlot->replot();
    updateStatusStats();
}

// Методы для множественных графиков
void MainWindow::addFileToChart() {
    QString lastDir = getLastDirectory();
    QString filePath = QFileDialog::getOpenFileName(this,
        "Add CSV File to Chart", lastDir,
        "CSV files (*.csv);;All files (*.*)");
        
    if (filePath.isEmpty()) return;
    
    setLastDirectory(QFileInfo(filePath).absolutePath());
    
    // ИСПРАВИТЬ: если есть данные в старой системе, перенести их в новую
    if (dataSets.isEmpty() && !originalData.isEmpty()) {
        // Первое добавление - нужно перенести существующий файл в dataSets
        QString existingFileName = currentFilePath.isEmpty() ? "Existing data" : QFileInfo(currentFilePath).fileName();
        addDataSet(originalData, rawData, existingFileName);
        
        // Переключиться в режим наложения
        overlayMode = true;
        
        DEBUG_LOG("Moved existing data to dataSets, now adding new file");
    }
    
    // Загрузить данные в режиме наложения
    overlayMode = true;
    loadFileData(filePath, true);
}

void MainWindow::clearAllGraphs() {
    // Очистить новую систему
    dataSets.clear();
    currentDataSetIndex = -1;
    overlayMode = false;
    
    // Очистить старые переменные
    originalData.clear();
    rawData.clear();
    
    // ИСПРАВИТЬ: полностью очистить и восстановить графики
    customPlot->clearGraphs();
    customPlot->clearItems(); // Очистить все элементы включая линии сетки
    
    // Восстановить базовые графики как в setupUI()
    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
    customPlot->graph(0)->setName("Temperature");
    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
    customPlot->graph(1)->setName("Humidity");
    
    // Восстановить курсор
    crossHairV = new QCPItemLine(customPlot);
    crossHairH = new QCPItemLine(customPlot);
    valueLabel = new QCPItemText(customPlot);
    
    valueLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
    valueLabel->setPadding(QMargins(5, 5, 5, 5));
    valueLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
    valueLabel->setPen(QPen(Qt::black));
    
    hideCrosshair();
    
    // Применить настройки визуализации
    applyVisualizationSettings();
    
    // Сбросить режимы
    showPointsMode = false;
    showCursor = false;
    
    customPlot->replot();
    
    setWindowTitle("HT View - Temperature/Humidity Plotter");
    statusBar()->showMessage("All datasets cleared", 2000);
}

void MainWindow::loadFileData(const QString& filePath, bool addToExisting) {
    // ИСПРАВИТЬ: сохранить зум только при добавлении к существующим данным
    QCPRange savedXRange, savedYRange, savedY2Range;
    bool preserveZoom = addToExisting && (!dataSets.isEmpty() || !originalData.isEmpty());
    
    if (preserveZoom) {
        savedXRange = customPlot->xAxis->range();
        savedYRange = customPlot->yAxis->range();
        savedY2Range = customPlot->yAxis2->range();
    }
    
    // ИСПРАВИТЬ: НЕ очищать при добавлении
    if (!addToExisting) {
        clearAllGraphs();
        currentFilePath = filePath;
        updateWindowTitle(filePath);
    }
    
    progressDialog = new QProgressDialog(
        addToExisting ? "Adding CSV file to chart..." : "Loading CSV file...", 
        "Cancel", 0, 100, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    progressDialog->setValue(10);
    
    QTimer::singleShot(50, [this, filePath, addToExisting, preserveZoom, savedXRange, savedYRange, savedY2Range]() {
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
        
        // Сохранить исходные данные
        QVector<DataPoint> rawDataSet = data;
        
        progressDialog->setLabelText("Smoothing data...");
        
        // Применить сглаживание
        QVector<DataPoint> smoothedData;
        if (vizSettings.enableSmoothing) {
            smoothedData = smoothData(data, vizSettings.smoothingWindow);
        } else {
            smoothedData = data;
        }
        
        if (addToExisting) {
            // ИСПРАВИТЬ: добавляем к существующим данным через новую систему
            addDataSet(smoothedData, rawDataSet, QFileInfo(filePath).fileName());
            
            progressDialog->setValue(90);
            progressDialog->setLabelText("Creating plot...");
            QCoreApplication::processEvents();
            
            // Обновить через новую систему множественных графиков
            updateAllGraphs();
        } else {
            // ИСПРАВИТЬ: загрузка одного файла через старую систему
            this->originalData = smoothedData;
            this->rawData = rawDataSet;
            
            progressDialog->setValue(90);
            progressDialog->setLabelText("Creating plot...");
            QCoreApplication::processEvents();
            
            // Использовать старую систему отображения
            plotData(smoothedData);
        }
        
        // ДОБАВИТЬ: восстановить зум если нужно
        if (preserveZoom) {
            customPlot->xAxis->setRange(savedXRange);
            customPlot->yAxis->setRange(savedYRange);
            customPlot->yAxis2->setRange(savedY2Range);
            updateTemperatureGrid();
            customPlot->replot();
        }
        
        progressDialog->setValue(100);
        progressDialog->close();
        progressDialog = nullptr;
        
        // Обновить заголовок окна
        if (addToExisting && dataSets.size() > 1) {
            setWindowTitle(QString("HT View - %1 datasets").arg(dataSets.size()));
        } else if (!addToExisting) {
            updateWindowTitle(filePath);
        }
    });
}

void MainWindow::addDataSet(const QVector<DataPoint>& originalData, 
                           const QVector<DataPoint>& rawData, 
                           const QString& fileName) {
    DataSet newDataSet;
    newDataSet.originalData = originalData;
    newDataSet.rawData = rawData;
    newDataSet.fileName = fileName;
    
    // Генерировать цвета для наложения
    generateOverlayColors(dataSets.size(), newDataSet.tempColor, newDataSet.humColor);
    
    dataSets.append(newDataSet);
    currentDataSetIndex = dataSets.size() - 1;
    
    // Для совместимости со старым кодом
    if (dataSets.size() == 1) {
        this->originalData = originalData;
        this->rawData = rawData;
    }
}

void MainWindow::generateOverlayColors(int index, QColor& tempColor, QColor& humColor) {
    if (index == 0) {
        // Первый набор - стандартные цвета (полная насыщенность)
        tempColor = vizSettings.temperatureColor;
        humColor = vizSettings.humidityColor;
    } else {
        // ИСПРАВИТЬ: использовать базовые цвета с уменьшением насыщенности
        QColor baseTempColor = vizSettings.temperatureColor;
        QColor baseHumColor = vizSettings.humidityColor;
        
        // Рассчитать коэффициент уменьшения насыщенности
        // index 1 = 70%, index 2 = 50%, index 3 = 30%, и т.д.
        double saturationFactor = qMax(0.3, 1.0 - (index * 0.2));
        int alphaValue = qMax(100, static_cast<int>(255 * saturationFactor));
        
        // Температура - сохранить оттенок, уменьшить насыщенность
        tempColor = baseTempColor;
        tempColor.setAlpha(alphaValue);
        
        // Влажность - сохранить оттенок, уменьшить насыщенность  
        humColor = baseHumColor;
        humColor.setAlpha(alphaValue);
        
        // Дополнительно: слегка изменить оттенок для четкого различия
        if (index > 1) {
            int hueShift = (index - 1) * 15; // сдвиг на 15° для каждого последующего
            
            int tempHue = (tempColor.hue() + hueShift) % 360;
            tempColor.setHsv(tempHue, tempColor.saturation(), tempColor.value(), alphaValue);
            
            int humHue = (humColor.hue() + hueShift) % 360;
            humColor.setHsv(humHue, humColor.saturation(), humColor.value(), alphaValue);
        }
    }
}

void MainWindow::updateAllGraphs() {
    if (dataSets.isEmpty()) return;
    
    // ИСПРАВИТЬ: правильная проверка существующего диапазона
    bool hasExistingRange = customPlot->graphCount() > 0 && !originalData.isEmpty();
    QCPRange savedXRange, savedYRange, savedY2Range;
    if (hasExistingRange) {
        savedXRange = customPlot->xAxis->range();
        savedYRange = customPlot->yAxis->range();
        savedY2Range = customPlot->yAxis2->range();
    }
    
    // Очистить существующие графики
    customPlot->clearGraphs();
    
    // Создать графики для каждого набора данных
    for (int i = 0; i < dataSets.size(); ++i) {
        const DataSet& dataSet = dataSets[i];
        if (!dataSet.visible) continue;
        
        // Выбрать данные в зависимости от режима отображения
        const QVector<DataPoint>& dataToUse = showPointsMode ? dataSet.rawData : dataSet.originalData;
        
        if (dataToUse.isEmpty()) continue;
        
        // Подготовить данные для графиков
        QVector<double> timeData, tempData, humData;
        for (const DataPoint& point : dataToUse) {
            timeData.append(point.timestamp.toSecsSinceEpoch());
            tempData.append(point.temperature);
            humData.append(point.humidity);
        }
        
        // Создать график температуры
        customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
        int tempGraphIndex = customPlot->graphCount() - 1;
        customPlot->graph(tempGraphIndex)->setData(timeData, tempData);
        customPlot->graph(tempGraphIndex)->setName(QString("Temperature (%1)").arg(dataSet.fileName));
        
        // ДОБАВИТЬ: настроить декоратор выделения для температуры
        QCPSelectionDecorator *tempDecorator = new QCPSelectionDecorator;
        tempDecorator->setPen(QPen(Qt::yellow, 4));
        tempDecorator->setBrush(QBrush(Qt::yellow, Qt::Dense6Pattern));
        customPlot->graph(tempGraphIndex)->setSelectionDecorator(tempDecorator);
        
        // Создать график влажности  
        customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
        int humGraphIndex = customPlot->graphCount() - 1;
        customPlot->graph(humGraphIndex)->setData(timeData, humData);
        customPlot->graph(humGraphIndex)->setName(QString("Humidity (%1)").arg(dataSet.fileName));
        
        // ДОБАВИТЬ: настроить декоратор выделения для влажности
        QCPSelectionDecorator *humDecorator = new QCPSelectionDecorator;
        humDecorator->setPen(QPen(Qt::cyan, 4));
        humDecorator->setBrush(QBrush(Qt::cyan, Qt::Dense6Pattern));
        customPlot->graph(humGraphIndex)->setSelectionDecorator(humDecorator);
        
        // Применить стили
        if (showPointsMode) {
            // Режим точек
            customPlot->graph(tempGraphIndex)->setLineStyle(QCPGraph::lsNone);
            customPlot->graph(tempGraphIndex)->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssCircle, dataSet.tempColor, dataSet.tempColor, vizSettings.pointSize));
                
            customPlot->graph(humGraphIndex)->setLineStyle(QCPGraph::lsNone);
            customPlot->graph(humGraphIndex)->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssCircle, dataSet.humColor, dataSet.humColor, vizSettings.pointSize));
        } else {
            // Режим линий
            QPen tempPen(dataSet.tempColor, vizSettings.temperatureLineWidth, Qt::SolidLine);
            customPlot->graph(tempGraphIndex)->setPen(tempPen);
            customPlot->graph(tempGraphIndex)->setLineStyle(QCPGraph::lsLine);
            
            QPen humPen(dataSet.humColor, vizSettings.humidityLineWidth, Qt::SolidLine);
            customPlot->graph(humGraphIndex)->setPen(humPen);
            customPlot->graph(humGraphIndex)->setLineStyle(QCPGraph::lsLine);
        }
    }
    
    // ИЗМЕНИТЬ: восстановить зум или автомасштабировать
    if (hasExistingRange) {
        // Восстановить сохраненные диапазоны
        customPlot->xAxis->setRange(savedXRange);
        customPlot->yAxis->setRange(savedYRange);
        customPlot->yAxis2->setRange(savedY2Range);
    } else {
        // Автомасштабирование только для первой загрузки
        customPlot->rescaleAxes();
    }
    
    updateTemperatureGrid();
    customPlot->replot();
    
    // ДОБАВИТЬ: обновить статистику для множественных графиков
    if (dataSets.size() > 1) {
        // Для множественных графиков показать краткую статистику
        QCPRange xRange = customPlot->xAxis->range();
        QDateTime startTime = QDateTime::fromSecsSinceEpoch(xRange.lower);
        QDateTime endTime = QDateTime::fromSecsSinceEpoch(xRange.upper);
        
        QString statsText = QString("Viewing: %1 to %2 | %3 datasets loaded")
            .arg(startTime.toString("dd.MM.yyyy hh:mm"))
            .arg(endTime.toString("dd.MM.yyyy hh:mm"))
            .arg(dataSets.size());
        
        statusBar()->showMessage(statsText, 0);
    } else {
        // Для одного графика использовать детальную статистику
        updateStatusStats();
    }
}

void MainWindow::onMouseMove(QMouseEvent* event) {
    if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        double x = customPlot->xAxis->pixelToCoord(event->pos().x());
        double y = customPlot->yAxis->pixelToCoord(event->pos().y());
        updateCrosshair(x, y);
        showCursor = true;
    } else if (showCursor) {
        hideCrosshair();
        showCursor = false;
    }
}

void MainWindow::onXAxisRangeChanged(const QCPRange &newRange) {
    updateTemperatureGrid();
    updateTimeScale(); // ДОБАВИТЬ: обновлять шкалу времени при изменении диапазона
    updateStatusStats();
}

// ДОБАВИТЬ новый метод для обновления шкалы времени:
void MainWindow::updateTimeScale() {
    if (originalData.isEmpty()) return;
    
    QCPRange xRange = customPlot->xAxis->range();
    double timeRange = xRange.upper - xRange.lower;
    
    QSharedPointer<QCPAxisTickerText> timeTicker(new QCPAxisTickerText);
    
    int timeStepSeconds;
    
    // Автоматически выбрать интервал в зависимости от масштаба
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
    QDateTime startDT = QDateTime::fromSecsSinceEpoch(xRange.lower);
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
         alignedTime <= xRange.upper; 
         alignedTime += timeStepSeconds) {
        
        if (alignedTime < xRange.lower) continue;
        
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
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == customPlot) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            
            // Навигация по времени
            switch (keyEvent->key()) {
            case Qt::Key_Left:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    panHorizontal(-1);  // Плавный сдвиг влево
                } else {
                    navigateTime(-1);   // Обычная навигация
                }
                return true;
                
            case Qt::Key_Right:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    panHorizontal(1);   // Плавный сдвиг вправо
                } else {
                    navigateTime(1);    // Обычная навигация
                }
                return true;
                
            case Qt::Key_Up:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    zoomVertical(0.8);  // Увеличить масштаб
                } else {
                    panVertical(1);     // Сдвинуть вверх
                }
                return true;
                
            case Qt::Key_Down:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    zoomVertical(1.25); // Уменьшить масштаб
                } else {
                    panVertical(-1);    // Сдвинуть вниз
                }
                return true;
                
            case Qt::Key_PageUp:
                jumpToPeriodBoundary(-1);
                return true;
                
            case Qt::Key_PageDown:
                jumpToPeriodBoundary(1);
                return true;
                
            case Qt::Key_Home:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    resetAllZoom();
                } else {
                    goToStart();
                }
                return true;
                
            case Qt::Key_End:
                goToEnd();
                return true;
                
            case Qt::Key_D:
                setTimeScale(Day);
                return true;
                
            case Qt::Key_W:
                setTimeScale(Week);
                return true;
                
            case Qt::Key_M:
                setTimeScale(Month);
                return true;
                
            case Qt::Key_T:
                toggleTemperatureGraph();
                return true;
                
            case Qt::Key_H:
                toggleHumidityGraph();
                return true;
                
            case Qt::Key_P:
                toggleDisplayMode();
                return true;
                
            // ДОБАВИТЬ: удаление выделенных графиков
            case Qt::Key_Delete:
                deleteSelectedGraphs();
                return true;
            }
        }
        
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            
            if (wheelEvent->modifiers() & Qt::ShiftModifier) {
                // Shift + колесо = вертикальный зум
                double factor = wheelEvent->angleDelta().y() > 0 ? 0.9 : 1.1;
                zoomVertical(factor);
                return true;
            } else {
                // Обычное колесо = горизонтальный зум (по времени)
                QCPRange range = customPlot->xAxis->range();
                double center = (range.lower + range.upper) / 2.0;
                double factor = wheelEvent->angleDelta().y() > 0 ? 0.9 : 1.1;
                double newSize = range.size() * factor;
                customPlot->xAxis->setRange(center - newSize/2, center + newSize/2);
                
                // ДОБАВИТЬ: обновить шкалу времени после зума
                updateTimeScale();
                
                customPlot->replot();
                updateStatusStats();
                return true;
            }
        }
        
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Shift) {
                if (showCursor) {
                    hideCrosshair();
                    showCursor = false;
                }
                return true;
            }
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::deleteSelectedGraphs() {
    if (!dataSets.isEmpty()) {
        // Новая система множественных графиков
        deleteSelectedFromDataSets();
    } else if (!originalData.isEmpty()) {
        // Старая система одного файла
        deleteSelectedFromSingleFile();
    }
}

void MainWindow::deleteSelectedFromDataSets() {
    if (dataSets.isEmpty()) return;
    
    QList<int> graphsToDelete;
    QStringList deletedFileNames;
    
    // Найти выделенные графики
    for (int i = 0; i < customPlot->graphCount(); ++i) {
        if (customPlot->graph(i)->selected()) {
            graphsToDelete.append(i);
        }
    }
    
    if (graphsToDelete.isEmpty()) {
        statusBar()->showMessage("No graphs selected for deletion", 2000);
        return;
    }
    
    // Определить какие наборы данных нужно удалить
    QSet<int> dataSetsToDelete;
    for (int graphIndex : graphsToDelete) {
        // Каждый набор данных создает 2 графика (temp + humidity)
        int dataSetIndex = graphIndex / 2;
        if (dataSetIndex < dataSets.size()) {
            dataSetsToDelete.insert(dataSetIndex);
            deletedFileNames.append(dataSets[dataSetIndex].fileName);
        }
    }
    
    // Подтверждение удаления
    if (dataSetsToDelete.size() > 1) {
        int ret = QMessageBox::question(this, "Delete Datasets",
            QString("Delete %1 selected datasets?\n\nFiles:\n%2")
                .arg(dataSetsToDelete.size())
                .arg(deletedFileNames.join("\n")),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
    } else if (dataSetsToDelete.size() == 1) {
        int ret = QMessageBox::question(this, "Delete Dataset",
            QString("Delete dataset: %1?").arg(deletedFileNames.first()),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
    }
    
    // Сохранить зум
    QCPRange savedXRange = customPlot->xAxis->range();
    QCPRange savedYRange = customPlot->yAxis->range();
    QCPRange savedY2Range = customPlot->yAxis2->range();
    
    // Удалить наборы данных (в обратном порядке чтобы индексы не сбивались)
    QList<int> sortedIndexes = dataSetsToDelete.values();
    std::sort(sortedIndexes.rbegin(), sortedIndexes.rend());
    
    for (int index : sortedIndexes) {
        dataSets.removeAt(index);
    }
    
    // Проверить что осталось
    if (dataSets.isEmpty()) {
        // Все удалили - вернуться к одиночному режиму
        overlayMode = false;
        originalData.clear();
        rawData.clear();
        currentDataSetIndex = -1;
        
        customPlot->clearGraphs();
        
        // Восстановить базовые графики
        customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
        customPlot->graph(0)->setName("Temperature");
        customPlot->addGraph(customPlot->xAxis, customPlot->yAxis2);
        customPlot->graph(1)->setName("Humidity");
        
        // Восстановить курсор
        crossHairV = new QCPItemLine(customPlot);
        crossHairH = new QCPItemLine(customPlot);
        valueLabel = new QCPItemText(customPlot);
        
        valueLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
        valueLabel->setPadding(QMargins(5, 5, 5, 5));
        valueLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
        valueLabel->setPen(QPen(Qt::black));
        
        hideCrosshair();
        
        // Применить настройки визуализации
        applyVisualizationSettings();
        
        // Сбросить режимы
        showPointsMode = false;
        showCursor = false;
        
        customPlot->replot();
        
        setWindowTitle("HT View - Temperature/Humidity Plotter");
        statusBar()->showMessage("All datasets deleted", 2000);
    } else {
        // Еще есть данные - обновить все графики
        if (dataSets.size() == 1) {
            // Остался один набор - можно вернуться к одиночному режиму
            originalData = dataSets[0].originalData;
            rawData = dataSets[0].rawData;
            currentFilePath = dataSets[0].fileName;
            
            overlayMode = false;
            currentDataSetIndex = -1;
            
            updateWindowTitle(currentFilePath);
        }
        
        updateAllGraphs();
        
        // Восстановить зум
        customPlot->xAxis->setRange(savedXRange);
        customPlot->yAxis->setRange(savedYRange);
        customPlot->yAxis2->setRange(savedY2Range);
        
        updateTemperatureGrid();
        customPlot->replot();
        
        // Обновить заголовок
        if (dataSets.size() > 1) {
            setWindowTitle(QString("HT View - %1 datasets").arg(dataSets.size()));
        }
        
        statusBar()->showMessage(
            QString("Deleted %1 dataset(s), %2 remaining")
                .arg(deletedFileNames.size())
                .arg(dataSets.size()), 3000);
    }
}

void MainWindow::deleteSelectedFromSingleFile() {
    // В режиме одного файла можем только скрыть графики
    bool tempSelected = false;
    bool humSelected = false;
    
    if (customPlot->graphCount() >= 2) {
        tempSelected = customPlot->graph(0)->selected();
        humSelected = customPlot->graph(1)->selected();
    }
    
    if (!tempSelected && !humSelected) {
        statusBar()->showMessage("No graphs selected for deletion", 2000);
        return;
    }
    
    QStringList toHide;
    if (tempSelected) toHide << "Temperature";
    if (humSelected) toHide << "Humidity";
    
    int ret = QMessageBox::question(this, "Hide Graphs",
        QString("Hide selected graphs: %1?\n\n(Use T/H keys to show them again)")
            .arg(toHide.join(", ")),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (tempSelected && customPlot->graphCount() >= 1) {
            customPlot->graph(0)->setVisible(false);
        }
        if (humSelected && customPlot->graphCount() >= 2) {
            customPlot->graph(1)->setVisible(false);
        }
        
        customPlot->replot();
        statusBar()->showMessage("Selected graphs hidden (use T/H to show)", 3000);
    }
}

// ИСПРАВИТЬ: методы печати - заменить строки с toRect()
void MainWindow::printChart() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Проверить режим черно-белой печати
        bool blackWhite = blackWhitePrintAction ? blackWhitePrintAction->isChecked() : false;
        
        if (blackWhite) {
            printInBlackAndWhite(&printer);
        } else {
            // ИСПРАВИТЬ: убрать toRect() - QRect уже является QRect
            QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
            QPainter painter(&printer);
            QRect printRect = printer.pageLayout().paintRectPixels(printer.resolution());
            painter.drawPixmap(printRect, pixmap);
        }
    }
}

void MainWindow::printPreview() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [this](QPrinter *printer) {
        // Проверить режим черно-белой печати
        bool blackWhite = blackWhitePrintAction ? blackWhitePrintAction->isChecked() : false;
        
        if (blackWhite) {
            printInBlackAndWhite(printer);
        } else {
            // ИСПРАВИТЬ: убрать toRect() - QRect уже является QRect
            QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
            QPainter painter(printer);
            QRect printRect = printer->pageLayout().paintRectPixels(printer->resolution());
            painter.drawPixmap(printRect, pixmap);
        }
    });
    
    preview.exec();
}

void MainWindow::printInBlackAndWhite(QPrinter *printer) {
    // Сохранить оригинальные цвета
    QMap<QCPGraph*, QPen> originalPens;
    QMap<QCPGraph*, QCPScatterStyle> originalScatterStyles;
    
    for (int i = 0; i < customPlot->graphCount(); ++i) {
        QCPGraph* graph = customPlot->graph(i);
        originalPens[graph] = graph->pen();
        originalScatterStyles[graph] = graph->scatterStyle();
        
        // Установить черно-белые стили
        QPen bwPen = graph->pen();
        if (i % 2 == 0) { // Температура - сплошная линия
            bwPen.setColor(Qt::black);
            bwPen.setStyle(Qt::SolidLine);
        } else { // Влажность - пунктирная линия
            bwPen.setColor(Qt::black);
            bwPen.setStyle(Qt::DashLine);
        }
        graph->setPen(bwPen);
        
        // Черно-белые точки
        QCPScatterStyle bwScatter = graph->scatterStyle();
        bwScatter.setPen(QPen(Qt::black));
        bwScatter.setBrush(QBrush(Qt::white));
        graph->setScatterStyle(bwScatter);
    }
    
    // ИСПРАВИТЬ: убрать toRect() - QRect уже является QRect
    customPlot->replot();
    QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
    QPainter painter(printer);
    QRect printRect = printer->pageLayout().paintRectPixels(printer->resolution());
    painter.drawPixmap(printRect, pixmap);
    
    // Восстановить оригинальные цвета
    for (auto it = originalPens.begin(); it != originalPens.end(); ++it) {
        it.key()->setPen(it.value());
    }
    for (auto it = originalScatterStyles.begin(); it != originalScatterStyles.end(); ++it) {
        it.key()->setScatterStyle(it.value());
    }
    customPlot->replot();
}

void MainWindow::copyChartToClipboard() {
    QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
    QApplication::clipboard()->setPixmap(pixmap);
    statusBar()->showMessage("Chart copied to clipboard", 2000);
}

void MainWindow::exportChart() {
    QString lastDir = getLastDirectory();
    QString filePath = QFileDialog::getSaveFileName(this,
        "Export Chart", lastDir + "/chart.png",
        "PNG Images (*.png);;JPEG Images (*.jpg);;PDF Files (*.pdf);;All Files (*)");
    
    if (filePath.isEmpty()) return;
    
    setLastDirectory(QFileInfo(filePath).absolutePath());
    
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    bool success = false;
    if (suffix == "pdf") {
        // ИСПРАВИТЬ: использовать savePdf для PDF
        success = customPlot->savePdf(filePath, 1920, 1080);
    } else {
        // Для PNG/JPEG экспорт с высоким качеством
        QPixmap pixmap = customPlot->toPixmap(1920, 1080, 2.0);
        success = pixmap.save(filePath);
    }
    
    if (success) {
        statusBar()->showMessage(QString("Chart exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, "Export Error", "Failed to export chart");
    }
}

