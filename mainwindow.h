#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCharts/QChartGlobal>
#include <QLineEdit>
#include <QLabel>
#include <QAudioRecorder>
#include <QAudioOutput>
#include <QSoundEffect>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class XYSeriesIODevice;

QT_BEGIN_NAMESPACE
class QAudioInput;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slots_save_button_clicked(void);
    void slots_start_recorder_button_clicked(void);
    void slots_stop_recorder_button_clicked(void);
    void slots_transform_button_clicked(void);
    void slots_play_pcm_button_clicked(void);

private:
    Ui::MainWindow *ui;

    XYSeriesIODevice *mic_device;
    QChart *mic_chart;
    QLineSeries *mic_series;
    QAudioInput *mic_audioInput;

    QLineEdit * path_lineedit;
    QLabel * status_Label;
    QAudioRecorder * audioRecorder;
    QAudioOutput * audio;
    QSoundEffect * effect;
};

#endif // MAINWINDOW_H
