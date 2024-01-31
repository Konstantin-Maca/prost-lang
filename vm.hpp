#ifndef PROST_VM_HPP
#define PROST_VM_HPP

#include "string"
#include "vector"
#include "map"

#include "symbol.hpp"
#include "parser.hpp"

namespace pvm
{

    typedef unsigned optr;
    typedef std::vector<optr> objvec;
    typedef std::pair<optr, optr> opair;

    class ProstError : public std::exception {};
    class NoFieldError : public ProstError
    {
    private:
        optr owner;
        symbol name;
    public:
        NoFieldError(optr, symbol);
        char* what();
    };
    class NoMessageError : public ProstError
    {
    private:
        optr owner;
        symbol name;
    public:
        NoMessageError(optr, symbol);
        char* what();
    };
    class NoMethodError : public ProstError
    {
    private:
        optr owner;
        symbol name;
        objvec params, args;
    public:
        NoMethodError(optr, symbol, objvec, objvec);
        char* what();
    };
    class NoObjectRelationError : public ProstError
    {
    private:
        optr owner;
        symbol name;
        optr prototype, value;
    public:
        NoObjectRelationError(optr, symbol, optr, optr);
        char* what();
    };
    class UnableRedefineMessageError : public ProstError
    {
    private:
        optr owner;
        symbol name;
        unsigned argc;
    public:
        UnableRedefineMessageError(optr, symbol, unsigned);
        char* what();
    };
    class DifferentParametersNumberError : public ProstError
    {
    private:
        optr owner;
        symbol name;
        unsigned message_argc, method_argc;
    public:
        DifferentParametersNumberError(optr, symbol, unsigned, unsigned);
        char* what();
    };

    enum class instr
    {
        COPY,
        INT, FLOAT, CHAR, SYMBOL,
        DEFF, SETF, GETF,
        MSG, MTD, CALL,
    };

    struct instruction
    {
        instr kind;
        int i;
        float f;
        char c;
        symbol sym;

	instruction();
        instruction(instr);
        instruction(instr, symbol);
        instruction(int);
        instruction(float);
        instruction(char);
        instruction(symbol);
    };
    
    typedef std::vector<instruction> instrvec;

    struct field
    {
        optr owner;
        std::string name;
        optr prototype, value_ptr;
        int i;
        float f;
        char c;
        symbol sym;

        field(optr, std::string, optr, optr);
        field(optr, std::string, int);
        field(optr, std::string, float);
        field(optr, std::string, char);
        field(optr, std::string, symbol);
    };

    struct message
    {
        optr owner;
        std::string name;
        unsigned argc;

        message(optr, std::string, optr);
    };

    struct method
    {
        optr owner;
        std::string name;
        objvec args;
        objvec ret;
        instrvec body;

        method(optr, std::string, objvec, objvec, instrvec);
        bool same_args(objvec);
    };
    
    optr defcopy(optr);
    unsigned reldeg(optr, optr, unsigned);

    void deffield(field);
    void deffield(optr, std::string, optr, optr);
    void setfield(optr, std::string, optr);
    field* getfield(optr, std::string);

    void defmessage(optr, std::string, unsigned);
    long long findrelmsg(optr, std::string);

    void defmethod(optr, symbol, objvec, objvec, instrvec);

    instrvec compile(std::vector<parser::token>);
    void typecheck(pvm::instrvec);

    void defstd();
    void run(pvm::instrvec);

} // namespace pvm

#endif
