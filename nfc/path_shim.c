/*
 * Path shim for Sony's legacy CXD224X NFC module.
 *
 * nfc_nci.cxd224x.msm8996.so builds its config paths as "/etc/" + name
 * (libnfc-nci.conf, libnfc-cxd-224xNNNNNNNN.conf). /etc is the system
 * image, which a Treble vendor cannot ship files into, so redirect those
 * lookups to /vendor/etc. Preloaded into the NFC HAL service only.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FROM "/etc/libnfc-"
#define TO "/vendor/etc/libnfc-"

static const char *redirect(const char *path, char *buf, size_t len)
{
    if (path && !strncmp(path, FROM, sizeof(FROM) - 1)) {
        snprintf(buf, len, TO "%s", path + sizeof(FROM) - 1);
        return buf;
    }
    return path;
}

FILE *fopen(const char *path, const char *mode)
{
    static FILE *(*real)(const char *, const char *);
    char buf[256];

    if (!real)
        real = dlsym(RTLD_NEXT, "fopen");
    return real(redirect(path, buf, sizeof(buf)), mode);
}

int open(const char *path, int flags, ...)
{
    static int (*real)(const char *, int, ...);
    char buf[256];
    mode_t mode = 0;

    if (!real)
        real = dlsym(RTLD_NEXT, "open");
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;

        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return real(redirect(path, buf, sizeof(buf)), flags, mode);
}

int access(const char *path, int amode)
{
    static int (*real)(const char *, int);
    char buf[256];

    if (!real)
        real = dlsym(RTLD_NEXT, "access");
    return real(redirect(path, buf, sizeof(buf)), amode);
}
