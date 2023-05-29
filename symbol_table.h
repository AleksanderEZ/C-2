enum RegType { type, globalVariable, localVariable, function };

struct Reg {
    char* regName;
    enum RegType type;
    struct Reg* typeReg;
    void* value;
    int lineOfDeclaration;
    struct Reg* next;
};

struct Reg* getTop();
struct Reg* search(char* regName);
struct Reg* searchRegType(char* id, enum RegType regType);
void newReg(char* regName, enum RegType type, struct Reg* typeReg, int line);
void newRegSetValue(char* regName, enum RegType type, struct Reg* typeReg, int line, void* value);
void setRegValue(char* regName, enum RegType localGlobal, void* value);
void* getRegValue(char* regName, enum RegType localGlobal);

void closeBlock();
void clear();
void dump(const char* message);