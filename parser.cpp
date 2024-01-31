#include "parser.hpp"

parser::token::token() : type(parser::ttype::ERROR) { }
parser::token::token(ttype type) : type(type) { }
parser::token::token(ttype type, symbol sym) : type(type), sym(sym) { }
parser::token::token(int value) : type(parser::ttype::INT), i(value) { }
parser::token::token(float value) : type(parser::ttype::FLOAT) { }
parser::token::token(char value) : type(parser::ttype::CHAR) { }
parser::token::token(std::string value, bool sym = false) : type(parser::ttype::SYMBOL) { }
// TODO: Block and array

parser::UnexpectedTokenError::UnexpectedTokenError(char* msg) : message(msg) {}
char* parser::UnexpectedTokenError::what() { return message; }

parser::ttype parser::guessttype(std::string source)
{

    if (source == "[") return ttype::OBRACE;
    if (source == "]") return ttype::CBRACE;
    if (std::regex_match(source, COMMENT)) return ttype::IGNORED ;
    if (std::regex_match(source, SPACING)) return ttype::IGNORED;
    if (std::regex_match(source, INT)) return ttype::INT;
    if (std::regex_match(source, FLOAT)) return ttype::FLOAT;
    if (std::regex_match(source, CHAR)) return ttype::CHAR;
    if (std::regex_match(source, STRING)) return ttype::STRING;
    if (std::regex_match(source, METHOD)) return ttype::METHOD;
    if (std::regex_match(source, CMETHOD)) return ttype::CMETHOD;
    if (std::regex_match(source, CFIELD)) return ttype::CFIELD;
    if (std::regex_match(source, GFIELD)) return ttype::GFIELD;
    if (std::regex_match(source, SYMBOL)) return ttype::SYMBOL;
    throw TokenTypeError();
    exit(1);
}

std::string compstr(std::string src)
{
    auto chars = std::vector<char>();
    bool repr = false;
    char c;
    for (unsigned i = 0; i < src.size() - 2; i++)
    {

        c = src[i + 1];
        if (repr)
        {
            switch (c)
            {
            case '\\':
                chars.push_back('\\');
                break;
            case '"':
                chars.push_back('"');
                break;
            case 'n':
                chars.push_back('\n');
                break;
            case 'r':
                chars.push_back('\r');
                break;
            case 't':
                chars.push_back('\t');
                break;
            default:
                chars.push_back('\\');
                chars.push_back(c);
                break;
            }
            repr = false;
        }
        else if (c == '\\') repr = true;
        else chars.push_back(c);
    }
    char str[chars.size()];
    for (unsigned i = 0; i < chars.size(); i++) str[i] = chars[i];
    return str;
}

std::vector<parser::token> parser::parse(const std::string& source)
{
    std::vector<token> tokens(0);
    std::smatch match;
    while (source.size() > 0 && std::regex_search(source, match, TOKEN))
    {
        ttype type = guessttype(match.str());
    }
    return tokens;
}
