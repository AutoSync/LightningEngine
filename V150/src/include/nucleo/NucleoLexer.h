// NucleoLexer.h — Tokeniser for C#-like Núcleo scripts.
#pragma once
#include "NucleoToken.h"
#include <vector>
#include <string>

namespace Nucleo {

class Lexer {
public:
    // Tokenise the full source string. Returns all tokens including END_OF_FILE.
    std::vector<Token> Tokenise(const std::string& source);

    // Last error message (empty if clean).
    const std::string& Error() const { return error; }

private:
    std::string  src;
    size_t       pos  = 0;
    int          line = 1;
    std::string  error;

    char  peek(int offset = 0) const;
    char  advance();
    bool  match(char c);
    void  skipWhitespaceAndComments();
    Token makeToken(TK t, const std::string& lex = "") const;

    Token readString();
    Token readChar();
    Token readNumber();
    Token readIdentOrKeyword();
};

} // namespace Nucleo
