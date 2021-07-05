#include "playthread.h"
#include <QThread>
#include <QtDebug>

#include <SDL2/SDL.h>
#include <QFile>

#define FILENAME "D:/Dev/C_CPP/Codes/ffmpeg/10_sdl_play_pcm/in.pcm"
// 采样率：每秒采集的样本数量。采样率44.1kHz表示1秒钟采集44100个样本。
#define SAMPLE_RATE 44100
// 采样格式（s16le）
#define SAMPLE_FORMAT AUDIO_S16LSB
// 将采样格式AUDIO_S16LSB（0x8010）按位与0xFF，得到位深度0x10（十进制16）
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节（4个字节）。向右位移3位表示：除以8
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
// 文件缓冲区的大小（4096）
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

typedef struct {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);
}

// 析构函数
PlayThread::~PlayThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

// 等待音频设备回调（会回调多次）
void pull_audio_data(void *userdata,
                     // 需要往stream中填充PCM数据
                     Uint8 * stream,
                     // 希望填充的大小（samples * format * channels / 8）
                     int len) {
    qDebug() << "pull_audio_data" << len;
    // 清空stream（静音处理）
    SDL_memset(stream, 0, len);

    AudioBuffer *buffer = (AudioBuffer *)userdata;

    //文件数据还没准备好
    if (buffer->len <= 0) return;

    //取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;

    //填充数据
    SDL_MixAudio(stream,  buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

/*
 SDL播放音频有2种模式：
 Push（推）：【程序】主动推送数据给【音频设备】
 Pull（拉）：【音频设备】主动向【程序】拉取数据
 */
void PlayThread::run() {
    // 初始化Audio子系统
    if (SDL_Init(SDL_INIT_AUDIO)) {//在C语言当中，非0为真
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    // 音频参数
    SDL_AudioSpec spec;
    // 采样率：每秒采集的样本数量
    spec.freq = SAMPLE_RATE;
    // 采样格式（s16le）
    spec.format = SAMPLE_FORMAT;
    // 声道数
    spec.channels = CHANNELS;
    // 音频缓冲区的样本数量（这个值必须是2的幂）
    spec.samples = SAMPLES;
    // 回调
    spec.callback = pull_audio_data;
    // 传递回调的参数
    AudioBuffer buffer;
    spec.userdata = &buffer;

    // 打开设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        //清除所有的子系统
        SDL_Quit();
        return;
    }

    // 打开文件
    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
        // 关闭设备
        SDL_CloseAudio();
        // 清除所有的子系统
        SDL_Quit();
        return;
    }

    // 开始播放（0是取消暂停）
    SDL_PauseAudio(0);

    // 存放从文件中读取的数据
    Uint8 data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if(buffer.len > 0) continue;
        buffer.len = file.read((char *) data, BUFFER_SIZE);
        // 文件数据已经读取完毕
        if (buffer.len <= 0) {
            // 剩余的样本数量 = （剩余的字节数 / 每个样本的大小）
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            // 剩余的时间 = 剩余的样本数量 / 采样率
            int ms = samples / SAMPLE_RATE;
            SDL_Delay(ms);
            qDebug() << ms;
            break;
        }

        //读取到了文件数据
        buffer.data = data;
    }

    // 采样率 freq（每秒采集的样本数量）：SAMPLE_RATE 44100

    // 每个样本的大小(size)：BYTES_PER_SAMPLE 4字节

    // 字节率：每秒要处理的字节数
    // 字节率 = 采样率 * size

    // 假如有20000个字节数据要播放，那么 20000 / 字节率 = 时间

    // 关闭文件
    file.close();

    // 关闭设备
    SDL_CloseAudio();

    // 清除所有的子系统
    SDL_Quit();
}


