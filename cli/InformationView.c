#include "InformationView.h"

#include <stdio.h>

#define VERSION "0.4.0"
#define VERSION_LONG "NASequencer CLI Version "VERSION" January 2, 2016\n"

#define ___doc_en_nas_md     ___doc_md_note_as_sequence_md
#define ___doc_en_nas_md_len ___doc_md_note_as_sequence_md_len
#define ___doc_en_abc_md     ___doc_md_abc_notation_md
#define ___doc_en_abc_md_len ___doc_md_abc_notation_md_len
#define ___doc_en_mml_md     ___doc_md_music_macro_language_md
#define ___doc_en_mml_md_len ___doc_md_music_macro_language_md_len

extern unsigned char ___doc_en_nas_md[];
extern int ___doc_en_nas_md_len;
extern unsigned char ___doc_en_abc_md[];
extern int ___doc_en_abc_md_len;
extern unsigned char ___doc_en_mml_md[];
extern int ___doc_en_mml_md_len;

extern unsigned char license_txt[];
extern int license_txt_len;

const char *HEADER =
"Welcome to the NASequencer.\n"
"\n"
"NASequencer CLI (beta): version "VERSION"\n"
"Copyright (c) 2016, Noriyoshi Abe. All Rights Reserved.\n";

const char *GETTING_STARTED =
"Type 'help' for help.  Type 'exit' to finish today's creative activity.\n";

const char *USAGE =
"Usage: naseq [options] [file]\n"
"Options:\n"
" -o, --outout <file>      Write output to SMF, WAV or AAC.\n"
"                          WAV and AAC output require valid synthesizer with -s, --sound-font option.\n"
" -s, --sound-font <file>  Specify sound font file for synthesizer.\n"
" -h, --help               This help text.\n";

void InformationViewShowWelCome()
{
    fputs(HEADER, stdout);
    fputs("\n", stdout);
    fputs(GETTING_STARTED, stdout);
}

void InformationViewShowHeader()
{
    fputs(HEADER, stdout);
}

void InformationViewShowUsage()
{
    fputs(USAGE, stdout);
}

void InformationViewShowVersion()
{
    fputs(VERSION_LONG, stdout);
}

void InformationViewShowNAMidiSyntax()
{
    fwrite(___doc_en_nas_md, 1, ___doc_en_nas_md_len, stdout);
}

void InformationViewShowABCSyntax()
{
    fwrite(___doc_en_abc_md, 1, ___doc_en_abc_md_len, stdout);
}

void InformationViewShowMMLSyntax()
{
    fwrite(___doc_en_mml_md, 1, ___doc_en_mml_md_len, stdout);
}

void InformationViewShowAbout()
{
    fwrite(license_txt, 1, license_txt_len, stdout);
}
