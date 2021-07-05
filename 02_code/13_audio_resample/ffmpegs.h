#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
    #include <libavformat/avformat.h>
}

typedef struct {
    /**
     * 文件名
     */
    const char *filename;
    /**
     * 采样率
     */
    int sampleRate;
    /**
     * 格式
     */
    AVSampleFormat sampleFmt;
    /**
     * 声道Layout
     */
    int chLayout;
} ResampleAudioSpec;

class FFmpegs
{
public:
    FFmpegs();
    static void resampleAudio(ResampleAudioSpec &in,
                              ResampleAudioSpec &out);

    static void resampleAudio(const char *inFilename,
                              int inSampleRate,
                              AVSampleFormat inSampleFmt,
                              int inChLayout,

                              const char *outFilename,
                              int outSampleRate,
                              AVSampleFormat outSampleFmt,
                              int outChLayout);
};

#endif // FFMPEGS_H
