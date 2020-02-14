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

#define SAMPLE_RATE  (48000)
#define BIT_RATE     (16)
#define BYTE_RATE    (2)

void fft_test(void)
{
    int N = 5;

    /**********************一维复数DFT变换，复数到复数**********************/
    fftw_complex *in1_c, *out1_c;//声明复数类型的输入数据in1_c和FFT变换的结果out1_c
    fftw_plan p;//声明变换策略
    in1_c = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* N);//申请动态内存,这里构造二维数组的方式值得学习
    out1_c = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* N);
    p = fftw_plan_dft_1d(N, in1_c, out1_c, FFTW_FORWARD, FFTW_ESTIMATE);//返回变换策略

    int n;
    for (n = 0; n<N; n++)//构造输入数据
    {
        in1_c[n][0] = 1;
        in1_c[n][1] = 2;
        //*(*(in1_c + n) + 0) = 1;
        //*(*(in1_c + n) + 1) = 2;
    }
    fftw_execute(p);//执行变换
    fftw_destroy_plan(p);//销毁策略

    //以下为打印代码
    qDebug("data of FFT is:\n");
    for (n = 0; n<N; n++)
    {
        qDebug("%3.2lf+%3.2lfi    ", in1_c[n][0], in1_c[n][1]);
    }
    qDebug("\n");
    qDebug("result of FFT is:\n");
    for (n = 0; n<N; n++)
    {
        qDebug("%3.2lf+%3.2lfi    ", out1_c[n][0], out1_c[n][1]);
        qDebug()<<out1_c[n][0]<<out1_c[n][1];
    }
    qDebug("\n");
    fftw_free(in1_c); fftw_free(out1_c);//释放内存
}

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
    QLabel * switch_label = new QLabel("选择录音文件路径：");

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

    //时域波形
    pcm_chart = new QChart;
    QChartView *pcm_chartView = new QChartView(pcm_chart);
    pcm_chartView->setRubberBand(QChartView::HorizontalRubberBand);
    pcm_series = new QLineSeries;
    pcm_chart->addSeries(pcm_series);
    pcm_axisX = new QDateTimeAxis;
    pcm_min.setMSecsSinceEpoch(0);
    pcm_max.setMSecsSinceEpoch(10000);
    pcm_axisX->setRange(pcm_min, pcm_max);
    pcm_axisX->setFormat("ss.zzz");
    pcm_axisX->setTitleText("time");
    QValueAxis *pcm_axisY = new QValueAxis;
    pcm_axisY->setRange(-1, 1);
    pcm_axisY->setTitleText("Audio level");
    pcm_chart->setAxisX(pcm_axisX, pcm_series);
    pcm_chart->setAxisY(pcm_axisY, pcm_series);
    pcm_chart->legend()->hide();
    pcm_chart->setTitle("Data from the pcm file");

    tabwidget->addTab(pcm_chartView, "时域波形");

    //频域上的幅度
    QChart *amplitude_chart = new QChart;
    QChartView *amplitude_chartView = new QChartView(amplitude_chart);
    amplitude_chartView->setRubberBand(QChartView::HorizontalRubberBand);
    amplitude_series = new QLineSeries;
    amplitude_chart->addSeries(amplitude_series);
    QValueAxis *amplitude_axisX = new QValueAxis;
    amplitude_axisX->setRange(0, 65536);
    amplitude_axisX->setLabelFormat("%g");
    amplitude_axisX->setTitleText("time");
    QValueAxis *amplitude_axisY = new QValueAxis;
    amplitude_axisY->setRange(0, 1000);
    amplitude_axisY->setTitleText("Audio level");
    amplitude_chart->setAxisX(amplitude_axisX, amplitude_series);
    amplitude_chart->setAxisY(amplitude_axisY, amplitude_series);
    amplitude_chart->legend()->hide();
    amplitude_chart->setTitle("Data from the pcm file");

    tabwidget->addTab(amplitude_chartView, "频域上的幅度");

    //频域上的相位
    QChart *phase_chart = new QChart;
    QChartView *phase_chartView = new QChartView(phase_chart);
    QLineSeries *phase_series = new QLineSeries;
    phase_chart->addSeries(phase_series);
    QValueAxis *phase_axisX = new QValueAxis;
    phase_axisX->setRange(0, 2000);
    phase_axisX->setLabelFormat("%g");
    phase_axisX->setTitleText("time");
    QValueAxis *phase_axisY = new QValueAxis;
    phase_axisY->setRange(-1, 1);
    phase_axisY->setTitleText("Audio level");
    phase_chart->setAxisX(phase_axisX, phase_series);
    phase_chart->setAxisY(phase_axisY, phase_series);
    phase_chart->legend()->hide();
    phase_chart->setTitle("Data from the pcm file");

    tabwidget->addTab(phase_chartView, "频域上的相位");

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout_1);
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
int MainWindow::show_time_waveform(char * file_path)
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
    pcm_max.setMSecsSinceEpoch(fp_ms);
    pcm_axisX->setRange(pcm_min, pcm_max);

    if( fp_ms < 60*1000 )
        pcm_axisX->setFormat("ss.zzz");
    if( (fp_ms >= 60*1000) && (fp_ms < 60*60*1000) )
        pcm_axisX->setFormat("mm.ss.zzz");
    if( fp_ms > 60*60*1000 )
        pcm_axisX->setFormat("hh.mm.ss.zzz");

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

    QVector<QPointF> points;
    points = pcm_series->pointsVector();

    for( int i=0; i<filesize/2; i++ )
    {
        //short x = buf[i*2] + buf[i*2+1]*255;
        short x = *((short *)(buf+(i*2)));
        points.append(QPointF(((double)1000/SAMPLE_RATE)*i, ((double)x)/32768));
    }

    pcm_series->replace(points);

    fclose(fp1);//关闭文件指针
    free (buf);//释放buf
}

//显示频域的幅度
int MainWindow::show_amplitude_waveform(char * file_path)
{
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

    double dx3 = (double)48000 / N;

    QVector<QPointF> points;
    points = amplitude_series->pointsVector();

    for( int i=0; i<N; i++ )
    {
        double val = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        points.append(QPointF(dx3 * i, val / (N / 2)));

        //qDebug("dx3 = %f, val = %f", dx3 * i, val);
    }

    amplitude_series->replace(points);

    fclose(fp1);    //关闭文件指针
    fftw_destroy_plan(p);
    free(buf);      //释放buf
    fftw_free(in);
    fftw_free(out);
}

//开始FFT
void MainWindow::slots_start_fft_button_clicked()
{
    QString  in_str;
    in_str = path_lineedit->text() + ".pcm";

    QFileInfo fileInfo(in_str);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(NULL, "错误", "\"" + in_str + "\"" + "文件不存在！", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    char * in_ch;
    QByteArray in_ba = in_str.toLatin1();
    in_ch = in_ba.data();

    //时域波形
    show_time_waveform(in_ch);

    //频域的幅度
    show_amplitude_waveform(in_ch);

    //频域的相位
}
