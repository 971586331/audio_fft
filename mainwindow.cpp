#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fftw-3.3.5-dll32/fftw3.h"
#include <QDebug>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtWidgets/QVBoxLayout>
#include <QtCharts/QValueAxis>
#include "xyseriesiodevice.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QFileDialog>
#include <QAudioRecorder>
#include <QMessageBox>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QStandardPaths>
#include <QDateTimeAxis>
#include <QDateTime>
#include "qcustomplot.h"

#define SAMPLE_RATE  (48000)
#define BIT_RATE     (16)
#define BYTE_RATE    (2)

//将wav文件转为pcm文件
int wav2pcm(char *in_file, char *out_file)
{
    size_t result;
    char  *buf;
    FILE *fp1=fopen(in_file, "rb");//wav文件打开，打开读权限
    FILE *fp2=fopen(out_file, "wb");//pcm文件创建，给予写权限
    fseek(fp1,0,SEEK_END);//文件指针从0挪到尾部
    long filesize;
    filesize=ftell(fp1);//ftell求文件指针相对于0的便宜字节数，就求出了文件字节数

    if(fp1==NULL||fp2==NULL)//判断两个文件是否打开
    {
        qDebug() << "file open filed!!" << endl;
        return -1;
    }

    rewind(fp1);//还原指针位置
    fseek(fp1,44,SEEK_SET);//wav文件的指针从头向后移动44字节
    buf=(char *)malloc(sizeof(char)*filesize);//开辟空间给缓存数组

    if(buf==NULL)
    {
        qDebug("memory  error");
        return -1;
    }

    result =fread(buf,1,(filesize-44),fp1);//每次读一个字节到buf，同时求读的次数
    if(result != (filesize-44))//判断读的次数和文件大小是否一致
    {
        qDebug() << "reing error!!" << endl;
        return -1;
    }
    fwrite(buf,1,(filesize-44),fp2);//写到pcm文件中
    fclose(fp1);//关闭文件指针
    fclose(fp2);
    free (buf);//释放buf
    return 0 ;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //创建一个顶层的widget
    QWidget *widget = new QWidget();
    this->setCentralWidget(widget);

    //创建标签
    QLabel * switch_label = new QLabel("选择录音文件1路径：");

    //创建一个文本框
    path_lineedit = new QLineEdit();

    //选择路径按钮
    QPushButton * save_button = new QPushButton("打开");
    connect(save_button, SIGNAL(clicked()), this, SLOT(slots_save_button_clicked()));

    //选择路径的水平布局
    QHBoxLayout *hLayout_1 = new QHBoxLayout();
    hLayout_1->addWidget(switch_label);
    hLayout_1->addWidget(path_lineedit);
    hLayout_1->addWidget(save_button);
    hLayout_1->setSpacing(10);
    hLayout_1->setContentsMargins(0,0,10,10);

    //创建标签
    QLabel * switch_label_2 = new QLabel("选择录音文件2路径：");

    //创建一个文本框
    path_lineedit_2 = new QLineEdit();

    //选择路径按钮
    QPushButton * save_button_2 = new QPushButton("打开");
    connect(save_button_2, SIGNAL(clicked()), this, SLOT(slots_save_button_2_clicked()));

    //选择路径的水平布局
    QHBoxLayout *hLayout_1_2 = new QHBoxLayout();
    hLayout_1_2->addWidget(switch_label_2);
    hLayout_1_2->addWidget(path_lineedit_2);
    hLayout_1_2->addWidget(save_button_2);
    hLayout_1_2->setSpacing(10);
    hLayout_1_2->setContentsMargins(0,0,10,10);

    //开始录音
    QPushButton * start_recorder_button = new QPushButton("开始录音");
    connect(start_recorder_button, SIGNAL(clicked()), this, SLOT(slots_start_recorder_button_clicked()));
    //停止录音
    QPushButton * stop_recorder_button = new QPushButton("停止录音");
    connect(stop_recorder_button, SIGNAL(clicked()), this, SLOT(slots_stop_recorder_button_clicked()));
    //转为PCM文件
    QPushButton * transform_button = new QPushButton("转为PCM文件");
    connect(transform_button, SIGNAL(clicked()), this, SLOT(slots_transform_button_clicked()));
    //播放录音
    QPushButton * play_pcm_button = new QPushButton("播放录音");
    connect(play_pcm_button, SIGNAL(clicked()), this, SLOT(slots_play_pcm_button_clicked()));
    //开始FFT
    QPushButton * start_fft_button = new QPushButton("开始FFT");
    connect(start_fft_button, SIGNAL(clicked()), this, SLOT(slots_start_fft_button_clicked()));

    //按钮的水平布局
    QHBoxLayout *hLayout_2 = new QHBoxLayout();
    hLayout_2->addWidget(start_recorder_button);
    hLayout_2->addWidget(stop_recorder_button);
    hLayout_2->addWidget(transform_button);
    hLayout_2->addWidget(play_pcm_button);
    hLayout_2->addWidget(start_fft_button);
    hLayout_2->setSpacing(10);
    hLayout_2->setContentsMargins(0,0,10,10);

    //创建tabwidget
    QTabWidget * tabwidget = new QTabWidget();

    //麦克网波形
    mic_chart = new QChart;
    QChartView *mic_chartView = new QChartView(mic_chart);
    mic_series = new QLineSeries;
    mic_chart->addSeries(mic_series);
    QValueAxis *mic_axisX = new QValueAxis;
    mic_axisX->setRange(0, 2000);
    mic_axisX->setLabelFormat("%g");
    mic_axisX->setTitleText("Samples");
    QValueAxis *mic_axisY = new QValueAxis;
    mic_axisY->setRange(-1, 1);
    mic_axisY->setTitleText("Audio level");
    mic_chart->setAxisX(mic_axisX, mic_series);
    mic_chart->setAxisY(mic_axisY, mic_series);
    mic_chart->legend()->hide();
    mic_chart->setTitle("Data from the microphone");

    //设置麦克网录音格式
    QAudioFormat formatAudio;
    formatAudio.setSampleRate(8000);
    formatAudio.setChannelCount(1);
    formatAudio.setSampleSize(8);
    formatAudio.setCodec("audio/pcm");
    formatAudio.setByteOrder(QAudioFormat::LittleEndian);
    formatAudio.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo inputDevices = QAudioDeviceInfo::defaultInputDevice();
    mic_audioInput = new QAudioInput(inputDevices,formatAudio, mic_chartView);
    mic_device = new XYSeriesIODevice(mic_series, mic_chartView);
    mic_device->open(QIODevice::WriteOnly);
    mic_audioInput->start(mic_device);

    tabwidget->addTab(mic_chartView, "本机麦克风");

    //
    QWidget *wave_widget = new QWidget();
    //按钮的水平布局
    QGridLayout *gLayout = new QGridLayout();

    //输入的时域波形
    QWidget *in_pcm_widget = new QWidget();
    in_pcm_polt = new QCustomPlot(in_pcm_widget);
    in_pcm_polt->plotLayout()->insertRow(0);
    QCPTextElement *in_pcm_title = new QCPTextElement(in_pcm_polt, "输入的时域波形", QFont("sans", 10, QFont::Bold));
    in_pcm_polt->plotLayout()->addElement(0, 0, in_pcm_title);
    in_pcm_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    in_pcm_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    in_pcm_polt->xAxis->setLabel("时间(ms)");
    in_pcm_polt->yAxis->setLabel("电压采样值(V)");
    in_pcm_polt->addGraph();

    in_pcm_polt->graph(0)->setPen(QPen(Qt::red));
    in_pcm_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(in_pcm_polt, 0, 0);


    //输入的频域上的幅度
    QWidget *in_amplitude_widget = new QWidget();
    in_amplitude_polt = new QCustomPlot(in_amplitude_widget);
    in_amplitude_polt->plotLayout()->insertRow(0);
    QCPTextElement *in_amplitude_title = new QCPTextElement(in_amplitude_polt, "输入的频域幅度", QFont("sans", 10, QFont::Bold));
    in_amplitude_polt->plotLayout()->addElement(0, 0, in_amplitude_title);
    in_amplitude_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    in_amplitude_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    in_amplitude_polt->xAxis->setLabel("频率(Hz)");
    in_amplitude_polt->yAxis->setLabel("幅度(V)");
    in_amplitude_polt->addGraph();

    in_amplitude_polt->graph(0)->setPen(QPen(Qt::red));
    in_amplitude_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(in_amplitude_polt, 0, 1);

    //输入的频域上的相位
    QWidget *in_phase_widget = new QWidget();
    in_phase_polt = new QCustomPlot(in_phase_widget);
    in_phase_polt->plotLayout()->insertRow(0);
    QCPTextElement *in_phase_title = new QCPTextElement(in_phase_polt, "输入的频域相位", QFont("sans", 10, QFont::Bold));
    in_phase_polt->plotLayout()->addElement(0, 0, in_phase_title);
    in_phase_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    in_phase_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    in_phase_polt->xAxis->setLabel("频率(Hz)");
    in_phase_polt->yAxis->setLabel("相位(度)");
    in_phase_polt->addGraph();

    in_phase_polt->graph(0)->setPen(QPen(Qt::red));
    in_phase_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(in_phase_polt, 0, 2);

    //输出的时域波形
    QWidget *out_pcm_widget = new QWidget();
    out_pcm_polt = new QCustomPlot(out_pcm_widget);
    out_pcm_polt->plotLayout()->insertRow(0);
    QCPTextElement *out_pcm_title = new QCPTextElement(out_pcm_polt, "输出的时域波形", QFont("sans", 10, QFont::Bold));
    out_pcm_polt->plotLayout()->addElement(0, 0, out_pcm_title);
    out_pcm_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    out_pcm_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    out_pcm_polt->xAxis->setLabel("时间(ms)");
    out_pcm_polt->yAxis->setLabel("电压采样值(V)");
    out_pcm_polt->addGraph();

    out_pcm_polt->graph(0)->setPen(QPen(Qt::red));
    out_pcm_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(out_pcm_polt, 1, 0);

    //输出的频域幅度
    QWidget *out_amplitude_widget = new QWidget();
    out_amplitude_polt = new QCustomPlot(out_amplitude_widget);
    out_amplitude_polt->plotLayout()->insertRow(0);
    QCPTextElement *out_amplitude_title = new QCPTextElement(out_amplitude_polt, "输出的频域幅度", QFont("sans", 10, QFont::Bold));
    out_amplitude_polt->plotLayout()->addElement(0, 0, out_amplitude_title);
    out_amplitude_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    out_amplitude_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    out_amplitude_polt->xAxis->setLabel("频率(Hz)");
    out_amplitude_polt->yAxis->setLabel("幅度(V)");
    out_amplitude_polt->addGraph();

    out_amplitude_polt->graph(0)->setPen(QPen(Qt::red));
    out_amplitude_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(out_amplitude_polt, 1, 1);

    //输出的频域相位
    QWidget *out_phase_widget = new QWidget();
    out_phase_polt = new QCustomPlot(out_phase_widget);
    out_phase_polt->plotLayout()->insertRow(0);
    QCPTextElement *out_phase_title = new QCPTextElement(out_phase_polt, "输出的频域相位", QFont("sans", 10, QFont::Bold));
    out_phase_polt->plotLayout()->addElement(0, 0, out_phase_title);
    out_phase_polt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    out_phase_polt->axisRect()->setRangeZoom(Qt::Horizontal);
    out_phase_polt->xAxis->setLabel("频率(Hz)");
    out_phase_polt->yAxis->setLabel("相位(度)");
    out_phase_polt->addGraph();

    out_phase_polt->graph(0)->setPen(QPen(Qt::red));
    out_phase_polt->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);

    gLayout->addWidget(out_phase_polt, 1, 2);

    wave_widget->setLayout(gLayout);
    tabwidget->addTab(wave_widget, "FFT");

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout_1);
    vLayout->addLayout(hLayout_1_2);
    vLayout->addLayout(hLayout_2);
    vLayout->addWidget(tabwidget);

    widget->setLayout(vLayout);

    //录音初始化
    audioRecorder = new QAudioRecorder;
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setSampleRate(SAMPLE_RATE);
    audioSettings.setChannelCount(1);
    audioSettings.setBitRate(BIT_RATE);
    //audioSettings.setQuality(QMultimedia::LowQuality);
    //audioSettings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    audioRecorder->setEncodingSettings(audioSettings);

    //状态栏
    QStatusBar* bar = statusBar(); //获取状态栏
    status_Label = new QLabel; //新建标签
    bar->addWidget(status_Label);

    //播放设置

    QFile inputFile;
    inputFile.setFileName("C:/Users/sy/Desktop/123.wav");
    inputFile.open(QIODevice::ReadOnly);

    effect = new QSoundEffect();
}

