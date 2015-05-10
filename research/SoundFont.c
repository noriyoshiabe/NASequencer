#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define UCHAR2DWARD(c1,c2,c3,c4) (c1|c2<<8|c3<<16|c4<<24)
#define DWARD_FROM_LE(v) UCHAR2DWARD(((uint8_t *)&v)[0],((uint8_t *)&v)[1],((uint8_t *)&v)[2],((uint8_t *)&v)[3])

#define RIFF UCHAR2DWARD('R','I','F','F')
#define LIST UCHAR2DWARD('L','I','S','T')

#define ifil UCHAR2DWARD('i','f','i','l')
#define isng UCHAR2DWARD('i','s','n','g')
#define INAM UCHAR2DWARD('I','N','A','M')
#define irom UCHAR2DWARD('i','r','o','m')
#define iver UCHAR2DWARD('i','v','e','r')
#define ICRD UCHAR2DWARD('I','C','R','D')
#define IENG UCHAR2DWARD('I','E','N','G')
#define IPRD UCHAR2DWARD('I','P','R','D')
#define ICOP UCHAR2DWARD('I','C','O','P')
#define ICMT UCHAR2DWARD('I','C','M','T')
#define ISFT UCHAR2DWARD('I','S','F','T')

// sdta
//? #define snam UCHAR2DWARD('s','n','a','m')
#define smpl UCHAR2DWARD('s','m','p','l')
#define sm24 UCHAR2DWARD('s','m','2','4')

#define pdta UCHAR2DWARD('p','d','t','a')
#define phdr UCHAR2DWARD('p','h','d','r')
#define pbag UCHAR2DWARD('p','b','a','g')
#define pmod UCHAR2DWARD('p','m','o','d')
#define pgen UCHAR2DWARD('p','g','e','n')
#define inst UCHAR2DWARD('i','n','s','t')
#define ibag UCHAR2DWARD('i','b','a','g')
#define imod UCHAR2DWARD('i','m','o','d')
#define igen UCHAR2DWARD('i','g','e','n')
#define shdr UCHAR2DWARD('s','h','d','r')

#define INFO UCHAR2DWARD('I','N','F','O')
#define sdta UCHAR2DWARD('s','d','t','a')
#define pdta UCHAR2DWARD('p','d','t','a')

const char *ChunkID2String(uint32_t id)
{
#define CASE(id) case id: return #id;
    switch (id) {
    CASE(RIFF);
    CASE(LIST);

    // INFO
    CASE(ifil);
    CASE(isng);
    CASE(INAM);
    CASE(irom);
    CASE(iver);
    CASE(ICRD);
    CASE(IENG);
    CASE(IPRD);
    CASE(ICOP);
    CASE(ICMT);
    CASE(ISFT);

    // sdta
    //? CASE(snam);
    CASE(smpl);
    CASE(sm24);

    // pdta
    CASE(phdr);
    CASE(pbag);
    CASE(pmod);
    CASE(pgen);
    CASE(inst);
    CASE(ibag);
    CASE(imod);
    CASE(igen);
    CASE(shdr);
    }
#undef CASE
    return "Unknown";
};


typedef struct {
    uint32_t id;
    uint32_t size;
} ChunkHeader;

static void dumpf(int level, const char *format, ...)
{
    static char indent[128];
    memset(indent, ' ', level * 2);
    indent[level * 2] = '\0';
    printf("%s", indent);

    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

static bool readRIFFChunk(FILE *file, uint32_t size, int level, size_t *readLength);
static bool readLISTChunk(FILE *file, uint32_t size, int level, size_t *readLength);

static bool readChunk(FILE *file, int level, size_t *readLength)
{
    ChunkHeader header;
    if (1 != fread(&header, sizeof(ChunkHeader), 1, file)) {
        return false;
    }

    if (readLength) {
        *readLength += sizeof(ChunkHeader);
    }

    header.id = DWARD_FROM_LE(header.id);
    header.size = DWARD_FROM_LE(header.size);

    dumpf(level, "[%s] size=%d\n", ChunkID2String(header.id), header.size);

    switch (header.id) {
    case RIFF:
        if (!readRIFFChunk(file, header.size, level, readLength)) {
            return false;
        }
        break;

    case LIST:
        if (!readLISTChunk(file, header.size, level, readLength)) {
            return false;
        }
        break;

    default:
        fseek(file, header.size, SEEK_CUR);
        break;
    }

    if (readLength) {
        *readLength += header.size;
    }

    return true;
}

static bool readRIFFChunk(FILE *file, uint32_t size, int level, size_t *readLength)
{
    char form[5] = {0};
    if (1 != fread(form, 4, 1, file)) {
        return false;
    }

    dumpf(level, "form=%s\n", form);

    while (!feof(file)) {
        if (!readChunk(file, level + 1, readLength)) {
            return false;
        }
    }

    return true;
}

static bool readLISTChunk(FILE *file, uint32_t size, int level, size_t *readLength)
{
    char type[5] = {0};
    if (1 != fread(type, 4, 1, file)) {
        return false;
    }

    size -= 4;

    dumpf(level, "type=%s\n", type);

    size_t _readLength = 0;

    while (_readLength < size) {
        if (!readChunk(file, level + 1, &_readLength)) {
            return false;
        }
    }

    return true;
}



int main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "rb");
    readChunk(file, 0 , NULL);
    fclose(file);

    return 0;
}
