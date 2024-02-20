#ifndef PROSTVM_H
#define PROSTVM_H

#include <string>
#include <vector>
#include <map>

namespace pvm
{

    struct fkey
    {
        std::string name;
        unsigned owner;
        void repr() const;
    };
    bool operator==(const fkey& a, const fkey& b);
    bool operator<(const fkey& a, const fkey& b);
    template <typename T>
    struct sizedarr
    {
        long unsigned size = 0;
        T* data = new T[0];
        sizedarr();
        sizedarr(std::vector<T> v);
    };
    union fvaluedata { unsigned u; int i; char c; float f; sizedarr<unsigned> a; };
    struct fvalue
    {
        unsigned ptype;
        fvaluedata value;
        void repr() const;
    };

    struct mkey
    {
        std::string name;
        unsigned owner;
        sizedarr<unsigned> arg_ptypes;
        void repr() const;
    };
    bool operator==(const mkey& a, const mkey& b);
    bool operator<(const mkey& a, const mkey& b);
    struct mvalue
    {
        sizedarr<unsigned> ret_ptypes;
        unsigned block_id;
        void repr() const;
    };

}

#endif
