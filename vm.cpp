#include "vm.hpp"

#include "parser.hpp"
#include "symbol.hpp"

pvm::NoFieldError::NoFieldError(pvm::optr owner, symbol name) : owner(owner), name(name) {}
char* pvm::NoFieldError::what()
{
    std::string msg("NoFieldError: owner=" + std::to_string(owner) + ", name=`" + name + "'");
    return msg.data();
}

pvm::NoMessageError::NoMessageError(pvm::optr owner, symbol name) : owner(owner), name(name) {}
char* pvm::NoMessageError::what()
{
    std::string msg("NoMessageError: owner=" + std::to_string(owner) + ", name=`" + name + "'");
    return msg.data();
}

pvm::NoMethodError::NoMethodError(pvm::optr owner, symbol name, pvm::objvec params, pvm::objvec args)
    : owner(owner), name(name), params(params), args(args) {}
char* pvm::NoMethodError::what()
{
    std::string msg(
        "NoMethodError: owner=" + std::to_string(owner) + ", name=`" + name + "', params=[ "
    );
    for (auto& p : params) msg += " " + std::to_string(p) + " ";
    msg += "], args=[ ";
    for (auto& a : args) msg += " " + std::to_string(a) + " ";
    msg += "]";
    return msg.data();
}

pvm::NoObjectRelationError::NoObjectRelationError(pvm::optr owner, symbol name, pvm::optr prototype, pvm::optr value)
    : owner(owner), name(name), prototype(prototype), value(value) {}
char* pvm::NoObjectRelationError::what()
{
    std::string msg(
        "NoObjectRelationError: owner=" + std::to_string(owner)
        + ", name=`" + name + "', prototype=" + std::to_string(prototype)
        + ", value=" + std::to_string(value)
    );
    return msg.data();
}

pvm::UnableRedefineMessageError::UnableRedefineMessageError(pvm::optr owner, symbol name, unsigned argc)
    : owner(owner), name(name), argc(argc) { }
char* pvm::UnableRedefineMessageError::what()
{
    std::string msg("UnableRedefineMessageError: owner=" + std::to_string(owner) + ", name=`" + name + "', argc=" + std::to_string(argc));
    return msg.data();
}

pvm::DifferentParametersNumberError::DifferentParametersNumberError(pvm::optr owner, symbol name, unsigned msgargc, unsigned mtdargc)
    : owner(owner), name(name), message_argc(msgargc), method_argc(mtdargc) {}
char* pvm::DifferentParametersNumberError::what()
{
    std::string msg(
        "UnableRedefineMessageError: owner=" + std::to_string(owner) + ", name=`" + name
        + "', message_argc=" + std::to_string(message_argc) + ", method_argc=" + std::to_string(method_argc)
    );
    return msg.data();
}

pvm::instruction::instruction(pvm::instr kind) : kind(kind) { }
pvm::instruction::instruction(pvm::instr kind, symbol sym) : kind(kind), sym(sym) { }
pvm::instruction::instruction(int value) : kind(instr::INT), i(value) { }
pvm::instruction::instruction(float value) : kind(instr::FLOAT), f(value) { }
pvm::instruction::instruction(char value) : kind(instr::CHAR), c(value) { }
pvm::instruction::instruction(symbol value) : kind(instr::SYMBOL), sym(value) { }

pvm::field::field(optr owner, std::string name, optr prototype, optr value)
    : owner(owner), name(name), prototype(prototype), value_ptr(value) { }
pvm::field::field(optr owner, std::string name, int value) : owner(owner), name(name), i(value) { }
pvm::field::field(optr owner, std::string name, float value) : owner(owner), name(name), f(value) { }
pvm::field::field(optr owner, std::string name, char value) : owner(owner), name(name), c(value) { }
pvm::field::field(optr owner, std::string name, symbol value) : owner(owner), name(name), sym(value) { }

pvm::message::message(optr owner, std::string name, unsigned argc)
    : owner(owner), name(name), argc(argc) { }

pvm::method::method(optr owner, std::string name, pvm::objvec args, pvm::objvec ret, pvm::instrvec body)
    : owner(owner), name(name), args(args), ret(ret), body(body) { }
bool pvm::method::same_args(pvm::objvec other_args)
{
    if (args.size() != other_args.size()) return false;
    for (unsigned i = 0; i < args.size(); i++)
        if (args[i] != other_args[i]) return false;
    return true;
}

pvm::optr defcopy(pvm::optr parent)
{
    pvm::objects.insert(pvm::opcount, parent);
    return pvm::opcount++;
}
unsigned pvm::reldeg(pvm::optr object, pvm::optr super_object, unsigned d = 1)
{
    if (object == super_object) return d;
    if (object == 0) return 0;
    auto it = objects.find(object);
    if (it == objects.end()) return 0;
    return reldeg(it->second, super_object);
}

