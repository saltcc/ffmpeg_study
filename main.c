#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>


int main()
{
    const char *input = "test.mp3";
    const char *output = "test.pcm";

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
        return 0;
    }

    if ((audio_stream_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) < 0){
        av_log(NULL, AV_LOG_INFO, "av_find_best_stream\n");
        return 0;
    }

    av_dump_format(pFormatCtx, audio_stream_index, input, 0);

    AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio_stream_index]->codecpar->codec_id);

    if (pCodec == NULL){
        av_log(NULL, AV_LOG_INFO, "find audio codec fail\n");
        return 0;
    }

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);

    if (pCodecCtx == NULL){
        av_log(NULL, AV_LOG_INFO, "avcodec_alloc_content3 fail\n");
        return 0;
    }

    
    // ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audio_stream_index]->codecpar);
    // if (ret < 0){
    //     av_log(NULL, AV_LOG_INFO, "avcodec_parameters_to_context fail\n");
    //     return 0;
    // }
    // AVCodecContext *pCodecCtx = pFormatCtx->streams[audio_stream_index]->codec;

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0){
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 fail\n");
        return 0;
    }

    // uint64_t out_channel_layout = AV_CH_LAYOUT_MONO;
    // int out_nb_samples = 1024;
    // enum AVSampleFormat  sample_fmt = AV_SAMPLE_FMT_S16;
    // int out_sample_rate = 44100;
    // int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    // int buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, sample_fmt, 1);
    // uint8_t *buffer = (uint8_t *)av_malloc(192000 * 2);
    // int64_t in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    // struct SwrContext *convert_ctx = swr_alloc();
    // convert_ctx = swr_alloc_set_opts(convert_ctx, out_channel_layout, sample_fmt, out_sample_rate, in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    AVFrame *frame = av_frame_alloc();

    int got_frame = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0){
        int ret = 0;
        int i = 0;
        int ch = 0;
        if (packet->size <= 0)
            continue;
            
        if ((ret = avcodec_send_packet(pCodecCtx, packet)) < 0)
        {
            return 0;
        }

        int data_size = av_get_bytes_per_sample(pCodecCtx->sample_fmt);

        while ((ret = avcodec_receive_frame(pCodecCtx, frame)) >= 0){
            
            for (i = 0; i < frame->nb_samples; i++){
                for (ch = 0; ch < pCodecCtx->channels; ch++){
                    fwrite(frame->data[ch] + data_size * i, 1, data_size, fout);
                }
            }
        }
        // if (packet->stream_index == audio_stream_index){
        //     int ret2 = avcodec_decode_audio4(pCodecCtx, frame, &got_frame, packet);

        //     if (ret2 < 0){
        //         av_log(NULL, AV_LOG_INFO, "avcodec_decode_audio4 fail\n");
        //         break;
        //     }
        //     if (got_frame > 0){

        //         swr_convert(convert_ctx, &buffer, 192000, (const uint8_t **)frame->data, frame->nb_samples);
        //         //size_t datalen = decoded_frame->nb_samples * av_get_bytes_per_sample(decoded_frame->format);
        //         size_t datalen = frame->nb_samples * 1 * 2;
        //         printf("zz:%d %d\n",av_get_bytes_per_sample(frame->format),datalen);
        //         fwrite(buffer, 1, buffer_size, fout);
        //     }
        //     got_frame = 0;
        // }
        // av_free_packet(packet);
    }

    fclose(fout);

    return 0;
}
