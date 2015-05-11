#include "SoundFont.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UCHAR2DWARD(c1,c2,c3,c4) (c1|c2<<8|c3<<16|c4<<24)
#define UCHAR2WARD(c1,c2) (c1|c2<<8)
#define DWARD_FROM_LE(v) UCHAR2DWARD(((uint8_t *)&v)[0],((uint8_t *)&v)[1],((uint8_t *)&v)[2],((uint8_t *)&v)[3])
#define WARD_FROM_LE(v) UCHAR2WARD(((uint8_t *)&v)[0],((uint8_t *)&v)[1])

// RIFF
#define ChunkID_RIFF UCHAR2DWARD('R','I','F','F')
#define ChunkID_LIST UCHAR2DWARD('L','I','S','T')

// INFO
#define ChunkID_ifil UCHAR2DWARD('i','f','i','l')
#define ChunkID_isng UCHAR2DWARD('i','s','n','g')
#define ChunkID_INAM UCHAR2DWARD('I','N','A','M')
#define ChunkID_irom UCHAR2DWARD('i','r','o','m')
#define ChunkID_iver UCHAR2DWARD('i','v','e','r')
#define ChunkID_ICRD UCHAR2DWARD('I','C','R','D')
#define ChunkID_IENG UCHAR2DWARD('I','E','N','G')
#define ChunkID_IPRD UCHAR2DWARD('I','P','R','D')
#define ChunkID_ICOP UCHAR2DWARD('I','C','O','P')
#define ChunkID_ICMT UCHAR2DWARD('I','C','M','T')
#define ChunkID_ISFT UCHAR2DWARD('I','S','F','T')

// sdta
// #define snam UCHAR2DWARD('s','n','a','m') 
//   -> FluidSynth has this but sf2 specification doesn't
#define ChunkID_smpl UCHAR2DWARD('s','m','p','l')
#define ChunkID_sm24 UCHAR2DWARD('s','m','2','4')

// pdta
#define ChunkID_pdta UCHAR2DWARD('p','d','t','a')
#define ChunkID_phdr UCHAR2DWARD('p','h','d','r')
#define ChunkID_pbag UCHAR2DWARD('p','b','a','g')
#define ChunkID_pmod UCHAR2DWARD('p','m','o','d')
#define ChunkID_pgen UCHAR2DWARD('p','g','e','n')
#define ChunkID_inst UCHAR2DWARD('i','n','s','t')
#define ChunkID_ibag UCHAR2DWARD('i','b','a','g')
#define ChunkID_imod UCHAR2DWARD('i','m','o','d')
#define ChunkID_igen UCHAR2DWARD('i','g','e','n')
#define ChunkID_shdr UCHAR2DWARD('s','h','d','r')

// type of RIFF or LIST chunk
#define TypeID_sfbk UCHAR2DWARD('s','f','b','k')
#define TypeID_INFO UCHAR2DWARD('I','N','F','O')
#define TypeID_sdta UCHAR2DWARD('s','d','t','a')
#define TypeID_pdta UCHAR2DWARD('p','d','t','a')

