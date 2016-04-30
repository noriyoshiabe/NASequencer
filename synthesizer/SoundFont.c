#include "SoundFont.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UCHAR2DWORD(c1,c2,c3,c4) (c1|c2<<8|c3<<16|c4<<24)
#define UCHAR2WORD(c1,c2) (c1|c2<<8)
#define DWORD_FROM_LE(v) UCHAR2DWORD(((uint8_t *)&v)[0],((uint8_t *)&v)[1],((uint8_t *)&v)[2],((uint8_t *)&v)[3])
#define WORD_FROM_LE(v) UCHAR2WORD(((uint8_t *)&v)[0],((uint8_t *)&v)[1])
#define SHORT_FROM_LE(v) UCHAR2WORD(((uint8_t *)&v)[0],((uint8_t *)&v)[1])

// RIFF
#define ChunkID_RIFF UCHAR2DWORD('R','I','F','F')
#define ChunkID_LIST UCHAR2DWORD('L','I','S','T')

// INFO
#define ChunkID_ifil UCHAR2DWORD('i','f','i','l')
#define ChunkID_isng UCHAR2DWORD('i','s','n','g')
#define ChunkID_INAM UCHAR2DWORD('I','N','A','M')
#define ChunkID_irom UCHAR2DWORD('i','r','o','m')
#define ChunkID_iver UCHAR2DWORD('i','v','e','r')
#define ChunkID_ICRD UCHAR2DWORD('I','C','R','D')
#define ChunkID_IENG UCHAR2DWORD('I','E','N','G')
#define ChunkID_IPRD UCHAR2DWORD('I','P','R','D')
#define ChunkID_ICOP UCHAR2DWORD('I','C','O','P')
#define ChunkID_ICMT UCHAR2DWORD('I','C','M','T')
#define ChunkID_ISFT UCHAR2DWORD('I','S','F','T')

// sdta
// #define snam UCHAR2DWORD('s','n','a','m') 
//   -> FluidSynth has this but sf2 specification doesn't
#define ChunkID_smpl UCHAR2DWORD('s','m','p','l')
#define ChunkID_sm24 UCHAR2DWORD('s','m','2','4')

// pdta
#define ChunkID_pdta UCHAR2DWORD('p','d','t','a')
#define ChunkID_phdr UCHAR2DWORD('p','h','d','r')
#define ChunkID_pbag UCHAR2DWORD('p','b','a','g')
#define ChunkID_pmod UCHAR2DWORD('p','m','o','d')
#define ChunkID_pgen UCHAR2DWORD('p','g','e','n')
#define ChunkID_inst UCHAR2DWORD('i','n','s','t')
#define ChunkID_ibag UCHAR2DWORD('i','b','a','g')
#define ChunkID_imod UCHAR2DWORD('i','m','o','d')
#define ChunkID_igen UCHAR2DWORD('i','g','e','n')
#define ChunkID_shdr UCHAR2DWORD('s','h','d','r')

// type of RIFF or LIST chunk
#define TypeID_sfbk UCHAR2DWORD('s','f','b','k')
#define TypeID_INFO UCHAR2DWORD('I','N','F','O')
#define TypeID_sdta UCHAR2DWORD('s','d','t','a')
#define TypeID_pdta UCHAR2DWORD('p','d','t','a')

static uint32_t crc32(uint8_t *bytes, int length);

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
    free_if(self->pmod);
    free_if(self->pgen);
    free_if(self->inst);
    free_if(self->ibag);
    free_if(self->imod);
    free_if(self->igen);
    free_if(self->shdr);
#undef free_if

    free(self);
}

typedef bool (*Processer)(SoundFont *, FILE *, uint32_t, uint32_t);
static Processer findProcesser(uint32_t chunkID);

