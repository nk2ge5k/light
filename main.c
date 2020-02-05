#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const char *MAX_BRIGHTNESS_FILE = "/sys/class/backlight/intel_backlight/max_brightness";
const char *CURRENT_BRIGHTNESS_FILE = "/sys/class/backlight/intel_backlight/brightness";

// readInt returns integer written in the file or -1 in case of failure
int readInt(const char *file_path) {
    FILE *fd;
    char buf[255];
    int val = -1;

    if (!(fd = fopen(file_path, "r"))) {
        fprintf(stderr, "could not open %s for reading: %s\n", file_path, strerror(errno));
        return -1;
    }

    fread(buf, 255, 1, fd);
    val = atoi(buf);

    if (fclose(fd) != 0) {
        fprintf(stderr, "could not close %s: %s\n", file_path, strerror(errno));
        return -1;
    }

    return val;
}

// writeInt writes integer into the file, returns 1 in case of success and 0 in case of failure
int writeInt(const char *file_path, int val) {
    FILE *fd;

    if (!(fd = fopen(file_path, "w"))) {
        fprintf(stderr, "could not open %s for writing: %s\n", file_path, strerror(errno));
        return 0;
    }

    fprintf(fd, "%d", val);

    if (fclose(fd) != 0) {
        fprintf(stderr, "could not close %s: %s\n", file_path, strerror(errno));
        return 0;
    }

    return 1;
}


int isOption(char *arg) { return arg[0] == '-'; }

int changeBrightness(short by_percent, unsigned int ms) {

    double current;
    double max;

    if ((current = readInt(CURRENT_BRIGHTNESS_FILE)) == -1) return 0;
    if ((max = readInt(MAX_BRIGHTNESS_FILE)) == -1)         return 0;

    double percent = ((current/max) * 100.0) + by_percent;
    int    result  = percent * (max/100);

    if (result < 0)   result = 0;
    if (result > max) result = max;

    writeInt(CURRENT_BRIGHTNESS_FILE, result);

    return 1;
}

// TODO:
void usage(char *cmd) {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "\t%s [OPTIONS] [value]\n", cmd);
    fprintf(stderr, "\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "\t-t, --time\tTime in milliseconds to to change brightness to the given value\n");
    /* fprintf(stderr, "\t-V, --verion\tPrint version info and exit\n"); */
    fprintf(stderr, "\t-h, --help\tShow this help message\n");
    fprintf(stderr, "\n");

    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: not enough arguments\n\n", argv[0]);
        usage(argv[0]);
    }

    unsigned int ms      = 0; // time to fade into given brightness
    short        percent = 0; // brightness percent to set

    int i = 1;
    for (; i < argc; i += 2) {
        if (!isOption(argv[i]) || i + 1 == argc) break;

        if (strcmp("-h", argv[i]) == 0 ||
            strcmp("--help", argv[i]) == 0) usage(argv[0]);

        if (strcmp("-t", argv[i]) == 0 ||
            strcmp("--time", argv[i]) == 0) {

            if (argc-1 == i) {
                fprintf(stderr, "%s: missing value for option \"%s\"\n\n", argv[0], argv[i]);
                usage(argv[0]);
            }

            int val = atoi(argv[i+1]);
            ms = (val < 0) ? val * -1 : val;

            continue;
        }

        fprintf(stderr, "%s: unknown option \"%s\"\n\n", argv[0], argv[i]);
        usage(argv[0]);
    }

    if (i == argc) {
        fprintf(stderr, "%s: missing brightness value\n\n", argv[0]);
        usage(argv[0]);
    }

    if (strcasecmp(argv[i], "max") == 0) {
        percent = 100;
    } else {
        percent = atoi(argv[i]); // TODO: check for values > 255
    }

    if (!changeBrightness(percent, ms)) {
        return 1;
    }

    return 0;
}
