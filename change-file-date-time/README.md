# Set file or directory date/time

The examples show who to use the utimensat to change
the file birth, creation or modification time.

```
setfiletime -f <filename> [-b <time>] [-m <time>] [-a <time>]
 -f <set the directory or filename (absolute path)
 -b set the file birth time (if supported by the filesystem)
 -m set the file modification time
 -a set the file access time
 Time is the desired date and time in RFC3339 format
 eg. "2011-07-12T11:03:12+02:00"
```
