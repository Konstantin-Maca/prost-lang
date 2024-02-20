#include "prostvm.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>

namespace pvm
{

    template<typename T>
    sizedarr<T>::sizedarr() : size(), data(new T[0]) { }
    template<typename T>
    sizedarr<T>::sizedarr(std::vector<T> v) : size(v.size()), data(v.data()) { }
        
    void fkey::repr() const { std::printf("[%s\t:%u\t]", name.data(), owner); }

    void fvalue::repr() const { std::printf("{%u\t:%u\t|%i\t|%c\t|%f|[...]}", ptype, value.u, value.i, value.c, value.f); }

    void mkey::repr() const
    {
        std::printf("(%s\t:%u\t:[", name.data(), owner);
        for (unsigned i = 0; i < arg_ptypes.size; i++) std::cout << arg_ptypes.data[i] << ",\t";
        std::cout << "]\t)";
    }

    void mvalue::repr() const
    {
        std::printf("{[");
        for (unsigned i = 0; i < ret_ptypes.size; i++) std::cout << ret_ptypes.data[i] << ",\t";
        std::printf("]\t->%u\t}", block_id);
    }

    bool operator==(const fkey& a, const fkey& b)
    {
        //std::printf("[ %s : %u ] == [ %s : %u ]\n", a.name.data(), a.owner, b.name.data(), b.owner);
        return a.owner == b.owner && a.name == b.name;
    }
    bool operator<(const fkey& a, const fkey& b)
    {
        bool r = a.owner < b.owner || a.name < b.name;
        //std:printf("[ %s : %u ] < [ %s : %u ] => %i\n", a.name.data(), a.owner, b.name.data(), b.owner, r);
        return r;
    }
    bool operator==(const mkey& a, const mkey& b) { return a.owner == b.owner && a.name == b.name; }
    bool operator<(const mkey& a, const mkey& b) { return a.owner < b.owner || a.name < b.name; }

}


// See actual byte-code instructions in translator.rb
const unsigned short BC_VER[3] = { 0, 3, 0 };


bool is_parameterized(std::string s)
{
    std::string ss[15] = { "int", "char", "float", "push", "rfld", "sfld", "sifld", "scfld", "sffld", "safld", "gfls", "rmtd", "call", "ccall", "block" };
    for (unsigned i = 0; i < 15; i++) if (s == ss[i]) return true;
    return false;
}


// Registers
unsigned pc = 0;
int ir;
float fr;
pvm::sizedarr<unsigned> ar;

// Stacks
std::vector<std::vector<unsigned>> stack;
std::vector<unsigned> cstack;
std::vector<unsigned> rstack;

// Dictiomaries
std::map<unsigned, unsigned> odict;
std::map<unsigned, unsigned> bdict;
std::map<pvm::fkey, pvm::fvalue> fdict;
std::map<pvm::mkey, pvm::mvalue> mdict;


void print_vm_state()
{
    std::cout << "\tPC=" << pc << "\tIR=" << ir << "\tFR=" << fr << "\n"
        << "\tSTACK\t=\t";
    for (auto& i : stack)
    {
        std::cout << "[ ";
        for (auto& j : i) std::cout << j << " ";
        std::cout << "]\n\t\t\t";
    }

    std::cout << "\r\tCONTEXT\t=\t[ ";
    for (auto& i : cstack) std::cout << i << " ";

    std::cout << "]\n\tRETURN\t=\t[ ";
    for (auto& i : rstack) std::cout << i << " ";

    std::cout << "]\n\tOBJECTS\t=\t{ ";
    for (auto const& [p, p0] : odict) std::cout << p << "->" << p0 << " ";

    std::cout << "}\n\tBLOCKS\t=\t{ ";
    for (auto const& [b, i] : bdict) std::cout << b << "->" << i << " ";

    std::cout << "}\n\tFIELDS\t=\t";
    for (auto const& [k, v] : fdict)
    {
        k.repr(); std::cout << " -> "; v.repr(); std::cout << "\n\t\t\t";
    }

    std::cout << "\n\tMETHODS\t=\t";
    for (auto const& [k, v] : mdict)
    {
        k.repr(); std::cout << " -> "; v.repr(); std::cout << "\n\t\t\t";
    }
    std::cout << '\n';
}