static const char *ChunkID2String(uint32_t id)
{
#define CASE(id) case id: return &(#id[8]);
    switch (id) {
    CASE(ChunkID_RIFF);
    CASE(ChunkID_LIST);

    // INFO
    CASE(ChunkID_ifil);
    CASE(ChunkID_isng);
    CASE(ChunkID_INAM);
    CASE(ChunkID_irom);
    CASE(ChunkID_iver);
    CASE(ChunkID_ICRD);
    CASE(ChunkID_IENG);
    CASE(ChunkID_IPRD);
    CASE(ChunkID_ICOP);
    CASE(ChunkID_ICMT);
    CASE(ChunkID_ISFT);

    // sdta
    // CASE(ChunkID_snam);
    CASE(ChunkID_smpl);
    CASE(ChunkID_sm24);

    // pdta
    CASE(ChunkID_phdr);
    CASE(ChunkID_pbag);
    CASE(ChunkID_pmod);
    CASE(ChunkID_pgen);
    CASE(ChunkID_inst);
    CASE(ChunkID_ibag);
    CASE(ChunkID_imod);
    CASE(ChunkID_igen);
    CASE(ChunkID_shdr);
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
#define free_if(p) do { if (p) free(p); } while (0)
    free_if(self->isng);
    free_if(self->INAM);
    free_if(self->irom);
    free_if(self->ICRD);
    free_if(self->IENG);
    free_if(self->IPRD);
    free_if(self->ICOP);
    free_if(self->ICMT);
    free_if(self->ISFT);
    free_if(self->smpl);
    free_if(self->sm24);
    free_if(self->phdr);
    free_if(self->pbag);
#undef free_if

    free(self);
}

typedef bool (*Processer)(SoundFont *, FILE *, uint32_t, uint32_t);
static Processer findProcesser(uint32_t chunkID);

SoundFont *SoundFontRead(const char *filepath, SoundFontError *error)
{
    const uint32_t ListTypes[] = {TypeID_INFO, TypeID_sdta, TypeID_pdta};
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
        case ChunkID_RIFF:
            if (1 != fread(&type, sizeof(type), 1, fp)
                    || TypeID_sfbk != DWARD_FROM_LE(type)) {
                _error = SoundFontErrorInvalidFileFormat;
                goto ERROR_2;
            }
            break;

        case ChunkID_LIST:
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

            if (ChunkID_ifil == header.id) {
                if (2 > self->ifil.wMajor) {
                    _error = SoundFontErrorUnsupportedVersion;
                    goto ERROR_2;
                }
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
    bool ret = 1 == fread(&self->ifil, sizeof(self->ifil), 1, fp);
    self->ifil.wMajor = WARD_FROM_LE(self->ifil.wMajor);
    self->ifil.wMinor = WARD_FROM_LE(self->ifil.wMinor);
    return ret;
}

#define DeclareProcessInfoTextChunk(id) \
    static bool process_##id##_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size) \
    { \
        self->id = malloc(size); \
        return 1 == fread(self->id, size, 1, fp); \
    }

DeclareProcessInfoTextChunk(isng);
DeclareProcessInfoTextChunk(INAM);
DeclareProcessInfoTextChunk(irom);

static bool process_iver_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    bool ret = 1 == fread(&self->iver, sizeof(self->iver), 1, fp);
    self->iver.wMajor = WARD_FROM_LE(self->iver.wMajor);
    self->iver.wMinor = WARD_FROM_LE(self->iver.wMinor);
    return ret;
}

DeclareProcessInfoTextChunk(ICRD);
DeclareProcessInfoTextChunk(IENG);
DeclareProcessInfoTextChunk(IPRD);
DeclareProcessInfoTextChunk(ICOP);
DeclareProcessInfoTextChunk(ICMT);
DeclareProcessInfoTextChunk(ISFT);

static bool process_smpl_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    volatile const int __is_little_endian__ = 1;

    self->smpl = malloc(size);
    self->smplLength = size / 2;
    bool ret = 1 == fread(self->smpl, size, 1, fp);

    if (!*((uint8_t *)&__is_little_endian__)) {
        for (int i = 0; i < size; ++i) {
            self->smpl[i] = WARD_FROM_LE(self->smpl[i]);
        }
    }

    return ret;
}

static bool process_sm24_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    if (self->smplLength != size || 4 > self->ifil.wMinor) {
        /*
         * If the smpl Sub-chunk is not present, the sm24 sub-chunk should be ignored.
         * If the ifil version of the format is less than that which represents 2.04, the sm24 sub-chunk should be ignored.
         * If the size of the sm24 chunk is not exactly equal to the 1/2 the size of the smpl chunk (+ 1 byte in the case that 1/2 the size of smpl chunk is an odd value), the sm24 sub-chunk should be ignored.
         *
         * (Quote from SoundFont Technical Specification Version 2.04)
         */
        return true;
    }

    self->sm24 = malloc(size);
    self->sm24Length = size;
    return 1 == fread(self->sm24, size, 1, fp);
}

