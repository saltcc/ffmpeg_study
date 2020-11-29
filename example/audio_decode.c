#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

void decode_it(AVCodecContext *pCodecCtx, AVPacket *packet, AVFrame *frame, FILE *fout)
{
    int ret = 0;
    int i = 0;
    int ch = 0;

    if ((ret = avcodec_send_packet(pCodecCtx, packet)) < 0){
        return;
    }

    int data_size = av_get_bytes_per_sample(pCodecCtx->sample_fmt);

    while ((ret = avcodec_receive_frame(pCodecCtx, frame)) >= 0){
        for (i = 0; i < frame->nb_samples; i++){
            for (ch = 0; ch < pCodecCtx->channels; ch++){
                fwrite(frame->data[ch] + data_size * i, 1, data_size, fout);
            }
        }
    }
    
    return;
}

int decode_audio(const char *input, const char *output)
{
    FILE *fout = fopen(output, "wb");
    int ret = -1;

    if (fout == NULL){
        printf("fopen [%s] fail\n", output);
        return 0;
    }
    
    AVFrame *decoded_frame = NULL;

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0){
        av_log(NULL,AV_LOG_ERROR,"open [%s] fail\n", input);
        exit(-1);
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0){
        av_log(NULL, AV_LOG_ERROR, "find stream fail\n");
        exit(-1);
    }

    int32_t audio_stream_index = -1;

    for (int32_t i = 0; i < pFormatCtx->nb_streams; ++i){
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
            av_log(NULL, AV_LOG_INFO, "audio stream index:%d\n", i);
            break;
        }
    }

    if (audio_stream_index == -1){
        av_log(NULL, AV_LOG_INFO, "not find audio stream\n");
        goto over;
    }

    av_dump_format(pFormatCtx, audio_stream_index, input, 0);

    AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio_stream_index]->codecpar->codec_id);

    if (pCodec == NULL){
        av_log(NULL, AV_LOG_INFO, "find audio codec fail\n");
        goto over;
    }

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);

    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audio_stream_index]->codecpar);

    if (pCodecCtx == NULL){
        av_log(NULL, AV_LOG_INFO, "avcodec_alloc_content3 fail\n");
        goto over;
    }

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0){
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 fail\n");
        goto over;
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    int got_frame = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0){

        if (packet->size <= 0){
            continue;
        }
        if (packet->stream_index == audio_stream_index)
            decode_it(pCodecCtx, packet, frame, fout);
    }

over:
    if (pFormatCtx)
        avformat_free_context(pFormatCtx);
    if (pCodecCtx)
        avcodec_free_context(&pCodecCtx);
    if (frame)
        av_frame_free(&frame);
    if (packet)
        av_packet_free(&packet);
    if (fout)
        fclose(fout);

    return 0;
}