unsigned last_stack()
{
    for (int i = stack.size() - 1; i >= 0; i--)
        if (stack[i].size() != 0) return stack[i].back();
    std::cout << "Failed to pop value from empty stack";
    exit(0);
}
unsigned pop_stack()
{
    //std::cout << "Trying to pop value from stack with length " << stack.size() << "\n";
    for (int i = stack.size() - 1; i >= 0; i--)
    {
        //std::cout << "Checking substack #" << i << '\n';
        if (stack[i].size() != 0)
        {
            auto a = stack[i].back();
            stack[i].pop_back();
            return a;
        }
    }
    std::puts("Failed to pop value from empty stack");
    exit(0);
}

unsigned findpar(unsigned p)
{
    auto it = odict.find(p);
    if (it == odict.end())
    {
        std::cout << "Failed to find parent of object #" << p << "\n";
        exit(0);
    }
    return it->second;
}

pvm::fvalue& findf(const pvm::fkey& k)
{
    auto it = fdict.find(k);
    if (it == fdict.end())
    {
        std::cout << "Failed to find field named `" << k.name << "' for object #" << k.owner << " (straight)\n";
        exit(0);
    }
    return it->second;
}
pvm::fvalue& findfr(const pvm::fkey& k)
{
    //auto it = fdict.find(k);
    auto it = fdict.begin();
    for(; it != fdict.end(); it++)
        if (it->first.name == k.name && it->first.owner == k.owner) break;
    if (it == fdict.end())
    {
        if (k.owner == 0)
        {
            std::cout << "Failed to find field named `" << k.name << "' for object #" << k.owner << " (related)\n";
            exit(0);
        }
        else return findfr(pvm::fkey { k.name, findpar(k.owner) });
    }
    return it->second;
}

pvm::mvalue& findm(const pvm::mkey& k)
{
    auto it = mdict.find(k);
    if (it == mdict.end())
    {
        std::cout << "Failed to find a method named `" << k.name << "' for object #" << k.owner << " (straight)\n";
        exit(0);
    }
    return it->second;
}
pvm::mvalue& findmr(const pvm::mkey k)
{
    auto it = mdict.find(k);
    if (it == mdict.end())
    {
        if (k.owner == 0)
        {
            std::cout << "Failed to find a method named `" << k.name << "' for object #" << k.owner << " (related)\n";
            exit(0);
        }
        else return findmr(pvm::mkey { k.name, findpar(k.owner), k.arg_ptypes });
    }
    return it->second;
}
pvm::mvalue& findmc(const std::string& name, std::vector<unsigned>::iterator cit)
{
    auto it = mdict.find(pvm::mkey { name, *cit, { } });
    if (it == mdict.end())
    {
        if (cit == cstack.begin())
        {
            std::cout << "Failed to find a method named `" << name << "' for context\n";
            exit(0);
        }
        return findmc(name, cit - 1);
    }
    return it->second;
}


namespace params
{
    bool debug = false, debug_builtins = false, debug_stack = false;
    char* filepath;
}

const char HELP_MESSAGE[] =
"prostvm [--help | -h] [--debug | -d] [file.pbc]\n\n"
"        --help | -h             : Print this message.\n"
"        --debug | -d            : Print each instruction while executing prost byte-code and state of registers, stacks, and dictionaries at the end of executing.\n"
"        --debug-state | -ds     : Print state of registers, stacks, and dictionaries after each command executing.\n"
"        --debug-builtins | -dbi : Do all --debug* commands for executing standard built-in files.\n"
;

