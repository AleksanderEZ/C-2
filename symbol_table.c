#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void yyerror(char* message);
extern int yylineno;

struct Reg* top = NULL;

struct Reg* search(char* regName) {
    if (top == NULL) return NULL;
    struct Reg* p = top;
    while (p->next != NULL) {
        p = p->next;
    }
    while(p != NULL && strcmp(p->regName, regName) != 0) p = p->previous;
    return p;
}

struct Reg* getTop() {
    return top;
}

struct Reg* searchRegType(char* regName, enum RegType regType) {
    struct Reg *p = search(regName);
    if(p != NULL && p->type==regType) return p; else return NULL;
}

void newReg(char* regName, enum RegType regType, struct Reg* typeReg, int line) {
    if(searchRegType(regName, regType) != NULL && searchRegType(regName, type) != NULL && strcmp(regName, "openBlock") != 0) yyerror("Name already defined");
    struct Reg* new = (struct Reg*) malloc(sizeof(struct Reg));
    new->regName = regName;
    new->type = regType;
    new->value = 0x24001;
    if(strcmp(regName, "openBlock") != 0) {
        new->typeReg = typeReg;
        new->lineOfDeclaration = line;
    }
    if (top == NULL) {
        top = new;
    } else {
        struct Reg* p = top;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = new;
        new->previous = p;
    }
}

void newParameter(char* regName, struct Reg* typeReg, int line) {
    struct Reg* new = (struct Reg*) malloc(sizeof(struct Reg));
    new->regName = regName;
    new->type = parameter;
    new->value = 0x24001;
    new->typeReg = typeReg;
    new->lineOfDeclaration = line;
    
    if (top == NULL) {
        top = new;
    } else {
        struct Reg* p = top;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = new;
        new->previous = p;
    }
}

void newFunction(char* regName, enum RegType type, struct Reg* typeReg, int line, int nParameters) {
    newReg(regName, type, typeReg, line);
    struct Reg* p = top;
    while (p->next != NULL) {
        p = p->next;
    }
    p->nParameters = nParameters;
}

void removeLast() {
    struct Reg *p = top;
    while(p->next->next != NULL){
        p = p->next;
    }
    if (p->next->regName != NULL) free(p->next->regName);
    free(p->next);
    p->next = NULL;
}

void closeBlock() {
    dump("--CLOSING BLOCK--\n");
    printf("In line %d\n", yylineno);
    startClosingBlock();
}

void startClosingBlock() {
    if (top == NULL || top->next == NULL) yyerror("ST Empty");
    struct Reg *p = top;
    while(p->next->next != NULL){
        p = p->next;
    }
    if(p->next->type == function && strcmp(p->next->regName, "openBlock") == 0) {
        if (p->next->regName != NULL) free(p->next->regName);
        free(p->next);
        p->next = NULL;
    } else {
        removeLast();
        startClosingBlock();
    }
}

void clear() {
    printf("--INIT FREE --\n");
    while(top != NULL){
        struct Reg* p = top->next;
        printf("Freeing %s\n", top->regName);
        if (top->regName != NULL) free(top->regName); 
        free(top); 
        top = p;
    }
    top = NULL;
}

void dump(const char* message) {
    printf("--INIT DUMP -- %s\n", message);
    struct Reg* p = top;
    while(p != NULL){
        printf("%p Line: %d, Name: %s (%d), Type: %s, Value address: 0x%x\n", p, p->lineOfDeclaration, p->regName, p->type, p->typeReg==NULL?"·":p->typeReg->regName, p->value);
        p = p->next;
    }
}

int checkValueExists(int value) {
    struct Reg* p = getTop();
    while(p != NULL) {
        if (p->value == value) return 1;
        p=p->next;
    }
    return 0;
}
        