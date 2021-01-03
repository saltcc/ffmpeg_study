#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int decode_video_pic(const char *input, const char *output){
    
    AVFormatContext *pFormatCtx = NULL;

    pFormatCtx = avformat_alloc_context();
    return 0;
}