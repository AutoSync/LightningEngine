// NucleoToken.h — Token types and Token struct for the Núcleo script lexer.
#pragma once
#include <string>

namespace Nucleo {

enum class TK {
    // ── Literals ──────────────────────────────────────────────────────────────
    INT_LIT, FLOAT_LIT, STRING_LIT, CHAR_LIT,
    TRUE, FALSE, NULL_LIT,

    // ── Type keywords ─────────────────────────────────────────────────────────
    KW_INT, KW_FLOAT, KW_BOOL, KW_STRING, KW_VOID, KW_VAR,

    // ── Control flow ─────────────────────────────────────────────────────────
    KW_IF, KW_ELSE, KW_WHILE, KW_FOR, KW_FOREACH, KW_IN,
    KW_RETURN, KW_BREAK, KW_CONTINUE,

    // ── OOP ──────────────────────────────────────────────────────────────────
    KW_CLASS, KW_NEW, KW_THIS, KW_BASE,
    KW_PUBLIC, KW_PRIVATE, KW_PROTECTED, KW_STATIC, KW_OVERRIDE,

    // ── Identifier ───────────────────────────────────────────────────────────
    IDENT,

    // ── Arithmetic ───────────────────────────────────────────────────────────
    PLUS, MINUS, STAR, SLASH, PERCENT,
    PLUS_PLUS, MINUS_MINUS,

    // ── Assignment ───────────────────────────────────────────────────────────
    EQ,
    PLUS_EQ, MINUS_EQ, STAR_EQ, SLASH_EQ,

    // ── Comparison ───────────────────────────────────────────────────────────
    EQ_EQ, BANG_EQ,
    LT, LT_EQ, GT, GT_EQ,

    // ── Logical ──────────────────────────────────────────────────────────────
    AMP_AMP, PIPE_PIPE, BANG,

    // ── Bitwise ──────────────────────────────────────────────────────────────
    AMP, PIPE, CARET, TILDE,

    // ── Punctuation ──────────────────────────────────────────────────────────
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    LBRACKET, RBRACKET,
    SEMICOLON, COMMA, DOT, COLON, DOUBLE_COLON, QUESTION, ARROW,

    // ── Special ──────────────────────────────────────────────────────────────
    AT,           // @ (attribute)
    END_OF_FILE
};

struct Token {
    TK          type   = TK::END_OF_FILE;
    std::string lexeme;
    int         line   = 0;

    bool is(TK t)             const { return type == t; }
    bool isLiteral()          const {
        return type == TK::INT_LIT || type == TK::FLOAT_LIT ||
               type == TK::STRING_LIT || type == TK::CHAR_LIT ||
               type == TK::TRUE || type == TK::FALSE || type == TK::NULL_LIT;
    }
};

} // namespace Nucleo
