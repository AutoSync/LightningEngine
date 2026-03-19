#include "../../include/nucleo/NucleoLexer.h"
#include <cctype>
#include <unordered_map>

namespace Nucleo {

static const std::unordered_map<std::string, TK> kKeywords = {
    {"int",       TK::KW_INT},    {"float",   TK::KW_FLOAT},
    {"bool",      TK::KW_BOOL},   {"string",  TK::KW_STRING},
    {"void",      TK::KW_VOID},   {"var",     TK::KW_VAR},
    {"if",        TK::KW_IF},     {"else",    TK::KW_ELSE},
    {"while",     TK::KW_WHILE},  {"for",     TK::KW_FOR},
    {"foreach",   TK::KW_FOREACH},{"in",      TK::KW_IN},
    {"return",    TK::KW_RETURN}, {"break",   TK::KW_BREAK},
    {"continue",  TK::KW_CONTINUE},
    {"class",     TK::KW_CLASS},  {"new",     TK::KW_NEW},
    {"this",      TK::KW_THIS},   {"base",    TK::KW_BASE},
    {"public",    TK::KW_PUBLIC}, {"private", TK::KW_PRIVATE},
    {"protected", TK::KW_PROTECTED},
    {"static",    TK::KW_STATIC}, {"override",TK::KW_OVERRIDE},
    {"true",      TK::TRUE},      {"false",   TK::FALSE},
    {"null",      TK::NULL_LIT},
};

// ─────────────────────────────────────────────────────────────────────────────

char Lexer::peek(int offset) const
{
    size_t i = pos + offset;
    return i < src.size() ? src[i] : '\0';
}

char Lexer::advance()
{
    char c = src[pos++];
    if (c == '\n') ++line;
    return c;
}

bool Lexer::match(char c)
{
    if (pos < src.size() && src[pos] == c) { ++pos; return true; }
    return false;
}

Token Lexer::makeToken(TK t, const std::string& lex) const
{
    Token tk; tk.type = t; tk.lexeme = lex; tk.line = line;
    return tk;
}

void Lexer::skipWhitespaceAndComments()
{
    while (pos < src.size()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            while (pos < src.size() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            advance(); advance();
            while (pos + 1 < src.size() && !(peek() == '*' && peek(1) == '/'))
                advance();
            if (pos + 1 < src.size()) { advance(); advance(); }
        } else break;
    }
}

Token Lexer::readString()
{
    std::string val;
    while (pos < src.size() && peek() != '"') {
        char c = advance();
        if (c == '\\') {
            char e = advance();
            switch (e) {
            case 'n': val += '\n'; break;
            case 't': val += '\t'; break;
            case '\\': val += '\\'; break;
            case '"':  val += '"';  break;
            default:   val += e;   break;
            }
        } else val += c;
    }
    if (pos < src.size()) advance(); // closing "
    return makeToken(TK::STRING_LIT, val);
}

Token Lexer::readChar()
{
    std::string val;
    if (pos < src.size() && peek() != '\'') val += advance();
    if (pos < src.size() && peek() == '\'') advance();
    return makeToken(TK::CHAR_LIT, val);
}

Token Lexer::readNumber()
{
    size_t start = pos - 1;
    bool isFloat = false;
    while (pos < src.size() && std::isdigit(peek())) advance();
    if (pos < src.size() && peek() == '.' && std::isdigit(peek(1))) {
        isFloat = true;
        advance(); // '.'
        while (pos < src.size() && std::isdigit(peek())) advance();
    }
    if (pos < src.size() && (peek() == 'f' || peek() == 'F')) {
        isFloat = true; advance();
    }
    std::string lex = src.substr(start, pos - start);
    return makeToken(isFloat ? TK::FLOAT_LIT : TK::INT_LIT, lex);
}

Token Lexer::readIdentOrKeyword()
{
    size_t start = pos - 1;
    while (pos < src.size() && (std::isalnum(peek()) || peek() == '_')) advance();
    std::string lex = src.substr(start, pos - start);
    auto it = kKeywords.find(lex);
    return makeToken(it != kKeywords.end() ? it->second : TK::IDENT, lex);
}

// ─────────────────────────────────────────────────────────────────────────────

std::vector<Token> Lexer::Tokenise(const std::string& source)
{
    src   = source;
    pos   = 0;
    line  = 1;
    error = "";
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();
        if (pos >= src.size()) break;

        char c = advance();

        if (c == '"')  { tokens.push_back(readString()); continue; }
        if (c == '\'') { tokens.push_back(readChar());   continue; }
        if (std::isdigit(c)) { tokens.push_back(readNumber());        continue; }
        if (std::isalpha(c) || c == '_') { tokens.push_back(readIdentOrKeyword()); continue; }

        Token t;
        t.line = line;
        t.lexeme = std::string(1, c);

        switch (c) {
        case '+':
            if      (match('+')) t = makeToken(TK::PLUS_PLUS,  "++");
            else if (match('=')) t = makeToken(TK::PLUS_EQ,    "+=");
            else                 t = makeToken(TK::PLUS,        "+");
            break;
        case '-':
            if      (match('-')) t = makeToken(TK::MINUS_MINUS, "--");
            else if (match('=')) t = makeToken(TK::MINUS_EQ,   "-=");
            else if (match('>')) t = makeToken(TK::ARROW,      "->");
            else                 t = makeToken(TK::MINUS,       "-");
            break;
        case '*': t = makeToken(match('=') ? TK::STAR_EQ   : TK::STAR,  match('=') ? "*=" : "*"); break;
        case '/': t = makeToken(match('=') ? TK::SLASH_EQ  : TK::SLASH, match('=') ? "/=" : "/"); break;
        case '%': t = makeToken(TK::PERCENT, "%"); break;
        case '=': t = makeToken(match('=') ? TK::EQ_EQ   : TK::EQ,   match('=') ? "==" : "="); break;
        case '!': t = makeToken(match('=') ? TK::BANG_EQ : TK::BANG, match('=') ? "!=" : "!"); break;
        case '<': t = makeToken(match('=') ? TK::LT_EQ   : TK::LT,   match('=') ? "<=" : "<"); break;
        case '>': t = makeToken(match('=') ? TK::GT_EQ   : TK::GT,   match('=') ? ">=" : ">"); break;
        case '&': t = makeToken(match('&') ? TK::AMP_AMP  : TK::AMP,  match('&') ? "&&" : "&"); break;
        case '|': t = makeToken(match('|') ? TK::PIPE_PIPE : TK::PIPE, match('|') ? "||" : "|"); break;
        case '^': t = makeToken(TK::CARET, "^");  break;
        case '~': t = makeToken(TK::TILDE, "~");  break;
        case '(': t = makeToken(TK::LPAREN,    "("); break;
        case ')': t = makeToken(TK::RPAREN,    ")"); break;
        case '{': t = makeToken(TK::LBRACE,    "{"); break;
        case '}': t = makeToken(TK::RBRACE,    "}"); break;
        case '[': t = makeToken(TK::LBRACKET,  "["); break;
        case ']': t = makeToken(TK::RBRACKET,  "]"); break;
        case ';': t = makeToken(TK::SEMICOLON, ";"); break;
        case ',': t = makeToken(TK::COMMA,     ","); break;
        case '.': t = makeToken(TK::DOT,       "."); break;
        case ':': t = makeToken(match(':') ? TK::DOUBLE_COLON : TK::COLON, match(':') ? "::" : ":"); break;
        case '?': t = makeToken(TK::QUESTION,  "?"); break;
        case '@': t = makeToken(TK::AT,        "@"); break;
        default:
            error = "Unexpected character '" + std::string(1,c) + "' at line " + std::to_string(line);
            break;
        }
        tokens.push_back(t);
    }

    tokens.push_back(makeToken(TK::END_OF_FILE, ""));
    return tokens;
}

} // namespace Nucleo
