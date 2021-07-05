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
     * 采样格式
     */
    AVSampleFormat sampleFmt;
    /**
     * 声道Layout
     */
    int chLayout;
} AudioEncodeSpec;

class FFmpegs
{
public:
    FFmpegs();
    static void aacEncode(AudioEncodeSpec &in, const char *outFilename);
};

#endif // FFMPEGS_H
