#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
    #include <libavutil/avutil.h>
}

typedef struct {
    /**
     * 文件名
     */
    const char *filename;
    int width;
    int height;
    /**
     * 像素格式
     */
    AVPixelFormat pixFmt;
    /**
     * 帧率
     */
    int fps;
} VideoEncodeSpec;

class FFmpegs
{
public:
    FFmpegs();
    static void h264Encode(VideoEncodeSpec &in, const char *outFilename);
};

#endif // FFMPEGS_H
