#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QTimer>
#include <QThread>
#include <QSettings>
#include <QAction>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDateTime>
#include <QString>
#include <QVector>
#include <limits>

// Включаем определение DataPoint из csvparser.h
#include "csvparser.h"

// Forward declarations для QCustomPlot
class QCustomPlot;
class QCPItemLine;
class QCPItemText;
class QCPRange;

// Forward declarations для других классов
class CSVWorker;
class QPrinter;  // ДОБАВИТЬ forward declaration

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
    
    // Новые слоты для навигации
    void onMouseMove(QMouseEvent* event);
    void onXAxisRangeChanged(const QCPRange &newRange);
    
    // ОБНОВЛЕННЫЕ СЛОТЫ для печати и копирования
    void printChart();          // Прямая печать
    void printPreview();        // Предпросмотр печати
    void copyChartToClipboard();
    void exportChart();
    
    // НОВЫЕ МЕТОДЫ для навигации и управления
    void zoomVertical(double factor);
    void panVertical(int direction);
    void panHorizontal(int direction);  // ДОБАВИТЬ
    void resetAllZoom();
    void goToStart();
    void goToEnd();
    void jumpToPeriodBoundary(int direction);
    void toggleTemperatureGraph();
    void toggleHumidityGraph();
    void updateTemperatureGrid(); // ДОБАВИТЬ новый метод

private:
    // Существующие члены
    QCustomPlot *customPlot;
    QThread *workerThread;
    CSVWorker *csvWorker;
    QProgressDialog *progressDialog;
    
    QSettings *settings;
    QString currentFilePath;
    
    static const int MaxRecentFiles = 10;
    QAction *recentFileActions[MaxRecentFiles];
    QAction *recentFilesSeparator;
    QMenu *recentFilesMenu;
    
    // Новые члены для навигации
    enum TimeScale { Day, Week, Month };
    TimeScale currentTimeScale;
    QVector<DataPoint> originalData;
    QVector<DataPoint> rawData;      // ДОБАВИТЬ: исходные несглаженные данные
    QCPItemLine *crossHairV; // Вертикальная линия курсора
    QCPItemLine *crossHairH; // Горизонтальная линия курсора
    QCPItemText *valueLabel; // Подпись с значениями
    bool showCursor;
    
    // Состояние печати
    QAction *blackWhitePrintAction;  // ДОБАВИТЬ
    
    // Режимы отображения  
    bool showPointsMode = false;     // ДОБАВИТЬ: false = линии, true = точки
    
    // Методы
    void setupUI();
    void setupMenus();
    void setupTimeNavigation();
    
    void loadSettings();
    void saveSettings();
    QString getLastDirectory();
    void setLastDirectory(const QString& dirPath);
    void addToRecentFiles(const QString& filePath);
    void updateRecentFilesMenu();
    QString shortenPath(const QString& fullPath, int maxLength = 80);
    void updateWindowTitle(const QString& filePath = QString());
    
    QString loadParserPattern();
    void optimizePlotting();
    QVector<DataPoint> smoothData(const QVector<DataPoint>& data, int windowSize);
    void plotData(const QVector<DataPoint>& data);
    
    // Методы навигации
    void setTimeScale(TimeScale scale);
    void navigateTime(int direction); // -1 назад, +1 вперед
    void updateCrosshair(double x, double y);
    void hideCrosshair();
    QString formatValueAtPosition(double timePos);
    QString getVisibleRangeStats();
    void updateStatusStats();
    
    void renderChartToPrinter(QPrinter *printer, bool useColor = true);  // ДОБАВЛЕН параметр useColor
    
    void toggleDisplayMode();              // метод переключения

private:
    // ДОБАВИТЬ: настройки визуализации
    struct VisualizationSettings {
        // Цвета
        QColor temperatureColor = Qt::red;
        QColor humidityColor = Qt::blue;
        QColor gridColor = Qt::lightGray;
        QColor crosshairColor = Qt::darkGray;
        
        // Толщина линий
        int temperatureLineWidth = 2;
        int humidityLineWidth = 2;
        int gridLineWidth = 1;
        int crosshairLineWidth = 1;
        
        // Параметры сглаживания
        int smoothingWindow = 5;
        bool enableSmoothing = true;
        
        // Размеры точек
        int pointSize = 4;
        
        // Прозрачность
        int gridOpacity = 255;      // 0-255
        int crosshairOpacity = 200; // 0-255
    } vizSettings;
    
    void loadVisualizationSettings(QSettings& settings);
    void loadDefaultVisualizationSettings();
    void applyVisualizationSettings();
};

#endif // MAINWINDOW_H
