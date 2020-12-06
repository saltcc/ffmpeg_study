#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

extern int decode_audio(const char *,const char *);
extern int audio_encode(const char *input, const char *output);

int main()
{
    const char *input = "test.mp4";
    const char *output = "test.pcm";
    decode_audio(input, output);

    const char *input1 = "test.pcm";
    const char *output1 = "test.aac";
    audio_encode(input1, output1);
    return 0;
}
