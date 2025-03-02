#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/mman.h>

#define DECODE_VARINT(schifter,  buf, val, idx)         \
        {                                               \
            shifter = 0, val = 0;                       \
            do {                                        \
                idx++;                                  \
                val |= (buf[idx] & 0x7F) << shifter;    \
                shifter += 7;                           \
            } while (buf[idx] & 0x80);                  \
        }

#define DECODE_FIELD_NUMBER(buf, val, idx)              \
        {                                               \
            val = buf[idx] >> 3;                        \
        }

#define DECODE_FIELD_TYPE(buf, val, idx)                \
        {                                               \
            val = buf[idx] & 7;                         \
        }

#define PB_VARINT   0
#define PB_I64      1
#define PB_LEN      2
#define PB_SGROUP   3
#define PB_EGROUP   4
#define PB_I32      5

int parse_protobuf(uint8_t * data, uint64_t len, int deep)
{
    uint64_t shifter;
    uint64_t i, j;
    uint8_t type;
    uint8_t number;

    i = 0;
    j = 0;
    while (i < len) {
        
        DECODE_FIELD_NUMBER( data, number, i);
        DECODE_FIELD_TYPE( data, type, i);

        printf("%*s-----------\n", deep, "");
        printf("%*sfield number=%" PRIu8 "\n", deep*2, "", number);
        printf("%*sfield type=%" PRIu8 "\n", deep*2, "", type);

        switch (type) {
        case PB_VARINT:
            DECODE_VARINT(shifter, data, j, i);
            printf("%*svarint: uint %" PRIu64 " sint %c%" PRIu64 "\n", deep*2, "",
                j,
                j%2 ? '-':'+',
                j%2 ? (j/2)+1 : (j/2));
            i++;
            break;
        case PB_I64:
            i += 9;
            break;
        case PB_LEN:
            DECODE_VARINT(shifter, data, j, i);
            i++;
            if ((i + j) > len) {
                return -1;
            }
            if (parse_protobuf(&data[i], j, deep + 1) != 0) {
            
                int b_ascci=1;
                for (int k = 0; k < j ; k++) {
                    if (!isascii(data[i+k])) {
                        b_ascci = 0;
                        break;
                    }
                }

                printf("%*s%s = [", deep*2, "", b_ascci ? "string" : "data");
                for (int k = 0; k < j ; k++) {
                    if (b_ascci)
                        printf("%c", data[i+k]);
                    else
                        printf("%00x", data[i+k]);
                }
                printf("]\n");
            }
            i += j;
            break;
        default:
            printf("%*sBad message\n",deep*2, "");
            printf("%*s-----------\n", deep, "");
            return -1;
        }
        printf("%*s-----------\n", deep, "");
    }

    return 0;
}

int main(int argc, char * argv[])
{
    uint8_t data_in[] = {
        0x0a, 0x2f, 0x0a, 0x08,
        0x4a, 0x6f, 0x68, 0x6e,
        0x20, 0x44, 0x6f, 0x65,
        0x10, 0x01, 0x1a, 0x10,
        0x6a, 0x6f, 0x68, 0x6e,
        0x40, 0x65, 0x78, 0x61,
        0x6d, 0x70, 0x6c, 0x65,
        0x2e, 0x63, 0x6f, 0x6d,
        0x22, 0x0f, 0x0a, 0x0b,
        0x31, 0x31, 0x31, 0x2d,
        0x32, 0x32, 0x32, 0x2d,
        0x33, 0x33, 0x33, 0x10,
        0x01, 0x0a, 0x1e, 0x0a,
        0x08, 0x4a, 0x61, 0x6e,
        0x65, 0x20, 0x44, 0x6f,
        0x65, 0x10, 0x02, 0x1a,
        0x10, 0x6a, 0x61, 0x6e,
        0x65, 0x40, 0x65, 0x78,
        0x61, 0x6d, 0x70, 0x6c,
        0x65, 0x2e, 0x63, 0x6f,
        0x6d
    };

    return parse_protobuf(data_in, 81, 0);
}