void pvm::deffield(pvm::field fld)
{
    for (auto& f : fields)
    {
        if (f.owner == fld.owner && f.name == fld.name)
        {
            f.prototype = fld.prototype;
            f.value_ptr = fld.value_ptr;
            f.i = fld.i;
            f.f = fld.f;
            f.c = fld.c;
            f.sym = fld.sym;
            return;
        }
    }
    fields.push_back(fld);
}
void pvm::deffield(pvm::optr owner, std::string name, pvm::optr prototype, pvm::optr value)
{
    for (auto& f : fields)
    {
        if (f.owner == owner && f.name == name)
        {
            f.prototype = prototype;
            f.value_ptr = value;
            return;
        }
    }
    fields.push_back(field(owner, name, prototype, value));
}
void pvm::setfield(pvm::optr owner, std::string name, pvm::optr value)
{
    for (auto& f : fields)
    {
        if (f.owner == owner && f.name == name)
        {
            if (pvm::reldeg(value, f.prototype)) f.value_ptr = value;
            else throw NoObjectRelationError(owner, name, value, f.prototype);
            return;
        }
    }
    if (owner)
    {
        auto it = objects.find(owner);
        if (it == objects.end()) throw NoFieldError(owner, name);
        try { return setfield(it->second, name, value); }
        catch (const NoFieldError& e) { throw NoFieldError(owner, name); }
    }
    throw NoFieldError(owner, name);
}
pvm::field* pvm::getfield(pvm::optr owner, std::string name)
{
    for (auto& f : fields)
        if (f.owner == owner && f.name == name) return &f;
    if (owner)
    {
        auto it = objects.find(owner);
        if (it == objects.end()) throw NoFieldError(owner, name);
        try { return getfield(it->second, name); }
        catch (const NoFieldError& e) { throw NoFieldError(owner, name); }
    }
    throw NoFieldError(owner, name);
}

void pvm::defmessage(pvm::optr owner, std::string name, unsigned argc)
{
    for (auto& msg : messages)
    {
        if (msg.owner == owner && msg.name == name)
        {
            if (msg.argc == argc) return;
            for (auto& mtd : methods)
                if (mtd.owner == owner && mtd.name == name) throw UnableRedefineMessageError(owner, name, argc);
            msg.argc = argc;
            return;
        }
    }
    messages.push_back(message(owner, name, argc));
}
long long pvm::findrelmsg(optr owner, std::string name)
{
    for (unsigned i = 0; i < messages.size(); i++)
        if (messages[i].owner == owner && messages[i].name == name) return i;
    return -1;
}

void pvm::defmethod(optr owner, std::string name, pvm::objvec args, pvm::objvec ret, pvm::instrvec body)
{
    long long msgi = findrelmsg(owner, name);
    if (msgi == -1) throw NoMessageError(owner, name);
    if (messages[msgi].argc != args.size()) throw DifferentParametersNumberError(owner, name, messages[msgi].argc, args.size());
    if (messages[msgi].owner == owner)
    {
        for (auto& mtd : methods)
        {
            if (mtd.owner == owner && mtd.name == name && mtd.args.size() == args.size() && mtd.same_args(args))
            {
                mtd.ret = ret;
                mtd.body = body;
                return;
            }
        }
        methods.push_back(method(owner, name, args, ret, body));
    }
    else
    {
        messages.push_back(message(owner, name, messages[msgi].argc));
        methods.push_back(method(owner, name, args, ret, body));
    }
}

void pvm::defstd()
{
    objects.insert(opcount++, 0u);
    defmessage(0, "copy", 0);
    defmethod(0, "copy", objvec(0), objvec { 0u }, instrvec { instruction(instr::COPY) });
}

pvm::instrvec pvm::compile(std::vector<parser::token> tokens)
{
    instrvec bytecode(0);
    for (auto& token : tokens)
    {
        switch (token.type)
        {
        case parser::ttype::INT:
            bytecode.push_back(instruction(token.i));
            break;
        case parser::ttype::FLOAT:
            bytecode.push_back(instruction(token.f));
            break;
        case parser::ttype::CHAR:
            bytecode.push_back(instruction(token.c));
            break;
        case parser::ttype::SYMBOL:
            bytecode.push_back(instruction(token.sym));
            break;
        case parser::ttype::METHOD:
            bytecode.push_back(instruction(pvm::instr::CALL, token.sym));
            break;
        default:
            break;
        }
    }
    return bytecode;
}

void pvm::typecheck(pvm::instrvec bytecode);

void pvm::run(pvm::instrvec bytecode);