static bool SoundFontValidate(SoundFont *self)
{
    if (0 == self->ifil.wMajor && 0 == self->ifil.wMinor) return false;

    if (!self->INAM) return false;
    if (!self->phdr) return false;
    if (!self->pbag) return false;
    if (!self->pmod) return false;
    if (!self->pgen) return false;
    if (!self->inst) return false;
    if (!self->ibag) return false;
    if (!self->imod) return false;
    if (!self->igen) return false;
    if (!self->shdr) return false;

    return true;
}

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

        header.id = DWORD_FROM_LE(header.id);
        header.size = DWORD_FROM_LE(header.size);

#if 0
        printf("[%s] size=%d\n", ChunkID2String(header.id), header.size);
#endif

        switch (header.id) {
        case ChunkID_RIFF:
            if (1 != fread(&type, sizeof(type), 1, fp)
                    || TypeID_sfbk != DWORD_FROM_LE(type)) {
                _error = SoundFontErrorInvalidFileFormat;
                goto ERROR_2;
            }
            break;

        case ChunkID_LIST:
            if (1 != fread(&type, sizeof(type), 1, fp)
                    || sizeof(ListTypes)/sizeof(ListTypes[0]) <= listIndex
                    || ListTypes[listIndex] != DWORD_FROM_LE(type)) {
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

    if (!SoundFontValidate(self)) {
        _error = SoundFontErrorInvalidFileFormat;
        goto ERROR_2;
    }

    int32_t data[11];

    data[0] = self->smplLength;
    data[1] = self->sm24Length;
    data[2] = self->phdrLength;
    data[3] = self->pbagLength;
    data[4] = self->pmodLength;
    data[5] = self->pgenLength;
    data[6] = self->instLength;
    data[7] = self->ibagLength;
    data[8] = self->imodLength;
    data[9] = self->igenLength;
    data[10] = self->shdrLength;

    self->crc32 = crc32((uint8_t *)data, 11 * 4);

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
    self->ifil.wMajor = WORD_FROM_LE(self->ifil.wMajor);
    self->ifil.wMinor = WORD_FROM_LE(self->ifil.wMinor);
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
    self->iver.wMajor = WORD_FROM_LE(self->iver.wMajor);
    self->iver.wMinor = WORD_FROM_LE(self->iver.wMinor);
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
            self->smpl[i] = WORD_FROM_LE(self->smpl[i]);
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
    int num = size / sizeof(SFPresetHeader);
    self->phdr = calloc(num, sizeof(SFPresetHeader));
    self->phdrLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->phdr[i], sizeof(SFPresetHeader), 1, fp)) {
            return false;
        }

        self->phdr[i].wPreset = WORD_FROM_LE(self->phdr[i].wPreset);
        self->phdr[i].wBank = WORD_FROM_LE(self->phdr[i].wBank);
        self->phdr[i].wPresetBagNdx = WORD_FROM_LE(self->phdr[i].wPresetBagNdx);

        self->phdr[i].dwLibrary = DWORD_FROM_LE(self->phdr[i].dwLibrary);
        self->phdr[i].dwGenre = DWORD_FROM_LE(self->phdr[i].dwGenre);
        self->phdr[i].dwMorphology = DWORD_FROM_LE(self->phdr[i].dwMorphology);
    }

    return true;
}

static bool process_pbag_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFPresetBag);
    self->pbag = calloc(num, sizeof(SFPresetBag));
    self->pbagLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->pbag[i], sizeof(SFPresetBag), 1, fp)) {
            return false;
        }

        self->pbag[i].wGenNdx = WORD_FROM_LE(self->pbag[i].wGenNdx);
        self->pbag[i].wModNdx = WORD_FROM_LE(self->pbag[i].wModNdx);
    }

    return true;
}

static bool process_pmod_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFModList);
    self->pmod = calloc(num, sizeof(SFModList));
    self->pmodLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->pmod[i], sizeof(SFModList), 1, fp)) {
            return false;
        }

        self->pmod[i].sfModDestOper = WORD_FROM_LE(self->pmod[i].sfModDestOper);
        self->pmod[i].modAmount = SHORT_FROM_LE(self->pmod[i].modAmount);
        self->pmod[i].sfModTransOper = WORD_FROM_LE(self->pmod[i].sfModTransOper);
    }

    return true;
}

