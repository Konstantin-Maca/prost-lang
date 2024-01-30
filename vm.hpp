#ifndef VM_HPP
#define VM_HPP

#include "string"
#include "vector"
#include "map"

#include "symbol.hpp"

namespace pvm
{

    typedef unsigned optr;
    typedef std::vector<optr> objvec;

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

    optr opcount = 0;
    std::map<optr, optr> objects;

    std::vector<field> fields;
    std::vector<message> messages;
    std::vector<method> methods;

    objvec ctxstack;
    objvec stack;

    optr defcopy(optr);
    unsigned reldeg(optr, optr, unsigned);

    void pvm::deffield(field);
    void deffield(optr, std::string, optr, optr);
    void setfield(optr, std::string, optr);
    field* getfield(optr, std::string);

    void defmessage(optr, std::string, unsigned);
    long long findrelmsg(optr, std::string);

    void defmethod(optr, std::string, objvec, objvec, std::vector<instr>);

    pvm::instrvec pvm::compile(std::vector<parser::token>);
    void pvm::typecheck(pvm::instrvec);

    void defstd();
    void pvm::run(pvm::instrvec);

    namespace stdo
    {
    
        optr OBJECT = 0, INT, FLOAT, CHAR, SYMBOL, ARRAY, BLOCK;
        
    } // namespace stdo

} // namespace pvm

#endif