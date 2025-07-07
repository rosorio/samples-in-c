#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>

typedef enum {
    NONE  = 0,
    BIRTH = 1,
    ACCESS= 2,
    MODIF = 4
} enum_file_changes;


void usage(char *binname)
{
    printf("%s -f <filename> [-b <time>] [-m <time>] [-a <time>]\n"
           " -b set the file birth time (if supported by the filesystem)\n"
           " -m set the file modification time\n"
           " -a set the file access time\n"
           " Time is the desired date and time in RFC3339 format\n"
           " eg. \"2011-07-12T11:03:12+02:00\"\n\n", binname);
}

/* convert the iso format as the RFC3339 is a subset of it */
time_t
iso8601_to_time_t(char *s)
{
  struct tm date;
  time_t t;
  errno = 0;
  char *pos = strptime(s, "%Y-%m-%dT%H:%M:%S.%fZ", &date);
  if (pos == NULL) {
    /* Modify the last HH:MM to HHMM if necessary */
    if (s[strlen(s) - 3] == ':' ) {
      s[strlen(s) - 3] = s[strlen(s) - 2];
      s[strlen(s) - 2] = s[strlen(s) - 1];
      s[strlen(s) - 1] = '\0';
    }
    pos =strptime(s, "%Y-%m-%dT%H:%M:%S%z", &date);
  }
  if (pos == NULL) {
    errno = EINVAL;
    warn("Convert  ISO8601 '%s' to struct tm failed", s);
    return 0;
  }
  t = mktime(&date);
  if (t == (time_t)-1) {
    errno = EINVAL;
    warn("Convert struct tm (from '%s') to time_t failed", s);
    return 0;
  }
  return t;
}

int main(int argc, char * argv[])
{
    int ch;
    time_t birth = 0,
          access = 0,
    modification = 0;
    enum_file_changes what_to_change = NONE;
    char * file = NULL;
    struct timespec times[2];
    int ret;
    char * binname = basename(argv[0]);

    while ((ch = getopt(argc, argv, "b:a:m:f:h")) != -1) {
        switch (ch) {
        case 'b':
            what_to_change |= BIRTH;
            birth = iso8601_to_time_t(optarg);
            if (birth == 0) {
                errno = EINVAL;
                warn("Error: Invalid birth time");
                usage(binname);
                exit(1);
            }
            break;
        case 'a':
            what_to_change |= ACCESS;
            access = iso8601_to_time_t(optarg);
            if (access == 0) {
                errno = EINVAL;
                warn("Error: Invalid access time");
                usage(binname);
                exit(1);
            }

            break;
        case 'm':
            what_to_change |= MODIF;
            modification = iso8601_to_time_t(optarg);
            if (modification == 0) {
                errno = EINVAL;
                warn("Error: Invalid modification time");
                usage(binname);
                exit(1);
            }
            break;
        case 'f':
            file = optarg;
            break;
        case 'h':
            usage(binname);
            exit(0);
            break;
        default:
            usage(binname);
            exit(1);
            break;
        }
    }

    if (file == NULL) {
        errno = EINVAL;
        warn("Error: No file provided");
        exit(1);
    } else {
        struct stat fileinfo;
        if (stat(file, &fileinfo) != 0) {
            errno = EINVAL;
            warn("Error: Invalid file path");
            exit(1);
        }
        if (!S_ISREG(fileinfo.st_mode) && !S_ISDIR(fileinfo.st_mode)) {
            errno = EINVAL;
            warn("Error: Invalid file type");
            exit(1);
        }
    }

    if (what_to_change == NONE) {
        errno = EINVAL;
        warn("Error: We must change something");
        exit(1);
    }

    if (what_to_change & BIRTH) {
        memset(times,0, sizeof(times));
        times[0].tv_sec = birth;
        times[1].tv_sec = birth;
        if (utimensat(0, file, times, AT_EMPTY_PATH) != 0) {
            warn("utimensat: %d", ret);
            exit(1);
        }
    }

    printf("Set BIRTH %ld %s", birth, ctime(&birth));
    if (what_to_change & (ACCESS|MODIF))
    {
        memset(times,0, sizeof(times));
        if (what_to_change & ACCESS) {
            printf("Set ACCES %ld %s", access, ctime(&access));
            times[0].tv_sec = access;
        } else {
            times[0].tv_nsec = UTIME_OMIT;
        }

        if (what_to_change & MODIF) {
            printf("Set MODIF %ld %s", modification, ctime(&modification));
            times[1].tv_sec = modification;
        } else {
            times[1].tv_nsec = UTIME_OMIT;
        }

        if (utimensat(0, file, times, AT_EMPTY_PATH) != 0) {
            warn("utimensat: %d", ret);
            exit(1);
        }
    }

    return (0);
}