static bool process_pgen_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFGenList);
    self->pgen = calloc(num, sizeof(SFGenList));
    self->pgenLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->pgen[i], sizeof(SFGenList), 1, fp)) {
            return false;
        }

        self->pgen[i].sfGenOper = WORD_FROM_LE(self->pgen[i].sfGenOper);

        switch (self->pgen[i].sfGenOper) {
        case SFGeneratorType_keyRange:
        case SFGeneratorType_velRange:
            break;
        case SFGeneratorType_instrument:
            self->pgen[i].genAmount.wAmount = WORD_FROM_LE(self->pgen[i].genAmount.wAmount);
            break;
        default:
            self->pgen[i].genAmount.shAmount = SHORT_FROM_LE(self->pgen[i].genAmount.shAmount);
            break;
        }
    }

    return true;
}

static bool process_inst_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFInst);
    self->inst = calloc(num, sizeof(SFInst));
    self->instLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->inst[i], sizeof(SFInst), 1, fp)) {
            return false;
        }

        self->inst[i].wInstBagNdx = WORD_FROM_LE(self->inst[i].wInstBagNdx);
    }

    return true;
}

static bool process_ibag_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFInstBag);
    self->ibag = calloc(num, sizeof(SFInstBag));
    self->ibagLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->ibag[i], sizeof(SFInstBag), 1, fp)) {
            return false;
        }

        self->ibag[i].wInstGenNdx = WORD_FROM_LE(self->ibag[i].wInstGenNdx);
        self->ibag[i].wInstModNdx = WORD_FROM_LE(self->ibag[i].wInstModNdx);
    }

    return true;
}

static bool process_imod_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFModList);
    self->imod = calloc(num, sizeof(SFModList));
    self->imodLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->imod[i], sizeof(SFModList), 1, fp)) {
            return false;
        }

        self->imod[i].sfModDestOper = WORD_FROM_LE(self->imod[i].sfModDestOper);
        self->imod[i].modAmount = SHORT_FROM_LE(self->imod[i].modAmount);
        self->imod[i].sfModTransOper = WORD_FROM_LE(self->imod[i].sfModTransOper);
    }

    return true;
}

static bool process_igen_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFGenList);
    self->igen = calloc(num, sizeof(SFGenList));
    self->igenLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->igen[i], sizeof(SFGenList), 1, fp)) {
            return false;
        }

        self->igen[i].sfGenOper = WORD_FROM_LE(self->igen[i].sfGenOper);

        switch (self->igen[i].sfGenOper) {
        case SFGeneratorType_keyRange:
        case SFGeneratorType_velRange:
            break;
        case SFGeneratorType_instrument:
            self->igen[i].genAmount.wAmount = WORD_FROM_LE(self->igen[i].genAmount.wAmount);
            break;
        default:
            self->igen[i].genAmount.shAmount = SHORT_FROM_LE(self->igen[i].genAmount.shAmount);
            break;
        }
    }

    return true;
}

