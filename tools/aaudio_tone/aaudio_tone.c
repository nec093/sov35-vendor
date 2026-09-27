/*
 * Headless AAudio playback test: plays a sine through AudioFlinger and the
 * primary HAL (not tinyalsa directly).
 * usage: aaudio_tone [seconds] [freq] [usage]
 *   usage: AAudio usage constant, 1 = media (default), 2 = voice comm
 */
#include <stdbool.h>
#include <stdint.h>
#include <aaudio/AAudio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int secs = argc > 1 ? atoi(argv[1]) : 3;
    double freq = argc > 2 ? atof(argv[2]) : 1000.0;
    int usage = argc > 3 ? atoi(argv[3]) : AAUDIO_USAGE_MEDIA;
    AAudioStreamBuilder *b;
    AAudioStream *s;
    aaudio_result_t r;

    AAudio_createStreamBuilder(&b);
    AAudioStreamBuilder_setDirection(b, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setFormat(b, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(b, 2);
    AAudioStreamBuilder_setSampleRate(b, 48000);
    AAudioStreamBuilder_setSharingMode(b, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setUsage(b, usage);
    r = AAudioStreamBuilder_openStream(b, &s);
    if (r != AAUDIO_OK) {
        printf("open failed: %s\n", AAudio_convertResultToText(r));
        return 1;
    }
    int rate = AAudioStream_getSampleRate(s);
    printf("opened: rate %d ch %d burst %d\n", rate,
           AAudioStream_getChannelCount(s), AAudioStream_getFramesPerBurst(s));
    r = AAudioStream_requestStart(s);
    if (r != AAUDIO_OK) {
        printf("start failed: %s\n", AAudio_convertResultToText(r));
        return 1;
    }
    int16_t buf[2 * 480];
    double ph = 0, inc = 2 * M_PI * freq / rate;
    long total = (long)secs * rate, done = 0;
    while (done < total) {
        for (int i = 0; i < 480; i++) {
            int16_t v = (int16_t)(sin(ph) * 12000);
            buf[2 * i] = buf[2 * i + 1] = v;
            ph += inc;
        }
        r = AAudioStream_write(s, buf, 480, 1000000000LL);
        if (r < 0) {
            printf("write failed: %s\n", AAudio_convertResultToText(r));
            break;
        }
        done += r;
    }
    printf("written %ld frames, xruns %d, frames read by HAL %lld\n", done,
           AAudioStream_getXRunCount(s),
           (long long)AAudioStream_getFramesRead(s));
    AAudioStream_requestStop(s);
    AAudioStream_close(s);
    AAudioStreamBuilder_delete(b);
    return 0;
}
