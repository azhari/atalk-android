/*
 * Jitsi, the OpenSource Java VoIP and Instant Messaging client.
 *
 * Distributable under LGPL license.
 * See terms of license at gnu.org.
 */
#include "FFmpeg.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libswscale/swscale.h>

#ifndef AV_ERROR_MAX_STRING_SIZE
#define AV_ERROR_MAX_STRING_SIZE 64
#endif

#define DEFINE_AVCODECCONTEXT_F_PROPERTY_SETTER(name, property) \
    JNIEXPORT void JNICALL \
    Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1##name \
        (JNIEnv *env, jclass clazz, jlong avctx, jfloat property) \
    { \
        ((AVCodecContext *) (intptr_t) avctx)->property = (float) property; \
    }

#define DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(name, property) \
    JNIEXPORT void JNICALL \
    Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1##name \
        (JNIEnv *env, jclass clazz, jlong avctx, jint property) \
    { \
        ((AVCodecContext *) (intptr_t) avctx)->property = (int) property; \
    }

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    av_strerror
 * Signature: (I)Ljava.lang.String;
 */
JNIEXPORT jstring JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_av_1strerror
    (JNIEnv *env, jclass clazz, jint errnum)
{
    char str[AV_ERROR_MAX_STRING_SIZE];
    av_strerror((int)errnum, str, AV_ERROR_MAX_STRING_SIZE);
    return (*env)->NewStringUTF(env, str);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    av_get_pix_fmt
 * Signature: (Ljava.lang.String;)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_av_1get_1pix_1fmt
    (JNIEnv *env, jclass clazz, jstring name)
{
    const char *cname = (*env)->GetStringUTFChars(env, name, 0);
    enum AVPixelFormat pix_fmt = av_get_pix_fmt(cname);
    (*env)->ReleaseStringUTFChars(env, name, cname);
    return (jint)pix_fmt;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    av_free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_av_1free
    (JNIEnv *env, jclass clazz, jlong ptr)
{
    av_free((void *) (intptr_t) ptr);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    av_malloc
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_av_1malloc
    (JNIEnv *env, jclass clazz, jint size)
{
    return (jlong) (intptr_t) av_malloc((unsigned int) size);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_alloc_context3
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1alloc_1context3
    (JNIEnv *env, jclass clazz, jlong codec)
{
    return (jlong) (intptr_t) avcodec_alloc_context3((const AVCodec *) (intptr_t) codec);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_alloc_frame
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1alloc_1frame
    (JNIEnv *env, jclass clazz)
{
    return (jlong) (intptr_t) av_frame_alloc();
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_alloc_packet
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1alloc_1packet
    (JNIEnv *env, jclass clazz, jint size)
{
    AVPacket *pkt = av_malloc(sizeof(AVPacket));

    if (pkt) {
        if (av_new_packet(pkt, size)) {
            av_free(pkt);
            pkt = 0;
        }
    }
    return (jlong) (intptr_t) pkt;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_close
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1close
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    return (jint) avcodec_close((AVCodecContext *) (intptr_t) avctx);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_decode_audio4
 * Signature: (JJ[ZJ)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1decode_1audio4
    (JNIEnv *env, jclass clazz, jlong avctx, jlong avframe, jbooleanArray got_frame, jlong avpkt)
{
    jint ret;
    AVPacket *pkt;

    pkt = (AVPacket *) (intptr_t) avpkt;
    ret = avcodec_send_packet((AVCodecContext *) (intptr_t) avctx, pkt);
    if (ret == 0) {
        ret = avcodec_receive_frame((AVCodecContext *) (intptr_t) avctx, (AVFrame *) (intptr_t) avframe);
        jboolean b = (ret == 0) ? JNI_TRUE : JNI_FALSE;
        (*env)->SetBooleanArrayRegion(env, got_frame, 0, 1, &b);
        if (ret == 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ret = pkt->size;
        }
    }
    return ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_decode_video
 * Signature: (JJ[Z[BI)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1decode_1video__JJ_3Z_3BI
    (JNIEnv *env, jclass clazz, jlong avctx, jlong avframe,
	jbooleanArray got_picture, jbyteArray buf, jint buf_size)
{
    jint ret;

    if (buf) {
        jbyte *buf_ = (*env)->GetByteArrayElements (env, buf, NULL);
        if (buf_) {
            AVPacket *avpkt = av_packet_alloc();
            avpkt->data = (uint8_t *) buf_;
            avpkt->size = (int) buf_size;

            ret = avcodec_send_packet((AVCodecContext *) (intptr_t) avctx, avpkt);
            if (ret == 0) {
                ret = avcodec_receive_frame((AVCodecContext *) (intptr_t) avctx, (AVFrame *) (intptr_t) avframe);
                jboolean j_got_picture = (ret == 0) ? JNI_TRUE : JNI_FALSE;
                (*env)->SetBooleanArrayRegion (env, got_picture, 0, 1, &j_got_picture);
                if (ret == 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                    ret = avpkt->size;
                }
            }
            (*env)->ReleaseByteArrayElements (env, buf, buf_, 0);
        } else
            ret = -1;
    } else
        ret = -1;
    return ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_decode_video
 * Signature: (JJJI)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1decode_1video__JJJI
    (JNIEnv *env, jclass clazz, jlong avctx, jlong avframe, jlong src, jint src_length)
{
    int got_picture = 0;
    int ret;

    AVPacket *avpkt = av_packet_alloc();
    avpkt->data = (uint8_t*) (intptr_t) src;
    avpkt->size = (int) src_length;

    ret = avcodec_send_packet((AVCodecContext *) (intptr_t) avctx, avpkt);
    if (ret == 0) {
        ret = avcodec_receive_frame((AVCodecContext *) (intptr_t) avctx, (AVFrame *) (intptr_t) avframe);
        if (ret == 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ret = avpkt->size;
        }
    }
    return ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_encode_audio
 * Signature: (J[BII[BI)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1encode_1audio
    (JNIEnv *env, jclass clazz, jlong ctx, jbyteArray buf, jint buf_offset,
	jint buf_size, jbyteArray samples, jint samples_offset)
{
    jint ret = -1;
    jint samples_size;
    jbyte *buf_ = NULL;
    jbyte *samples_ = NULL;
    AVFrame *frame = NULL;
    AVCodecContext *avctx = NULL;
    int got_output = 0;
    int pkt_size;
    AVPacket *avpkt = av_packet_alloc();

    // validate parameters
    if (!buf || !ctx) {
        goto end;
    }

    // convert java objects to pointers
    avctx = (AVCodecContext*)(intptr_t) ctx;
    buf_ = (*env)->GetByteArrayElements(env, buf, NULL);
    if (!buf_) {
        goto end;
    }

    samples_ = (*env)->GetByteArrayElements(env, samples, NULL);
    if (!samples_) {
        goto end;
    }
    samples_size = (*env)->GetArrayLength(env, samples);

    // prepare encoder input
    frame = av_frame_alloc();
    if (!frame) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    frame->nb_samples = avctx->frame_size;
    frame->format = avctx->sample_fmt;
    frame->channel_layout = avctx->channel_layout;

    ret = avcodec_fill_audio_frame(frame, avctx->channels, avctx->sample_fmt,
        (uint8_t*) (samples_ + samples_offset), samples_size - samples_offset, 0);
    if (ret < 0) {
        goto end;
    }

    avpkt->data = (uint8_t*)(buf_ + buf_offset);
    avpkt->size = buf_size;

    ret = avcodec_send_frame((AVCodecContext *) (intptr_t) avctx, (const AVFrame *) (intptr_t) frame);
    if (ret == 0) {
        ret = avcodec_receive_packet((AVCodecContext *) (intptr_t) avctx, avpkt);
        memcpy((buf_ + buf_offset), avpkt->data, avpkt->size);
    }
    ret = (ret == 0) ? avpkt->size : 0;
end:
    if (buf_) {
        (*env)->ReleaseByteArrayElements(env, buf, buf_, 0);
    }
    if (samples_) {
        (*env)->ReleaseByteArrayElements(env, samples, samples_, JNI_ABORT);
    }

    av_frame_free(&frame);
    av_packet_unref(avpkt);
    return (jint) ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_encode_video
 * Signature: (J[BIJ)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1encode_1video
    (JNIEnv *env, jclass clazz, jlong avctx, jbyteArray buf, jint buf_size, jlong frame)
{
    jint ret;
    if (buf) {
        jbyte *buf_ = (*env)->GetByteArrayElements (env, buf, NULL);
        if (buf_) {
            int pkt_size;
            AVPacket *avpkt = av_packet_alloc();

            ret = avcodec_send_frame((AVCodecContext *) (intptr_t) avctx, (const AVFrame *) (intptr_t) frame);
            while (ret >= 0) {
                ret = avcodec_receive_packet((AVCodecContext *) (intptr_t) avctx, avpkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    (*env)->ReleaseByteArrayElements (env, buf, buf_, 0);
                    return pkt_size;
                }
                else if (ret < 0)
                    return -1;

                memcpy(buf_, avpkt->data, avpkt->size);
                pkt_size = (ret == 0) ? avpkt->size : 0;
                av_packet_unref(avpkt);
            }

        } else
            ret = -1;
    } else {
        ret = -1;
	}
    return ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_find_decoder
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1find_1decoder
    (JNIEnv *env, jclass clazz, jint id)
{
    return (jlong) (intptr_t) avcodec_find_decoder((enum AVCodecID) id);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_find_encoder
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1find_1encoder
    (JNIEnv *env, jclass clazz, jint id)
{
    return (jlong) (intptr_t) avcodec_find_encoder((enum AVCodecID) id);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_free_packet
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1free_1packet
    (JNIEnv *env, jclass clazz, jlong pkt)
{
    if (pkt) {
        AVPacket *pkt_ = (AVPacket *) (intptr_t) pkt;

        av_packet_unref(pkt_);
        av_free(pkt_);
    }
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodec_open2
 * Signature: (JJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodec_1open2
    (JNIEnv *env, jclass clazz, jlong avctx, jlong codec, jobjectArray options)
{
    AVDictionary *options_ = NULL;
    int ret = 0;

    if (options) {
        jsize length = (*env)->GetArrayLength(env, options);

        if (length) {
            if (length % 2) {
                ret = AVERROR(EINVAL);
			}
            else {
                jsize i = 0;

                while ((0 <= ret) && (i < length)) {
                    jstring key = (jstring) (*env)->GetObjectArrayElement(env, options, i++);
                    const char *key_;

                    if (key) {
                        key_ = (*env)->GetStringUTFChars(env, key, NULL);
                        if (!key_)
                            ret = AVERROR(ENOMEM);
                    }
                    else {
                        key_ = NULL;
					}
                    if (0 <= ret) {
                        jstring value = (jstring) (*env)->GetObjectArrayElement(env, options, i++);
                        const char *value_;

                        if (value) {
                            value_ = (*env)->GetStringUTFChars(env, value, NULL);
                            if (!value_)
                                ret = AVERROR(ENOMEM);
                        }
                        else {
                            value_ = NULL;
						}
                        if (0 <= ret) {
                            ret = av_dict_set(&options_, key_, value_, 0);
                            (*env)->ReleaseStringUTFChars(env, value, value_);
                        }
                        (*env)->ReleaseStringUTFChars(env, key, key_);
                    }
                }
            }
        }
    }
    if (0 <= ret) {
        ret = avcodec_open2((AVCodecContext *) (intptr_t) avctx,
                    (AVCodec *) (intptr_t) codec, &options_);
    }
    if (options_)
        av_dict_free(&options_);
    return ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_add_flags
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1add_1flags
    (JNIEnv *env, jclass clazz, jlong avctx, jint flags)
{
    ((AVCodecContext *) (intptr_t) avctx)->flags |= (int) flags;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_add_flags2
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1add_1flags2
    (JNIEnv *env, jclass clazz, jlong avctx, jint flags2)
{
    ((AVCodecContext *) (intptr_t) avctx)->flags2 |= (int) flags2;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_get_frame_size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1get_1frame_1size
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    return (jint) (((AVCodecContext *) (intptr_t) avctx)->frame_size);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_get_height
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1get_1height
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    return (jint) (((AVCodecContext *) (intptr_t) avctx)->height);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_get_pix_fmt
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1get_1pix_1fmt
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    return (jint) (((AVCodecContext *) (intptr_t) avctx)->pix_fmt);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_get_width
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1get_1width
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    return (jint) (((AVCodecContext *) (intptr_t) avctx)->width);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_b_frame_strategy
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1b_1frame_1strategy
    (JNIEnv *env, jclass clazz, jlong avctx, jint b_frame_strategy)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set_int(avctx_->priv_data, "b_frame_strategy", (int) b_frame_strategy, AV_OPT_SEARCH_CHILDREN);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_bit_rate
 * Method:    avcodeccontext_set_bit_rate_tolerance
 * Method:    avcodeccontext_set_channels
 * Method:    avcodeccontext_set_channel_layout
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(bit_1rate, bit_rate)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(bit_1rate_1tolerance, bit_rate_tolerance)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(channels, channels)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(channel_1layout, channel_layout)

JNIEXPORT void JNICALL
/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_chromaoffset
 * Signature: (JI)V
 */
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1chromaoffset
    (JNIEnv *env, jclass clazz, jlong avctx, jint chromaoffset)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set_int(avctx_->priv_data, "chromaoffset", (int) chromaoffset, AV_OPT_SEARCH_CHILDREN);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_gop_size
 * Method:    avcodeccontext_set_i_quant_factor
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(gop_1size, gop_size)
DEFINE_AVCODECCONTEXT_F_PROPERTY_SETTER(i_1quant_1factor, i_quant_factor)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_keyint_min
 * Method:    avcodeccontext_set_max_b_frames
 * Method:    avcodeccontext_set_mb_decision
 * Method:    avcodeccontext_set_me_cmp
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(keyint_1min, keyint_min)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(max_1b_1frames, max_b_frames)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(mb_1decision, mb_decision)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(me_1cmp, me_cmp)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_me_method
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1me_1method
    (JNIEnv *env, jclass clazz, jlong avctx, jint me_method)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set_int(avctx_->priv_data, "me_method", (int) me_method, AV_OPT_SEARCH_CHILDREN);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_me_range
 * Method:    avcodeccontext_set_me_subpel_quality
 * Method:    avcodeccontext_set_pix_fmt
 * Method:    avcodeccontext_set_profile
 * Method:    avcodeccontext_set_qcompress
 * Signature: (JF)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(me_1range, me_range)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(me_1subpel_1quality, me_subpel_quality)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(pix_1fmt, pix_fmt)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(profile, profile)

DEFINE_AVCODECCONTEXT_F_PROPERTY_SETTER(qcompress, qcompress)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_quantizer
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1quantizer
    (JNIEnv *env, jclass clazz, jlong avctx, jint qmin, jint qmax, jint max_qdiff)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;

    avctx_->qmin = (int) qmin;
    avctx_->qmax = (int) qmax;
    avctx_->max_qdiff = (int) max_qdiff;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_rc_buffer_size
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(rc_1buffer_1size, rc_buffer_size)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_rc_eq
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1rc_1eq
    (JNIEnv *env, jclass clazz, jlong avctx, jstring rc_eq)
{
    char *s;
    if (rc_eq) {
        const char *js = (*env)->GetStringUTFChars(env, rc_eq, NULL);

        if (js) {
            s = av_strdup(js);
            (*env)->ReleaseStringUTFChars(env, rc_eq, js);
        } else
            s = NULL;
    } else
        s = NULL;

    // ((AVCodecContext *) (intptr_t) avctx)->rc_eq = s;
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set(avctx_->priv_data, "rc_eq", s, AV_OPT_SEARCH_CHILDREN);
}

DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(rc_1max_1rate, rc_max_rate)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(refs, refs)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_rtp_payload_size
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1rtp_1payload_1size
    (JNIEnv *env, jclass clazz, jlong avctx, jint rtp_payload_size)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set_int(avctx_->priv_data, "rtp_payload_size", (int) rtp_payload_size, AV_OPT_SEARCH_CHILDREN);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_sample_aspect_ratio
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1sample_1aspect_1ratio
    (JNIEnv *env, jclass clazz, jlong avctx, jint num, jint den)
{
    AVRational *sample_aspect_ratio
        = &(((AVCodecContext *) (intptr_t) avctx)->sample_aspect_ratio);

    // sample_aspect_ratio->num = (int) num;
    // sample_aspect_ratio->den = (int) den;
    *sample_aspect_ratio = (AVRational){num, den};
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_sample_fmt
 * Method:    avcodeccontext_set_sample_rate
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(sample_1fmt, sample_fmt)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(sample_1rate, sample_rate)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_scenechange_threshold
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1scenechange_1threshold
    (JNIEnv *env, jclass clazz, jlong avctx, jint scenechange_threshold)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    av_opt_set_int(avctx_->priv_data, "scenechange_threshold", (int) scenechange_threshold, AV_OPT_SEARCH_CHILDREN);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_size
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1size
    (JNIEnv *env, jclass clazz, jlong avctx, jint width, jint height)
{
    AVCodecContext *avctx_ = (AVCodecContext *) (intptr_t) avctx;
    avctx_->width = (int) width;
    avctx_->height = (int) height;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_thread_count
 * Method:    avcodeccontext_set_ticks_per_frame
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(thread_1count, thread_count)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(ticks_1per_1frame, ticks_per_frame)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_time_base
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avcodeccontext_1set_1time_1base
    (JNIEnv *env, jclass clazz, jlong avctx, jint num, jint den)
{
    // time_base->num = (int) num;
    // time_base->den = (int) den;

    AVRational *time_base = &(((AVCodecContext *) (intptr_t) avctx)->time_base);
    AVRational *framerate = &(((AVCodecContext *) (intptr_t) avctx)->framerate);
    *time_base = (AVRational){num, den};
    *framerate = (AVRational){den, num};
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avcodeccontext_set_trellis
 * Method:    avcodeccontext_set_workaround_bugs
 * Signature: (JI)V
 */
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(trellis, trellis)
DEFINE_AVCODECCONTEXT_I_PROPERTY_SETTER(workaround_1bugs, workaround_bugs)

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avfilter_graph_alloc
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avfilter_1graph_1alloc
    (JNIEnv *env, jclass clazz)
{
    AVFilterGraph* graph = avfilter_graph_alloc();
    if (graph) {
        avfilter_graph_set_auto_convert(graph, -1);
    }

    return (jlong) (intptr_t) graph;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avfilter_graph_config
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avfilter_1graph_1config
    (JNIEnv *env, jclass clazz, jlong graph, jlong log_ctx)
{
    return
        (jint)
            avfilter_graph_config(
                    (AVFilterGraph *) (intptr_t) graph,
                    (AVClass *) (intptr_t) log_ctx);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avfilter_graph_free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avfilter_1graph_1free
    (JNIEnv *env, jclass clazz, jlong graph)
{
    AVFilterGraph *graph_ = (AVFilterGraph *) (intptr_t) graph;

    avfilter_graph_free(&graph_);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avfilter_graph_get_filter
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avfilter_1graph_1get_1filter
    (JNIEnv *env, jclass clazz, jlong graph, jstring name)
{
    const char *name_ = (*env)->GetStringUTFChars(env, name, NULL);
    AVFilterContext *filter;

    if (name_) {
        filter = avfilter_graph_get_filter(
                    (AVFilterGraph *) (intptr_t) graph, (char *) name_);
        (*env)->ReleaseStringUTFChars(env, name, name_);
    }
    else
        filter = NULL;
    return (jlong) (intptr_t) filter;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avfilter_graph_parse
 * Signature: (JLjava/lang/String;JJJ)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avfilter_1graph_1parse
    (JNIEnv *env, jclass clazz, jlong graph, jstring filters, jlong inputs,
	jlong outputs, jlong log_ctx)
{
    const char *filters_ = (*env)->GetStringUTFChars(env, filters, NULL);
    int ret;

    if (filters_) {
        AVFilterGraph *graph_ = (AVFilterGraph *) (intptr_t) graph;

		ret = avfilter_graph_parse_ptr(
					graph_, filters_,
                    (AVFilterInOut **) (intptr_t) inputs,
                    (AVFilterInOut **) (intptr_t) outputs,
                    (AVClass *) (intptr_t) log_ctx);

        (*env)->ReleaseStringUTFChars(env, filters, filters_);
    }
    else
        ret = AVERROR(ENOMEM);
    return (jint) ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_get_data0
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1get_1data0
    (JNIEnv *env, jclass clazz, jlong frame)
{
    return (jlong) (intptr_t) (((AVFrame *) (intptr_t) frame)->data[0]);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_get_linesize0
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1get_1linesize0
    (JNIEnv *env, jclass clazz, jlong frame)
{
    return ((AVFrame *) (intptr_t) frame)->linesize[0];
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_get_pts
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1get_1pts
    (JNIEnv *env, jclass clazz, jlong frame)
{
    return (jlong) (((AVFrame *) (intptr_t) frame)->pts);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_set_properties
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1set_1properties
    (JNIEnv *env, jclass clazz, jlong frame, jint format, jint frame_width, jint frame_height)
{
    AVFrame *frame_ = (AVFrame *) (intptr_t) frame;
    frame_->format = format;
    frame_->width  = frame_width;
    frame_->height = frame_height;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_set_data
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1set_1data
    (JNIEnv *env, jclass clazz, jlong frame, jlong data0, jlong offset1, jlong offset2)
{
    AVFrame *frame_ = (AVFrame *) (intptr_t) frame;

    frame_->data[0] = (uint8_t *) (intptr_t) data0;
    frame_->data[1] = frame_->data[0] + offset1;
    frame_->data[2] = frame_->data[1] + offset2;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_set_key_frame
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1set_1key_1frame
    (JNIEnv *env, jclass clazz, jlong frame, jboolean key_frame)
{
    AVFrame *frame_ = (AVFrame *) (intptr_t) frame;

    if (JNI_TRUE == key_frame) {
        frame_->key_frame = 1;
        frame_->pict_type = AV_PICTURE_TYPE_I;
    }
    else {
        frame_->key_frame = 0;
        frame_->pict_type = 0;
    }
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avframe_set_linesize
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avframe_1set_1linesize
    (JNIEnv *env, jclass clazz, jlong frame, jint linesize0, jint linesize1, jint linesize2)
{
    AVFrame *frame_ = (AVFrame *) (intptr_t) frame;

    frame_->linesize[0] = (int) linesize0;
    frame_->linesize[1] = (int) linesize1;
    frame_->linesize[2] = (int) linesize2;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avpacket_set_data
 * Signature: (J[BII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avpacket_1set_1data
    (JNIEnv *env, jclass clazz, jlong pkt, jbyteArray data, jint offset,
        jint length)
{
    jbyte *data_;
    jboolean ok;

    if (data) {
        data_ = (*env)->GetPrimitiveArrayCritical(env, data, NULL);
        ok = data_ ? JNI_TRUE : JNI_FALSE;
    }
    else {
        data_ = NULL;
        ok = JNI_TRUE;
    }
    if (JNI_TRUE == ok) {
        int delta;
        AVPacket *pkt_ = (AVPacket *) (intptr_t) pkt;

        delta = length - pkt_->size;
        if (delta > 0) {
            if (av_grow_packet(pkt_, delta) != 0)
                ok = JNI_FALSE;
        }
        else if (delta < 0) {
            av_shrink_packet(pkt_, length);
        }
        if (JNI_TRUE == ok)
            memcpy(pkt_->data, data_ + offset, length);
        if (data_)
            (*env)->ReleasePrimitiveArrayCritical(env, data, data_, JNI_ABORT);
    }
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    avpicture_fill
 * Signature: (JJIII)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_avpicture_1fill
    (JNIEnv *env, jclass clazz, jlong frame, jlong ptr, jint pix_fmt, jint width, jint height)
{
    AVFrame *avframe = (AVFrame *) (intptr_t) frame;
    return (jint) av_image_fill_arrays(avframe->data, avframe->linesize,
            (uint8_t *) (intptr_t) ptr, (int) pix_fmt, (int) width, (int) height, 1);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    get_filtered_video_frame
 * Signature: (JIIIJJJ)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_get_1filtered_1video_1frame
    (JNIEnv *env, jclass clazz, jlong input, jint width, jint height,
	jint pixFmt, jlong buffer, jlong ffsink, jlong output)
{
    AVFrame *input_ = (AVFrame *) (intptr_t) input;
    AVFilterContext *buffer_ = (AVFilterContext *) (intptr_t) buffer;
    int result;

    input_->width = width;
    input_->height = height;
    input_->format = pixFmt;
    result = av_buffersrc_write_frame(buffer_, input_);
    if (result != 0) {
        return (jlong)result;
    }

    AVFilterContext *ffsink_ = (AVFilterContext *) (intptr_t) ffsink;
    AVFrame *output_ = (AVFrame *)(intptr_t)output;
    result = av_buffersink_get_frame(ffsink_, output_);
    return (jlong)result;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    memcpy
 * Signature: ([BIIJ)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_memcpy___3BIIJ
    (JNIEnv *env, jclass clazz, jbyteArray dst, jint dst_offset,
        jint dst_length, jlong src)
{
    (*env)->SetByteArrayRegion(env, dst, dst_offset, dst_length, (jbyte *) (intptr_t) src);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    memcpy
 * Signature: ([IIIJ)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_memcpy___3IIIJ
    (JNIEnv *env, jclass clazz, jintArray dst, jint dst_offset, jint dst_length, jlong src)
{
    (*env)->SetIntArrayRegion(env, dst, dst_offset, dst_length, (jint *) (intptr_t) src);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    memcpy
 * Signature: (J[BII)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_memcpy__J_3BII
    (JNIEnv *env, jclass clazz, jlong dst, jbyteArray src, jint src_offset, jint src_length)
{
    (*env)->GetByteArrayRegion(env, src, src_offset, src_length, (jbyte *) (intptr_t) dst);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    PIX_FMT_BGR32
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_PIX_1FMT_1BGR32
    (JNIEnv *env, jclass clazz)
{
    return AV_PIX_FMT_BGR32;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    PIX_FMT_BGR32_1
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_PIX_1FMT_1BGR32_11
    (JNIEnv *env, jclass clazz)
{
    return AV_PIX_FMT_BGR32_1;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    PIX_FMT_RGB24
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_PIX_1FMT_1RGB24
    (JNIEnv *env, jclass clazz)
{
    uint32_t test = 1;
    int little_endian = *((uint8_t*) &test);

    return little_endian ? AV_PIX_FMT_BGR24 : AV_PIX_FMT_RGB24;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    PIX_FMT_RGB32
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_PIX_1FMT_1RGB32
    (JNIEnv *env, jclass clazz)
{
    return AV_PIX_FMT_RGB32;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    PIX_FMT_RGB32_1
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_PIX_1FMT_1RGB32_11
    (JNIEnv *env, jclass clazz)
{
    return AV_PIX_FMT_RGB32_1;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    sws_freeContext
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_sws_1freeContext
    (JNIEnv *env, jclass clazz, jlong avctx)
{
    sws_freeContext((struct SwsContext *) (intptr_t) avctx);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    sws_getCachedContext
 * Signature: (JIIIIIII)J
 */
JNIEXPORT jlong JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_sws_1getCachedContext
    (JNIEnv *env, jclass clazz, jlong avctx, jint srcW, jint srcH,
        jint srcFormat, jint dstW, jint dstH, jint dstFormat, jint flags)
{
    return (jlong) (intptr_t)
        sws_getCachedContext(
            (struct SwsContext *) (intptr_t) avctx,
            (int) srcW, (int) srcH, (enum AVPixelFormat) srcFormat,
            (int) dstW, (int) dstH, (enum AVPixelFormat) dstFormat,
            (int) flags, NULL, NULL, NULL);
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    sws_scale
 * Signature: (JJIILjava/lang/Object;III)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_sws_1scale__JJIILjava_lang_Object_2III
    (JNIEnv *env, jclass clazz, jlong avctx, jlong src, jint srcSliceY,
        jint srcSliceH, jobject dst, jint dstFormat, jint dstW, jint dstH)
{
    AVFrame *srcFrame;
    uint8_t *dst_;
    int ret;

    srcFrame = (AVFrame *) (intptr_t) src;
    dst_ = (*env)->GetPrimitiveArrayCritical(env, dst, NULL);
    if (dst_) {
        AVFrame dstFrame;

        /* Turn the bytes into an AVFrame */
        av_image_fill_arrays(dstFrame.data, dstFrame.linesize,
            dst_, (int) dstFormat, (int) dstW, (int) dstH, 1);

        ret = sws_scale((struct SwsContext *) (intptr_t) avctx,
                (const uint8_t * const *) srcFrame->data, (int *) srcFrame->linesize,
                (int) srcSliceY, (int) srcSliceH,
                (uint8_t **) dstFrame.data, (int *) dstFrame.linesize);
        (*env)->ReleasePrimitiveArrayCritical(env, dst, dst_, 0);
    }
    else {
        ret = -1;
    }
    return (jint) ret;
}

/*
 * Class:     org_atalk_impl_neomedia_codec_FFmpeg
 * Method:    sws_scale
 * Signature: (JLjava/lang/Object;IIIIILjava/lang/Object;III)I
 */
JNIEXPORT jint JNICALL
Java_org_atalk_impl_neomedia_codec_FFmpeg_sws_1scale__JLjava_lang_Object_2IIIIILjava_lang_Object_2III
    (JNIEnv *env, jclass class, jlong avctx, jobject src, jint srcFormat,
	    jint srcW, jint srcH, jint srcSliceY, jint srcSliceH, jobject dst,
		jint dstFormat, jint dstW, jint dstH)
{
    uint8_t *src_;
    jint ret;

    src_ = (*env)->GetPrimitiveArrayCritical(env, src, NULL);
    if (src_) {
        AVFrame srcFrame;
        av_image_fill_arrays(srcFrame.data, srcFrame.linesize,
            src_, (int) srcFormat, (int) srcW, (int) srcH, 1);

        ret = Java_org_atalk_impl_neomedia_codec_FFmpeg_sws_1scale__JJIILjava_lang_Object_2III(
                env, class, avctx, (jlong) (intptr_t) &srcFrame, srcSliceY, srcSliceH,
                dst, dstFormat, dstW, dstH);
        (*env)->ReleasePrimitiveArrayCritical(env, src, src_, 0);
    }
    else {
        ret = -1;
    }
    return ret;
}