static bool process_shdr_Chunk(SoundFont *self, FILE *fp, uint32_t chunkId, uint32_t size)
{
    int num = size / sizeof(SFSampleHeader);
    self->shdr = calloc(num, sizeof(SFSampleHeader));
    self->shdrLength = num;

    for (int i = 0; i < num; ++i) {
        if (1 != fread(&self->shdr[i], sizeof(SFSampleHeader), 1, fp)) {
            return false;
        }

        self->shdr[i].dwStart = DWORD_FROM_LE(self->shdr[i].dwStart);
        self->shdr[i].dwEnd = DWORD_FROM_LE(self->shdr[i].dwEnd);
        self->shdr[i].dwStartloop = DWORD_FROM_LE(self->shdr[i].dwStartloop);
        self->shdr[i].dwEndloop = DWORD_FROM_LE(self->shdr[i].dwEndloop);
        self->shdr[i].dwSampleRate = DWORD_FROM_LE(self->shdr[i].dwSampleRate);

        self->shdr[i].wSampleLink = WORD_FROM_LE(self->shdr[i].wSampleLink);
        self->shdr[i].sfSampleType = WORD_FROM_LE(self->shdr[i].sfSampleType);
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
    {ChunkID_pmod, process_pmod_Chunk},
    {ChunkID_pgen, process_pgen_Chunk},
    {ChunkID_inst, process_inst_Chunk},
    {ChunkID_ibag, process_ibag_Chunk},
    {ChunkID_imod, process_imod_Chunk},
    {ChunkID_igen, process_igen_Chunk},
    {ChunkID_shdr, process_shdr_Chunk},
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
        printf("phdr[%d]: ", i);
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
        printf("pbag[%d]: ", i);
        printf("wGenNdx=%u ", self->pbag[i].wGenNdx);
        printf("wModNdx=%u", self->pbag[i].wModNdx);
        printf("\n");
    }
    printf("pmod: num of mod list=%u\n", self->pmodLength);
    for (int i = 0; i < self->pmodLength; ++i) {
        printf("    ");
        printf("pmod[%d]: ", i);
        printf("sfModSrcOper[");
        printf("Type=%u ", self->pmod[i].sfModSrcOper.Type);
        printf("P=%u ", self->pmod[i].sfModSrcOper.P);
        printf("D=%u ", self->pmod[i].sfModSrcOper.D);
        printf("CC=%u ", self->pmod[i].sfModSrcOper.CC);
        printf("Index=%u] ", self->pmod[i].sfModSrcOper.Index);
        printf("sfModDestOper=%d ", self->pmod[i].sfModDestOper);
        printf("modAmount=%d ", self->pmod[i].modAmount);
        printf("sfModAmtSrcOper[");
        printf("Type=%u ", self->pmod[i].sfModAmtSrcOper.Type);
        printf("P=%u ", self->pmod[i].sfModAmtSrcOper.P);
        printf("D=%u ", self->pmod[i].sfModAmtSrcOper.D);
        printf("CC=%u ", self->pmod[i].sfModAmtSrcOper.CC);
        printf("Index=%u] ", self->pmod[i].sfModAmtSrcOper.Index);
        printf("sfModTransOper=%u", self->pmod[i].sfModTransOper);
        printf("\n");
    }
    printf("pgen: num of gen list=%u\n", self->pgenLength);
    for (int i = 0; i < self->pgenLength; ++i) {
        printf("    ");
        printf("pgen[%d]: ", i);
        printf("sfGenOper=%u ", self->pgen[i].sfGenOper);
        switch (self->pgen[i].sfGenOper) {
        case SFGeneratorType_keyRange:
        case SFGeneratorType_velRange:
            printf("genAmount.ranges.byLo=%u ", self->pgen[i].genAmount.ranges.byLo);
            printf("genAmount.ranges.byHi=%u ", self->pgen[i].genAmount.ranges.byHi);
            break;
        case SFGeneratorType_instrument:
            printf("genAmount.wAmount=%u", self->pgen[i].genAmount.wAmount);
            break;
        default:
            printf("genAmount.shAmount=%d", self->pgen[i].genAmount.shAmount);
            break;
        }
        printf("\n");
    }
    printf("inst: num of instruments list=%u\n", self->instLength);
    for (int i = 0; i < self->instLength; ++i) {
        printf("    ");
        printf("inst[%d]: ", i);
        printf("achInstName=%s ", self->inst[i].achInstName);
        printf("wInstBagNdx=%u ", self->inst[i].wInstBagNdx);
        printf("\n");
    }
    printf("ibag: num of inst bag=%u\n", self->ibagLength);
    for (int i = 0; i < self->ibagLength; ++i) {
        printf("    ");
        printf("ibag[%d]: ", i);
        printf("wInstGenNdx=%u ", self->ibag[i].wInstGenNdx);
        printf("wInstModNdx=%u", self->ibag[i].wInstModNdx);
        printf("\n");
    }
    printf("imod: num of mod list=%u\n", self->imodLength);
    for (int i = 0; i < self->imodLength; ++i) {
        printf("    ");
        printf("imod[%d]: ", i);
        printf("sfModSrcOper[");
        printf("Type=%u ", self->imod[i].sfModSrcOper.Type);
        printf("P=%u ", self->imod[i].sfModSrcOper.P);
        printf("D=%u ", self->imod[i].sfModSrcOper.D);
        printf("CC=%u ", self->imod[i].sfModSrcOper.CC);
        printf("Index=%u] ", self->imod[i].sfModSrcOper.Index);
        printf("sfModDestOper=%d ", self->imod[i].sfModDestOper);
        printf("modAmount=%d ", self->imod[i].modAmount);
        printf("sfModAmtSrcOper[");
        printf("Type=%u ", self->imod[i].sfModAmtSrcOper.Type);
        printf("P=%u ", self->imod[i].sfModAmtSrcOper.P);
        printf("D=%u ", self->imod[i].sfModAmtSrcOper.D);
        printf("CC=%u ", self->imod[i].sfModAmtSrcOper.CC);
        printf("Index=%u] ", self->imod[i].sfModAmtSrcOper.Index);
        printf("sfModTransOper=%u", self->imod[i].sfModTransOper);
        printf("\n");
    }
    printf("igen: num of gen list=%u\n", self->igenLength);
    for (int i = 0; i < self->igenLength; ++i) {
        printf("    ");
        printf("igen[%d]: ", i);
        printf("sfGenOper=%u ", self->igen[i].sfGenOper);
        switch (self->igen[i].sfGenOper) {
        case SFGeneratorType_keyRange:
        case SFGeneratorType_velRange:
            printf("genAmount.ranges.byLo=%u ", self->igen[i].genAmount.ranges.byLo);
            printf("genAmount.ranges.byHi=%u ", self->igen[i].genAmount.ranges.byHi);
            break;
        case SFGeneratorType_instrument:
            printf("genAmount.wAmount=%u", self->igen[i].genAmount.wAmount);
            break;
        default:
            printf("genAmount.shAmount=%d", self->igen[i].genAmount.shAmount);
            break;
        }
        printf("\n");
    }
    printf("shdr: num of sample header=%u\n", self->shdrLength);
    for (int i = 0; i < self->shdrLength; ++i) {
        printf("    ");
        printf("shdr[%d]:\n", i);
        printf("        ");
        printf("achSampleName=%s ", self->shdr[i].achSampleName);
        printf("dwStart=%u ", self->shdr[i].dwStart);
        printf("dwEnd=%u ", self->shdr[i].dwEnd);
        printf("dwStartloop=%u ", self->shdr[i].dwStartloop);
        printf("dwEndloop=%u ", self->shdr[i].dwEndloop);
        printf("dwSampleRate=%u", self->shdr[i].dwSampleRate);
        printf("\n");
        printf("        ");
        printf("byOriginalPitch=%u ", self->shdr[i].byOriginalPitch);
        printf("chPitchCorrection=%u ", self->shdr[i].chPitchCorrection);
        printf("wSampleLink=%u ", self->shdr[i].wSampleLink);
        printf("sfSampleType=%u", self->shdr[i].sfSampleType);
        printf("\n");
    }
}

static uint32_t crc32(uint8_t *bytes, int length)
{
    const uint32_t table[256] = { 
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
        0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
        0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
        0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
        0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
        0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
        0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
        0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
        0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
        0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
        0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
        0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
        0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
        0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
        0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
        0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
    };

	uint32_t crc = 0 ^ 0xFFFFFFFF;
	
    for (; length--; bytes++) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ table[(crc ^ (*bytes)) & 0xFF];
	}

	return crc ^ 0xFFFFFFFF;
}
