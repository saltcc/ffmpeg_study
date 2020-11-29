#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

extern int decode_audio();

int main()
{
    const char *input = "test.mp4";
    const char *output = "test.pcm";
    decode_audio(input, output);

    return 0;
}
