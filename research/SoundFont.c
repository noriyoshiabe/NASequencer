#include "SoundFont.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UCHAR2DWARD(c1,c2,c3,c4) (c1|c2<<8|c3<<16|c4<<24)
#define UCHAR2WARD(c1,c2) (c1|c2<<8)
#define DWARD_FROM_LE(v) UCHAR2DWARD(((uint8_t *)&v)[0],((uint8_t *)&v)[1],((uint8_t *)&v)[2],((uint8_t *)&v)[3])
#define WARD_FROM_LE(v) UCHAR2WARD(((uint8_t *)&v)[0],((uint8_t *)&v)[1])

// RIFF
#define RIFF UCHAR2DWARD('R','I','F','F')
#define LIST UCHAR2DWARD('L','I','S','T')

// INFO
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
// #define snam UCHAR2DWARD('s','n','a','m') 
//   -> FluidSynth has this but sf2 specification doesn't
#define smpl UCHAR2DWARD('s','m','p','l')
#define sm24 UCHAR2DWARD('s','m','2','4')

// pdta
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

// type of RIFF or LIST chunk
#define sfbk UCHAR2DWARD('s','f','b','k')
#define INFO UCHAR2DWARD('I','N','F','O')
#define sdta UCHAR2DWARD('s','d','t','a')
#define pdta UCHAR2DWARD('p','d','t','a')

static const char *ChunkID2String(uint32_t id)
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
    // CASE(snam);
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

static SoundFont *SoundFontCreate()
{
    SoundFont *self = calloc(1, sizeof(SoundFont));
    return self;
}

void SoundFontDestroy(SoundFont *self)
{
    free(self);
}

typedef bool (*Processer)(SoundFont *, FILE *, uint32_t, uint32_t);
static Processer findProcesser(uint32_t chunkID);

SoundFont *SoundFontRead(const char *filepath, SoundFontError *error)
{
    const uint32_t ListTypes[] = {INFO, sdta, pdta};
    int listIndex = 0;

    SoundFontError _error;
    SoundFont *self = SoundFontCreate();

    FILE *fp = fopen(filepath, "rb");
    if (!fp)  {
        _error = SoundFontErrorFileNotFound;
        goto ERROR_1;
    }

    for (;;) {
        ChunkHeader header;
        uint32_t type;
        Processer processer;

        if (1 != fread(&header, sizeof(ChunkHeader), 1, fp)) {
            if (feof(fp)) {
                break;
            }

            _error = SoundFontErrorInvalidFileFormat;
            goto ERROR_2;
        }

        header.id = DWARD_FROM_LE(header.id);
        header.size = DWARD_FROM_LE(header.size);

#if 1
        printf("[%s] size=%d\n", ChunkID2String(header.id), header.size);
#endif

        switch (header.id) {
        case RIFF:
            if (1 != fread(&type, sizeof(type), 1, fp)
                    || sfbk != DWARD_FROM_LE(type)) {
                _error = SoundFontErrorInvalidFileFormat;
                goto ERROR_2;
            }
            break;

        case LIST:
            if (1 != fread(&type, sizeof(type), 1, fp)
                    || sizeof(ListTypes)/sizeof(ListTypes[0]) <= listIndex
                    || ListTypes[listIndex] != DWARD_FROM_LE(type)) {
                _error = SoundFontErrorInvalidFileFormat;
                goto ERROR_2;
            }
            ++listIndex;
            break;

        default:
            processer = findProcesser(header.id);
            if (!processer(self, fp, header.id, header.size)) {
                _error = SoundFontErrorInvalidFileFormat;
                goto ERROR_2;
            }
            break;
        }
    }

    fclose(fp);
    return self;

ERROR_2:
    fclose(fp);

ERROR_1:
    SoundFontDestroy(self);
    
    if (error) {
        *error = _error;
    }

    return NULL;
}

static bool process_ifil_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    bool ret = 1 == fread(&self->version, sizeof(ifil), 1, fp);
    self->version.major = WARD_FROM_LE(self->version.major);
    self->version.minor = WARD_FROM_LE(self->version.minor);
    return ret;
}

static const struct {
    uint32_t chunkID;
    Processer processer;
} processerTable[] = {
    {ifil, process_ifil_Chunk},
};

static bool processUnimplementedChunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    printf("processUnimplementedChunk() id=%s size=%d\n", ChunkID2String(chunkId), size);
    fseek(fp, size, SEEK_CUR);
    return true;
}

static Processer findProcesser(uint32_t chunkID)
{
    for (int i = 0; i < sizeof(processerTable)/sizeof(processerTable[0]); ++i) {
        if (processerTable[i].chunkID == chunkID) {
            return processerTable[i].processer;
        }
    }

    return processUnimplementedChunk;
}

void SoundFontDump(SoundFont *self)
{
    printf("\n");
    printf("sf2 dump\n");
    printf("---------------------\n");
    printf("version: %d.%d\n", self->version.major, self->version.minor);
}


int main(int argc, char **argv)
{
    SoundFontError error;

    SoundFont *sf = SoundFontRead(argv[1], &error);
    if (!sf) {
        printf("error read sf2 error=%s\n", SoundFontError2String(error));
        return 1;
    }

    SoundFontDump(sf);
    SoundFontDestroy(sf);
    return 0;
}
