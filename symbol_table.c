#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Reg* top = NULL;

struct Reg* search(char* regName) {
    struct Reg* p = top;
    while(p != NULL && strcmp(p->regName, regName) != 0) p = p->next;
    return p;
}

struct Reg* getTop() {
    return top;
}

struct Reg* searchRegType(char* regName, enum RegType regType) {
    struct Reg *p = search(regName);
    if(p != NULL && p->type==regType) return p; else return NULL;
}

void newReg(char* regName, enum RegType type, struct Reg* typeReg, int line) {
    if(search(regName) != NULL) printf("Error: Name already defined");
    struct Reg* new = (struct Reg*) malloc(sizeof(struct Reg));
    new->regName = regName;
    new->type = type;
    new->typeReg = typeReg;
    new->lineOfDeclaration = line;
    new->next = top;
    top = new;
}

void newRegSetValue(char* regName, enum RegType type, struct Reg* typeReg, int line, void* value) {
    if(search(regName) != NULL) printf("Error: Name already defined");
    struct Reg* new = (struct Reg*) malloc(sizeof(struct Reg));
    new->regName = regName;
    new->type = type;
    new->typeReg = typeReg;
    new->lineOfDeclaration = line;
    new->next = top;
    new->value = value;
    top = new;
}

void setRegValue(char* regName, enum RegType localGlobal, void* value) {
    struct Reg* result = searchRegType(regName, localGlobal);
    if(result == NULL) return;
    result->value = value;
}

void* getRegValue(char* regName, enum RegType localGlobal) {
    struct Reg* result = searchRegType(regName, localGlobal);
    if(result == NULL) return NULL;
    return result->value;
}

void closeBlock() {
    dump("--CLOSING BLOCK--\n");
    while(top != NULL && top->type != function && strcmp(top->regName, "openBlock") != 0) {
        struct Reg *p = top->next;
        if (top->regName != NULL) free(top->regName);
        if (top->value != NULL) free(top->value);
        free(top); 
        top=p;
    }
    struct Reg *p = top->next;
    if (top->regName != NULL) free(top->regName);
    if (top->value != NULL) free(top->value);
    free(top); 
    top=p;
}

void clear() {
    printf("--INIT FREE --\n");
    while(top != NULL){
        struct Reg* p = top->next;
        printf("Freeing %s\n", top->regName);
        if (top->regName != NULL) free(top->regName); 
        if (top->value != NULL) free(top->value);
        free(top); 
        top = p;
    }
    top = NULL;
}

void dump(const char* message) {
    printf("--INIT DUMP -- %s\n", message);
    struct Reg* p = top;
    while(p != NULL){
        printf("%p Line: %d, Name: %s (%d), Type: %s, Value address: %p\n", p, p->lineOfDeclaration, p->regName, p->type, p->typeReg==NULL?"·":p->typeReg->regName, p->value);
        p = p->next;
    }
}