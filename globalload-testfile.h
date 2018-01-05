#ifndef GLOBALLOAD_TESTFILE_H
#define GLOBALLOAD_TESTFILE_H

#define FUNCS \
    F(int, Frob, (void)) \
    F(float, Warb, (float)) \

#define F(RET, NAME, ARGS) RET (*NAME) ARGS;
FUNCS
#undef F

#define F(RET, NAME, ARGS) NAME = GetLibrarySymbol(TestLib, #NAME);
#define LoadAll() FUNCS

#endif // GLOBALLOAD_TESTFILE_H
