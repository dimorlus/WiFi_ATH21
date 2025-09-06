#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "csvparser.h"
#include "csvworker.h"
#include <QProgressDialog>
#include <QSettings>
#include <QAction>
#include <QTimer>
#include <QThread>

// Forward declarations
class QCPItemLine;
class QCPItemText;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void openFile();
    void openRecentFile();
    void onParsingProgress(int percentage);
    void onParsingStatus(const QString& status);
    void onParsingFinished(const QVector<DataPoint>& data);
    void onParsingError(const QString& message);
    void onMouseMove(QMouseEvent* event);
    void onXAxisRangeChanged(const QCPRange &newRange);
    
    // ДОБАВИТЬ: новые слоты для множественных графиков
    void addFileToChart();
    void clearAllGraphs();
    
    // Печать и экспорт
    void printChart();
    void printPreview();
    void copyChartToClipboard();
    void exportChart();

private:
    // Основные компоненты
    QCustomPlot *customPlot;
    QProgressDialog *progressDialog;
    QSettings *settings;
    QThread *workerThread;
    CSVWorker *csvWorker;
    
    // ДОБАВИТЬ: поддержка множественных графиков
    struct DataSet {
        QVector<DataPoint> originalData;
        QVector<DataPoint> rawData;
        QString fileName;
        QColor tempColor;
        QColor humColor;
        bool visible = true;
    };
    
    QVector<DataSet> dataSets;           // все загруженные наборы данных
    int currentDataSetIndex = -1;        // индекс текущего активного набора
    bool overlayMode = false;            // режим наложения графиков
    
    // Данные (для совместимости со старым кодом)
    QVector<DataPoint> originalData;
    QVector<DataPoint> rawData;
    QString currentFilePath;
    
    // Режимы отображения  
    bool showPointsMode = false;
    bool showCursor;
    
    // Элементы курсора
    QCPItemLine *crossHairV;
    QCPItemLine *crossHairH;
    QCPItemText *valueLabel;
    
    // Навигация
    enum TimeScale { Day, Week, Month };
    TimeScale currentTimeScale;
    
    // Recent files
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *recentFilesSeparator;
    QMenu *recentFilesMenu;
    
    // Печать
    QAction *blackWhitePrintAction;
    
    // Настройки визуализации
    struct VisualizationSettings {
        QColor temperatureColor = Qt::red;
        QColor humidityColor = Qt::blue;
        QColor gridColor = Qt::lightGray;
        QColor crosshairColor = Qt::darkGray;
        int temperatureLineWidth = 2;
        int humidityLineWidth = 2;
        int gridLineWidth = 1;
        int crosshairLineWidth = 1;
        int smoothingWindow = 5;
        bool enableSmoothing = true;
        int pointSize = 4;
        int gridOpacity = 255;
        int crosshairOpacity = 200;
    } vizSettings;
    
    // Методы настройки
    void setupUI();
    void setupMenus();
    void loadSettings();
    void saveSettings();
    void setupTimeNavigation();
    void loadVisualizationSettings(QSettings& settings);
    void loadDefaultVisualizationSettings();
    void applyVisualizationSettings();
    
    // Управление файлами
    QString getLastDirectory();
    void setLastDirectory(const QString& dirPath);
    void addToRecentFiles(const QString& filePath);
    void updateRecentFilesMenu();
    QString shortenPath(const QString& fullPath, int maxLength = 80);
    void updateWindowTitle(const QString& filePath = QString());
    
    // ДОБАВИТЬ: методы для работы с множественными графиками
    void loadFileData(const QString& filePath, bool addToExisting = false);
    void addDataSet(const QVector<DataPoint>& originalData, 
                   const QVector<DataPoint>& rawData, 
                   const QString& fileName);
    void updateAllGraphs();
    void generateOverlayColors(int index, QColor& tempColor, QColor& humColor);
    void deleteSelectedGraphs(); // ДОБАВИТЬ новый метод
    void deleteSelectedFromDataSets();
    void deleteSelectedFromSingleFile();

    // Парсинг и отображение
    QString loadParserPattern();
    void plotData(const QVector<DataPoint>& data);
    QVector<DataPoint> smoothData(const QVector<DataPoint>& data, int windowSize);
    void optimizePlotting();
    void setupAxes();
    
    // Навигация и масштабирование
    void setTimeScale(TimeScale scale);
    void navigateTime(int direction);
    void zoomVertical(double factor);
    void panVertical(int direction);
    void panHorizontal(int direction);
    void resetAllZoom();
    void goToStart();
    void goToEnd();
    void jumpToPeriodBoundary(int direction);
    
    // Отображение графиков
    void toggleTemperatureGraph();
    void toggleHumidityGraph();
    void toggleDisplayMode();
    
    // Курсор и статистика
    void updateCrosshair(double x, double y);
    void hideCrosshair();
    QString formatValueAtPosition(double timePos);
    QString getVisibleRangeStats();
    void updateStatusStats();
    void updateTemperatureGrid();
    
    // Печать и экспорт
    void renderChartToPrinter(QPrinter *printer, bool useColor);
    void printInBlackAndWhite(QPrinter *printer); // ДОБАВИТЬ
    
    void autoAdjustTimeScale(); // ДОБАВИТЬ
    void updateTimeScale(); // ДОБАВИТЬ
};

#endif // MAINWINDOW_H
