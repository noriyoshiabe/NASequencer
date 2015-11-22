#pragma once

#include "Parser.h"
#include "NAArray.h"

typedef struct _BuildInformation {
    NAArray *filepaths;
    NAArray *errors;
} BuildInformation;

extern BuildInformation *BuildInformationCreate();
extern BuildInformation *BuildInformationRetain(BuildInformation *self);
extern void BuildInformationRelease(BuildInformation *self);