MainWindow::~MainWindow()
{
    mic_audioInput->stop();
    mic_device->close();

    delete ui;
}

//选择路径
void MainWindow::slots_save_button_clicked()
{
    QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存文件",
                                                    desktop_path,
                                                    "*.");
    path_lineedit->setText(fileName);

    audioRecorder->setOutputLocation(QUrl::fromLocalFile(fileName));
}

//选择路径
void MainWindow::slots_save_button_2_clicked()
{
    QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存文件",
                                                    desktop_path,
                                                    "*.");
    path_lineedit_2->setText(fileName);

    audioRecorder->setOutputLocation(QUrl::fromLocalFile(fileName));
}

//开始录音
void MainWindow::slots_start_recorder_button_clicked()
{
    audioRecorder->record();
    status_Label->setText("开始录音");
}

//停止录音
void MainWindow::slots_stop_recorder_button_clicked()
{
    audioRecorder->stop();
    status_Label->setText("录音结束");
}


//转PCM文件
void MainWindow::slots_transform_button_clicked()
{
    QString  in_str, out_str;
    in_str = path_lineedit->text() + ".wav";
    out_str = path_lineedit->text() + ".pcm";

    QFileInfo fileInfo(in_str);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(NULL, "错误", "\"" + in_str + "\"" + "文件不存在！", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    char * in_ch;
    QByteArray in_ba = in_str.toLatin1();
    in_ch = in_ba.data();

    char * out_ch;
    QByteArray out_ba = out_str.toLatin1();
    out_ch = out_ba.data();

    int ret = wav2pcm(in_ch, out_ch);
    if( ret != 0 )
    {
        QMessageBox::critical(NULL, "错误", "转换失败！", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    status_Label->setText("转换完成");
}

//播放录音
void MainWindow::slots_play_pcm_button_clicked()
{
    QString out_str;
    out_str = path_lineedit->text() + ".wav";

    effect->setSource(QUrl::fromLocalFile(out_str));
    effect->setVolume(1.0f);
    effect->play();
}

//显示时域波形
int MainWindow::show_time_waveform(char * file_path, QCustomPlot * polt)
{
    size_t result;
    char  *buf;
    FILE *fp1=fopen(file_path, "rb");   //打开读权限
    fseek(fp1,0,SEEK_END);//文件指针从0挪到尾部
    long filesize;
    filesize=ftell(fp1);//ftell求文件指针相对于0的便宜字节数，就求出了文件字节数

    if(fp1==NULL)//判断两个文件是否打开
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "文件打开失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    //计算时间，设置坐标轴
    int fp_ms = (filesize / sizeof(unsigned short))*1000 / SAMPLE_RATE;
    polt->xAxis->setRange(0, fp_ms, Qt::AlignLeft);
    polt->yAxis->setRange(0, 2, Qt::AlignBottom);

    rewind(fp1);//还原指针位置
    buf=(char *)malloc(filesize);//开辟空间给缓存数组
    if(buf==NULL)
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "内存分配失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    result =fread(buf, 1, filesize, fp1);//每次读一个字节到buf，同时求读的次数
    if(result != filesize)//判断读的次数和文件大小是否一致
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "文件读取失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    for( int i=0; i<filesize/2; i++ )
    {
        short x = *((short *)(buf+(i*2)));
        polt->graph(0)->addData( ((double)1000/SAMPLE_RATE)*i, (((double)x)/32768) );
    }
    polt->replot();

    fclose(fp1);//关闭文件指针
    free (buf);//释放buf
}

//显示频域的幅度
int MainWindow::show_amplitude_waveform(char * file_path, QCustomPlot *polt_1, QCustomPlot *polt_2)
{
    double val_max = 0;
    size_t result;
    char  *buf;
    short *in_buf;
    FILE *fp1=fopen(file_path, "rb");   //打开读权限
    fseek(fp1,0,SEEK_END);//文件指针从0挪到尾部
    long filesize;
    filesize=ftell(fp1);//ftell求文件指针相对于0的便宜字节数，就求出了文件字节数
    int N = filesize/2;

    if(fp1==NULL)//判断两个文件是否打开
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "文件打开失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    rewind(fp1);//还原指针位置
    buf=(char *)malloc(filesize);//开辟空间给缓存数组
    if(buf==NULL)
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "内存分配失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    result =fread(buf, 1, filesize, fp1);//每次读一个字节到buf，同时求读的次数
    if(result != filesize)//判断读的次数和文件大小是否一致
    {
        QMessageBox::critical(NULL, "错误", "\"" + QString(QLatin1String(file_path)) + "\"" + "文件读取失败！", QMessageBox::Yes, QMessageBox::Yes);
        return -1;
    }

    in_buf = (short *)buf;

    double * in = (double*)fftw_malloc(sizeof(double) * N);
    for(int i=0; i<N; i++)
    {
        in[i] = in_buf[i];  //将pcm文件中的数据复制到fft的输入
    }

    fftw_complex * out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

    fftw_plan p = FFTW3_H::fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(p);

    double dx3 = (double)SAMPLE_RATE / N;

    polt_1->xAxis->setRange(0, SAMPLE_RATE/2, Qt::AlignLeft);

    //根据FFT计算的复数计算振幅谱
    for( int i=0; i<N/2; i++ )
    {
        double val = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        val = val / (N / 2);
        polt_1->graph(0)->addData( dx3 * i, val );

        if( val > val_max )
        {
            val_max = val;
        }

        double db = log(val);
        //qDebug("frequency = %f, amplitude = %f, db = %f", dx3 * i, val / (N / 2), db);
    }

    polt_1->yAxis->setRange(val_max*0.6, val_max*1.2, Qt::AlignBottom);
    polt_1->replot();

    polt_2->xAxis->setRange(0, SAMPLE_RATE/2, Qt::AlignLeft);
    polt_2->yAxis->setRange(0, 10, Qt::AlignBaseline);
    //根据FFT计算的复数计算相位谱
    for( int i=0; i<N/2; i++ )
    {
        double val = atan2(out[i][1], out[i][0]);
        polt_2->graph(0)->addData( dx3 * i, val );
    }
    polt_2->replot();

    fclose(fp1);    //关闭文件指针
    fftw_destroy_plan(p);
    free(buf);      //释放buf
    fftw_free(in);
    fftw_free(out);

    return 0;
}

//开始FFT
void MainWindow::slots_start_fft_button_clicked()
{
    QString  in_str;
    in_str = path_lineedit->text() + ".pcm";

    QFileInfo in_fileInfo(in_str);
    if(!in_fileInfo.isFile())
    {
        QMessageBox::critical(NULL, "错误", "\"" + in_str + "\"" + "文件不存在！", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    char * in_ch;
    QByteArray in_ba = in_str.toLatin1();
    in_ch = in_ba.data();

    //时域波形
    show_time_waveform(in_ch, in_pcm_polt);

    //频域的幅度和相位
    show_amplitude_waveform(in_ch, in_amplitude_polt, in_phase_polt);

    QString  out_str;
    out_str = path_lineedit_2->text() + ".pcm";

    QFileInfo out_fileInfo(out_str);
    if(!out_fileInfo.isFile())
    {
        QMessageBox::critical(NULL, "错误", "\"" + in_str + "\"" + "文件不存在！", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    char * out_ch;
    QByteArray out_ba = out_str.toLatin1();
    out_ch = out_ba.data();

    //时域波形
    show_time_waveform(out_ch, out_pcm_polt);

    //频域的幅度和相位
    show_amplitude_waveform(out_ch, out_amplitude_polt, out_phase_polt);
}
