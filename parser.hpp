#ifndef PROST_PARSER_HPP
#define PROST_PARSER_HPP

#include "string"
#include "vector"
#include "regex"

#include "symbol.hpp"

namespace parser
{

    const std::regex
        COMMENT("^(\\(.*?\\)|\\(.*|\\\\.*?\n)$"),
        STRING("^\".*?\"$"),
        CHAR("^'.$"),
        SYMBOL("^\\:([A-Za-z_]\\w+|[~#%^&*-+=|<>?/]+)$"),
        METHOD("^([A-Za-z_]\\w+|[~#%^&*-+=|<>?/]+)$"),
        CFIELD("^@([A-Za-z_]\\w+|[~#%^&*-+=|<>?/]+)$"),
        CMETHOD("^!([A-Za-z_]\\w+|[~#%^&*-+=|<>?/]+)$"),
        GFIELD("^\\$([A-Za-z_]\\w+|[~#%^&*-+=|<>?/]+)$"),
        INT("^-?\\d+$"),
        FLOAT("^-?\\d+\\.\\d+$"),
        SPACING("^\\s+$"),
        TOKEN("(\\(.*?\\))|(\\(.*)|(\\\\.*?\n)|(\".*?\")|('.)|([!@$:]?[A-Za-z_]\\w+)|(-?\\d+(\\.\\d+)?)|([!:]?[~#%^&*-+=\\|<>\\?/]+)|(\\s+)|(.)");

    class TokenTypeError : public std::exception {};
    class UnexpectedTokenError : public std::exception
    {
    private:
        char* message;
    public:
        UnexpectedTokenError(char*);
        char* what();
    };

    enum class ttype : unsigned short
    {
	ERROR,
        INT, FLOAT, CHAR, SYMBOL, STRING,
        METHOD, CMETHOD, CFIELD, GFIELD,
        OBRACKET, CBRACKET, ARRAY,
        OBRACE, CBRACE, BLOCK,
        IGNORED,
    };

    ttype guessttype(std::string src);

    std::string compstr(std::string);

    struct token
    {
        ttype type;
        int i;
        float f;
        char c;
        symbol sym;
        std::string str;

	token();
        token(ttype);
        token(ttype, symbol);
        token(int);
        token(float);
        token(char);
        token(std::string, bool);
        // TODO: Block and array
    };
    
    std::vector<token> parse(const std::string&);

} // namespace parser

#endif