void parse_params(int argc, char* args[])
{
    std::string s;
    int i = 1;
    for (; i < argc; i++)
    {
        s = std::string { args[i] };
        if (s == "--help" || s =="-h")
        {
            std::printf(HELP_MESSAGE);
            exit(0);
        }
        else if (s == "--debug" | s == "-d") params::debug = true;
        else if (s == "--debug-builtins" | s == "-dbi") params::debug_builtins = true;
        else if (s == "--debug-stack" | s == "-ds") params::debug_stack = true;
        else break;
    }

    if (i == argc)
    {
        std::printf(HELP_MESSAGE);
        exit(0);
    }
    params::filepath = args[i++];

    // TODO: Parse parameters for executing program
}


void execute_bytecode(std::ifstream& file, char filename[])
{
    std::string instr = "", name = "";
    unsigned u;
    long unsigned pos; char c;

    stack.push_back(std::vector<unsigned> { });
    while (file && file.peek() != std::ifstream::traits_type::eof())
    {
        pos = file.tellg();
        file >> instr;
        if (instr == "$") break;
        else if (instr[0] == ';')
        {
            do { file.get(c); } while (c != '\n');
            continue;
        }
        if (params::debug) std::printf("[%lu]\t%s\n", pos, instr.data());
        if (instr == "int")
        {
            file >> ir;
            if (params::debug) std::printf("\t%u\n", ir);
        }
        else if (instr == "char")
        {
            file >> ir;
            if (params::debug) std::printf("\t%u\n", ir);
        }
        else if (instr == "float")
        {
            file >> fr;
            if (params::debug) std::printf("\t%u\n", ir);
        }
        else if (instr == "arr")
        {
            auto arr_vec = stack.back();
            stack.pop_back();
            ar = { arr_vec };
        }
        else if (instr == "push")
        {
            file >> u;
            if (params::debug) std::printf("\t%u\n", u);
            stack.back().push_back(u);
        }
        else if (instr == "pushc") cstack.push_back(pop_stack());
        else if (instr == "pusha") stack.push_back(std::vector<unsigned>(0));
        else if (instr == "copy")
        {
            odict.insert({ pc, pop_stack() });
            stack.back().push_back(pc++);
        }
        else if (instr == "deff")
        {
            // Stack: ..., ptype, owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            unsigned owner = pop_stack(),
                     ptype = pop_stack();
            fdict.insert({ { name, owner }, { ptype, { .u = 0 } }});
        }
        else if (instr == "setf")
        {
            // Stack: ..., value, owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            unsigned owner = pop_stack(),
                     value = pop_stack();
            findfr(pvm::fkey { name, owner }).value.u = value;
        }
        else if (instr == "setfi")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            findfr(pvm::fkey { name, pop_stack() }).value.i = ir;
        }
        else if (instr == "setfc")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            pvm::fvalue& fv = findfr(pvm::fkey { name, pop_stack() });
            fv.value.i = 0;
            fv.value.c = (char)ir;
        }
        else if (instr == "setff")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            findfr(pvm::fkey { name, pop_stack() }).value.f = fr;
        }
        else if (instr == "setfa")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            findfr(pvm::fkey { name, pop_stack() }).value.a = ar;
        }
        else if (instr == "getf")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            stack.back().push_back(findfr(pvm::fkey { name, pop_stack() }).value.u);
        }
        else if (instr == "getfi")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            ir = findfr(pvm::fkey { name, pop_stack() }).value.i;
        }
        else if (instr == "getfc")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            ir = findfr(pvm::fkey { name, pop_stack() }).value.c;
        }
        else if (instr == "getff")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            fr = findfr(pvm::fkey { name, pop_stack() }).value.f;
        }
        else if (instr == "addfi")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            ir += findfr(pvm::fkey { name, pop_stack() }).value.i;
        }
        else if (instr == "subfi")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            ir -= findfr(pvm::fkey { name, pop_stack() }).value.i;
        }
        else if (instr == "mulfi")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            ir *= findfr(pvm::fkey { name, pop_stack() }).value.i;
        }
        else if (instr == "defm")
        {
            // Stack: ..., block, rets, args, owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            unsigned owner = pop_stack(),
                     args_obj = pop_stack(),
                     rets_obj = pop_stack(),
                     block_id = pop_stack();
            mdict.insert({
                    { name, owner, findfr({ "@", args_obj }).value.a },
                    { findfr({ "@", rets_obj }).value.a, block_id }
                });
        }
        else if (instr == "call")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            auto owner = pop_stack();
            unsigned id = findmr({ name, owner, { } }).block_id;
            auto it = bdict.find(id);
            if (it == bdict.end())
            {
                std::cout << "[FATAL] Failed to find block with id #" << id << "\n";
                exit(0);
            }
            file.seekg(it->second);
        }
        else if (instr == "ccall")
        {
            // Stack: ..., owner
            file >> name;
            if (params::debug) std::printf("\t%s\n", name.data());
            unsigned id = findmc(name, cstack.end() - 1).block_id;
            auto it = bdict.find(id);
            if (it == bdict.end())
            {
                std::cout << "[FATAL] Failed to find block with id #" << id << "\n";
                exit(0);
            }
        }
        else if (instr == "ctx") stack.back().push_back(cstack.back());
        else if (instr == "gtx") stack.back().push_back(cstack.front());
        else if (instr == "block")
        {
            file >> u;
            if (params::debug) std::printf("\t%u\n", u);
            bdict.insert({ u, pos });
            unsigned v;
            while (1)
            {
                file >> instr;
                if (instr == "block")
                {
                    file >> v;
                    if (u == v) break;
                    continue;
                }
                if (is_parameterized(instr)) file >> instr;
            }
        }
        else if (instr == "ret")
        {
            cstack.pop_back();
            file.seekg(rstack.back());
            rstack.pop_back();
        }
        else if (instr == "dup") stack.back().push_back(last_stack());
        else if (instr == "swap")
        {
            unsigned a = pop_stack(), b = pop_stack();
            stack.back().push_back(a);
            stack.back().push_back(b);
        }
        else if (instr == "over")
        {
            // TODO
        }
        else if (instr == "rot")
        {
            // TODO
        }
        else {
            std::printf("[FATAL] Unexpected byte-code instruction executing file `%s': %s\n\tVM's bytecode version: %u.%u.%u\n", filename, instr.data(), BC_VER[0], BC_VER[1], BC_VER[2]);
            file.close();
            std::exit(0);
        }
        if (params::debug && params::debug_stack) print_vm_state();
    }
}


