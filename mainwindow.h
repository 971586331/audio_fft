#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCharts/QChartGlobal>
#include <QLineEdit>
#include <QLabel>
#include <QAudioRecorder>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QDateTimeAxis>
#include <QDateTime>
#include "qcustomplot.h"

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

    int show_time_waveform(char * file_path, QCustomPlot *polt);
    int show_amplitude_waveform(char * file_path, QCustomPlot *polt_1, QCustomPlot *polt_2);

private slots:
    void slots_save_button_clicked(void);
    void slots_save_button_2_clicked(void);
    void slots_start_recorder_button_clicked(void);
    void slots_stop_recorder_button_clicked(void);
    void slots_transform_button_clicked(void);
    void slots_play_pcm_button_clicked(void);
    void slots_start_fft_button_clicked(void);

private:
    Ui::MainWindow *ui;

    XYSeriesIODevice *mic_device;
    QChart *mic_chart;
    QLineSeries *mic_series;
    QAudioInput *mic_audioInput;

    QLineEdit * path_lineedit;
    QLineEdit * path_lineedit_2;
    QLabel * status_Label;
    QAudioRecorder * audioRecorder;
    QAudioOutput * audio;
    QSoundEffect * effect;

    QCustomPlot *in_pcm_polt;
    QCustomPlot *in_amplitude_polt;
    QCustomPlot *in_phase_polt;
    QCustomPlot *out_pcm_polt;
    QCustomPlot *out_amplitude_polt;
    QCustomPlot *out_phase_polt;

    QLineSeries *amplitude_series;
    QLineSeries *phase_series;

    QLineSeries *amplitude_series_out;
    QLineSeries *phase_series_out;
};

#endif // MAINWINDOW_H
