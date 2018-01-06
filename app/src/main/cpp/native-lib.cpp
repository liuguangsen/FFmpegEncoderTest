#include <jni.h>
#include <string>
#include <android/log.h>
extern "C"
JNIEXPORT jstring JNICALL
Java_com_stick_gsliu_evoffmpegtest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
}

#define FFLOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);

extern "C"
JNIEXPORT void JNICALL
Java_com_stick_gsliu_evoffmpegtest_MainActivity_decode(JNIEnv *env, jclass type, jstring input_,
                                                       jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    // TODO
    FFLOGI("VideoEncoder  initEncode ...");
    /* AVCodec *c = NULL;
     do {
         c = av_codec_next(c);
         if (c != NULL) {
             FFLOGI("av_codec_next name:%s %s id:%d \n", c->name ? c->name : "", c->long_name ? c->long_name : "", c->id);
             if (c->decode != NULL) {
                 FFLOGI("VideoDecoder", "name:%s %s\n", c->name ? c->name : "",
                      c->long_name ? c->long_name : "");
             }
             if (c->encode2 != NULL) {
                 FFLOGI("VideoEncoder", "name:%s %s\n", c->name ? c->name : "",
                      c->long_name ? c->long_name : "");
             }

         }
     } while (c != NULL);

    AVCodec *temp = avcodec_find_encoder(AV_CODEC_ID_H264);
    FFLOGI("VideoEncoder", "temp name:%s %s\n", temp->name ? temp->name : "",
           temp->long_name ? temp->long_name : "");
    if (temp = NULL) {
        FFLOGI("VideoEncoder", "Didn't find EnCoder.\n");
    }
     */

    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int i, ret;
    FILE *fp_in;
    FILE *fp_out;
    AVFrame *pFrame;
    AVPacket pkt;
    int picture_size;//宽高
    uint8_t * picture_buf;
    int y_size;
    int framecnt = 0;

    AVCodecID codec_id = AV_CODEC_ID_H264;

    int in_w = 1920, in_h = 1080;
    int framenum = 156;

    avcodec_register_all();

    pCodec = avcodec_find_encoder(codec_id);
    if (!pCodec) {
        FFLOGI("VideoEncoder","Codec not found\n");
        return;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        FFLOGI("VideoEncoder","Could not allocate video codec context\n");
        return;
    }
    pCodecCtx->bit_rate = 10485760;//10485760;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;
    pCodecCtx->gop_size = 15;
    pCodecCtx->max_b_frames =0;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    AVDictionary *param = 0;
    if (codec_id == AV_CODEC_ID_H264)
        av_dict_set(&param, "preset", "slow", 0);

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        FFLOGI("VideoEncoder","Could not open codec\n");
        return;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        FFLOGI("VideoEncoder","Could not allocate video frame\n");
        return;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;

    picture_size = pCodecCtx->width * pCodecCtx->height * 3 / 2;
    //picture_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    picture_buf = (uint8_t *)av_malloc(picture_size);
    ret = av_image_fill_arrays(pFrame->data, pFrame->linesize, picture_buf,AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    ret = av_new_packet(&pkt, picture_size);


    //Input raw data
    fp_in = fopen(input, "rb");

    if (!fp_in) {
        FFLOGI("VideoEncoder","Could not open %s\n", fp_in);
        return;
    }
    //Output bitstream
    fp_out=fopen(output, "wb");

    if (!fp_out) {
        FFLOGI("VideoEncoder","Could not open %s\n", fp_out);
        return;
    }

    y_size = pCodecCtx->width * pCodecCtx->height;
    //Encode
    for (i = 0; i < framenum; i++) {
        //av_init_packet(&pkt);
        //pkt.data = NULL;    // packet data will be allocated by the encoder
        //pkt.size = 0;
        //Read raw YUV data
        //if (fread(pFrame->data[0], 1, y_size, fp_in) <= 0 ||       // Y
        //	fread(pFrame->data[1], 1, y_size / 4, fp_in) <= 0 || // U
        //	fread(pFrame->data[2], 1, y_size / 4, fp_in) <= 0) { // V
        //	return -1;
        //}
        //else if (feof(fp_in)) {
        //	break;
        //}

        if (fread(picture_buf, 1, (y_size * 3 / 2), fp_in) <= 0)
        {
            FFLOGI("VideoEncoder","Failed to read raw yuv data ! \n");
            return;
        }
        else if (feof(fp_in))
        {
            break;
        }

        pFrame->pts = i;
        /* encode the image */
        int ret = avcodec_send_frame(pCodecCtx, pFrame);

        ret = avcodec_receive_packet(pCodecCtx, &pkt);

        if (ret == 0) {
            FFLOGI("VideoEncoder","Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
            framecnt++;
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_packet_unref(&pkt);
        }
    }
    //Flush Encoder
    while (true)
    {
        //pkt.data = NULL;
        //pkt.size = 0;
        av_init_packet(&pkt);

        ret = avcodec_send_frame(pCodecCtx, NULL);
        ret = avcodec_receive_packet(pCodecCtx, &pkt);

        if (ret != 0) {
            break;
        }

        framecnt++;
        FFLOGI("VideoEncoder","Flush Encoder: Succeed to encode 1 frame! %5d size:%5d\n", framecnt, pkt.size);
        fwrite(pkt.data, 1, pkt.size, fp_out);
        av_packet_unref(&pkt);
        if (ret < 0)
            break;
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);

    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}


