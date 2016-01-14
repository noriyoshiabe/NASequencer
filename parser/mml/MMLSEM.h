#pragma once

#include "Node.h"
#include "NoteTable.h"
#include "NAArray.h"

typedef struct _SEMList {
    Node node;
} SEMList;

typedef struct _SEMVisitor {
    void (*visitList)(void *self, SEMList *sem);
} SEMVisitor;

extern SEMList *MMLSEMListCreate(FileLocation *location);