const char STDDIR[] = "/home/azzimoda/Projects/prost-lang-rb/stdlib/";

int main(int argc, char* args[])
{
    parse_params(argc, args);
    std::printf("DEBUG=%u DEBUG_STACK=%u DEBUG_BUILTINS=%u\n", params::debug, params::debug_stack, params::debug_builtins);

    // Standard file
    std::ifstream std_file(std::string { STDDIR } + "std.pbc");
    if (!std_file.is_open())
    {
        std::printf("[FATAL] Failed to open standard file `%s'\n", (std::string { STDDIR } + "std.pbc").data());
        exit(0);
    }

    std::ifstream file(params::filepath);
    if (!file.is_open())
    {
        std::printf("Failed to open file `%s'\n", params::filepath);
        exit(0);
    }
    
    std::printf("DEBUG=%u DEBUG_STACK=%u DEBUG_BUILTINS=%u\n", params::debug, params::debug_stack, params::debug_builtins);
    //bool d = params::debug;
    //params::debug = params::debug && params::debug_builtins;
    execute_bytecode(std_file, "std.pbc");
    //params::debug = d;
    if (params::debug && params::debug_builtins) print_vm_state();
    
    std::puts("Start executing...");
    std::printf("DEBUG=%u DEBUG_STACK=%u DEBUG_BUILTINS=%u\n", params::debug, params::debug_stack, params::debug_builtins);
    execute_bytecode(file, params::filepath);
    if (params::debug) print_vm_state();
    
    file.close();
    return 0;
}