static bool process_phdr_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    const int PresetHeaderSize = 38; // Care of alignment

    int num = size / PresetHeaderSize;
    self->phdr = calloc(num, sizeof(PresetHeader)); // Size including alignment is 40
    self->phdrLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->phdr[i], PresetHeaderSize, 1, fp)) {
            return false;
        }

        self->phdr[i].wPreset = WARD_FROM_LE(self->phdr[i].wPreset);
        self->phdr[i].wBank = WARD_FROM_LE(self->phdr[i].wBank);
        self->phdr[i].wPresetBagNdx = WARD_FROM_LE(self->phdr[i].wPresetBagNdx);

        self->phdr[i].dwLibrary = DWARD_FROM_LE(self->phdr[i].dwLibrary);
        self->phdr[i].dwGenre = DWARD_FROM_LE(self->phdr[i].dwGenre);
        self->phdr[i].dwMorphology = DWARD_FROM_LE(self->phdr[i].dwMorphology);
    }

    return true;
}

static bool process_pbag_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(PresetBag);
    self->pbag = calloc(num, sizeof(PresetBag));
    self->pbagLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->pbag[i], sizeof(PresetBag), 1, fp)) {
            return false;
        }

        self->pbag[i].wGenNdx = WARD_FROM_LE(self->pbag[i].wGenNdx);
        self->pbag[i].wModNdx = WARD_FROM_LE(self->pbag[i].wModNdx);
    }

    return true;
}

static const struct {
    uint32_t chunkID;
    Processer processer;
} processerTable[] = {
    {ChunkID_ifil, process_ifil_Chunk},
    {ChunkID_isng, process_isng_Chunk},
    {ChunkID_INAM, process_INAM_Chunk},
    {ChunkID_irom, process_irom_Chunk},
    {ChunkID_iver, process_iver_Chunk},
    {ChunkID_ICRD, process_ICRD_Chunk},
    {ChunkID_IENG, process_IENG_Chunk},
    {ChunkID_IPRD, process_IPRD_Chunk},
    {ChunkID_ICOP, process_ICOP_Chunk},
    {ChunkID_ICMT, process_ICMT_Chunk},
    {ChunkID_ISFT, process_ISFT_Chunk},
    {ChunkID_smpl, process_smpl_Chunk},
    {ChunkID_sm24, process_sm24_Chunk},
    {ChunkID_phdr, process_phdr_Chunk},
    {ChunkID_pbag, process_pbag_Chunk},
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
    printf("ifil: %u.%u\n", self->ifil.wMajor, self->ifil.wMinor);
    printf("isng: %s\n", self->isng);
    printf("INAM: %s\n", self->INAM);
    printf("irom: %s\n", self->irom);
    printf("iver: %u.%u\n", self->iver.wMajor, self->iver.wMinor);
    printf("ICRD: %s\n", self->ICRD);
    printf("IENG: %s\n", self->IENG);
    printf("IPRD: %s\n", self->IPRD);
    printf("ICOP: %s\n", self->ICOP);
    printf("ICMT: %s\n", self->ICMT);
    printf("ISFT: %s\n", self->ISFT);
    printf("smpl: num of sample=%u\n", self->smplLength);
    printf("sm24: num of sample=%u\n", self->sm24Length);
    printf("phdr: num of preset header=%u\n", self->phdrLength);
    for (int i = 0; i < self->phdrLength; ++i) {
        printf("    ");
        printf("achPresetName=%s ", self->phdr[i].achPresetName);
        printf("wPreset=%u ", self->phdr[i].wPreset);
        printf("wBank=%u ", self->phdr[i].wBank);
        printf("wPresetBagNdx=%u ", self->phdr[i].wPresetBagNdx);
        printf("dwLibrary=%u ", self->phdr[i].dwLibrary);
        printf("dwGenre=%u ", self->phdr[i].dwGenre);
        printf("dwMorphology=%u", self->phdr[i].dwMorphology);
        printf("\n");
    }
    printf("pbag: num of preset bag=%u\n", self->pbagLength);
    for (int i = 0; i < self->pbagLength; ++i) {
        printf("    ");
        printf("wGenNdx=%u ", self->pbag[i].wGenNdx);
        printf("wModNdx=%u", self->pbag[i].wModNdx);
        printf("\n");
    }
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
