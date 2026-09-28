/*
 * Headless AAudio capture test: records through AudioFlinger and the
 * primary HAL and prints the peak and RMS level per 0.5 s.
 * usage: aaudio_rec [seconds] [input_preset]
 *   input_preset: AAudio preset, 1 = generic (default), 6 = unprocessed
 */
#include <stdbool.h>
#include <stdint.h>
#include <aaudio/AAudio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int secs = argc > 1 ? atoi(argv[1]) : 3;
    int preset = argc > 2 ? atoi(argv[2]) : AAUDIO_INPUT_PRESET_GENERIC;
    AAudioStreamBuilder *b;
    AAudioStream *s;
    aaudio_result_t r;

    AAudio_createStreamBuilder(&b);
    AAudioStreamBuilder_setDirection(b, AAUDIO_DIRECTION_INPUT);
    AAudioStreamBuilder_setFormat(b, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(b, 1);
    AAudioStreamBuilder_setSampleRate(b, 48000);
    AAudioStreamBuilder_setInputPreset(b, preset);
    r = AAudioStreamBuilder_openStream(b, &s);
    if (r != AAUDIO_OK) {
        printf("open failed: %s\n", AAudio_convertResultToText(r));
        return 1;
    }
    int rate = AAudioStream_getSampleRate(s);
    printf("opened: rate %d ch %d\n", rate, AAudioStream_getChannelCount(s));
    r = AAudioStream_requestStart(s);
    if (r != AAUDIO_OK) {
        printf("start failed: %s\n", AAudio_convertResultToText(r));
        return 1;
    }
    int chunk = rate / 2;
    int16_t *buf = malloc(sizeof(int16_t) * chunk);
    for (int n = 0; n < secs * 2; n++) {
        int got = 0, peak = 0;
        double sum = 0;
        while (got < chunk) {
            r = AAudioStream_read(s, buf + got, chunk - got, 1000000000LL);
            if (r < 0) {
                printf("read failed: %s\n", AAudio_convertResultToText(r));
                goto out;
            }
            got += r;
        }
        for (int i = 0; i < chunk; i++) {
            int v = abs(buf[i]);
            if (v > peak)
                peak = v;
            sum += (double)buf[i] * buf[i];
        }
        printf("t=%.1fs peak %5d rms %7.1f\n", n / 2.0, peak, sqrt(sum / chunk));
    }
out:
    AAudioStream_requestStop(s);
    AAudioStream_close(s);
    AAudioStreamBuilder_delete(b);
    free(buf);
    return 0;
}
