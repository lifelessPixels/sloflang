#pragma once
#include <string>
#include <iostream>
#include <variant>
#include <unordered_map>

#include <types.h>

#define ENUMERATE_SLOF_KEYWORD_TOKEN_TYPES \
    TOKEN_ENUMERATOR(IfKeyword) \
    TOKEN_ENUMERATOR(ElseKeyword) \
    TOKEN_ENUMERATOR(LetKeyword) \
    TOKEN_ENUMERATOR(FuncKeyword) \
    TOKEN_ENUMERATOR(NotKeyword) \
    TOKEN_ENUMERATOR(AndKeyword) \
    TOKEN_ENUMERATOR(OrKeyword)

#define ENUMERATE_SLOF_SYMBOLIC_TOKEN_TYPES \
    TOKEN_ENUMERATOR(Dot) \
    TOKEN_ENUMERATOR(TwoDots) \
    TOKEN_ENUMERATOR(TwoDotsEquals) \
    TOKEN_ENUMERATOR(Comma) \
    TOKEN_ENUMERATOR(Colon) \
    TOKEN_ENUMERATOR(Semicolon) \
    TOKEN_ENUMERATOR(LeftBracket) \
    TOKEN_ENUMERATOR(RightBracket) \
    TOKEN_ENUMERATOR(LeftSquareBracket) \
    TOKEN_ENUMERATOR(RightSquareBracket) \
    TOKEN_ENUMERATOR(LeftCurlyBracket) \
    TOKEN_ENUMERATOR(RightCurlyBracket) \
    TOKEN_ENUMERATOR(LessThan) \
    TOKEN_ENUMERATOR(LessThanLessThan) \
    TOKEN_ENUMERATOR(LessThanEquals) \
    TOKEN_ENUMERATOR(LessThanLessThanEquals) \
    TOKEN_ENUMERATOR(GreaterThan) \
    TOKEN_ENUMERATOR(GreaterThanGreaterThan) \
    TOKEN_ENUMERATOR(GreaterThanEquals) \
    TOKEN_ENUMERATOR(GreaterThanGreaterThanEquals) \
    TOKEN_ENUMERATOR(Equals) \
    TOKEN_ENUMERATOR(EqualsEquals) \
    TOKEN_ENUMERATOR(EqualsGreaterThan) \
    TOKEN_ENUMERATOR(Plus) \
    TOKEN_ENUMERATOR(PlusEquals) \
    TOKEN_ENUMERATOR(Minus) \
    TOKEN_ENUMERATOR(MinusEquals) \
    TOKEN_ENUMERATOR(MinusGreaterThan) \
    TOKEN_ENUMERATOR(Star) \
    TOKEN_ENUMERATOR(StarStar) \
    TOKEN_ENUMERATOR(StarEquals) \
    TOKEN_ENUMERATOR(Slash) \
    TOKEN_ENUMERATOR(SlashEquals) \
    TOKEN_ENUMERATOR(Percent) \
    TOKEN_ENUMERATOR(PercentEquals) \
    TOKEN_ENUMERATOR(And) \
    TOKEN_ENUMERATOR(AndEquals) \
    TOKEN_ENUMERATOR(Pipe) \
    TOKEN_ENUMERATOR(PipeEquals) \
    TOKEN_ENUMERATOR(Caret) \
    TOKEN_ENUMERATOR(CaretEquals) \
    TOKEN_ENUMERATOR(ExclamationPoint) \
    TOKEN_ENUMERATOR(ExclamationEquals) \
    TOKEN_ENUMERATOR(QuestionMark) \
    TOKEN_ENUMERATOR(QuestionMarkEquals) \
    TOKEN_ENUMERATOR(QuestionMarkQuestionMark)

#define ENUMERATE_SLOF_TOKEN_TYPES \
    ENUMERATE_SLOF_KEYWORD_TOKEN_TYPES \
    ENUMERATE_SLOF_SYMBOLIC_TOKEN_TYPES \
    TOKEN_ENUMERATOR(Comment) \
    TOKEN_ENUMERATOR(Identifier) \
    TOKEN_ENUMERATOR(IntegerLiteral) \
    TOKEN_ENUMERATOR(FloatLiteral) \
    TOKEN_ENUMERATOR(StringLiteral) \
    TOKEN_ENUMERATOR(Invalid)

namespace slof  {

#define TOKEN_ENUMERATOR(x) x,
enum class TokenType : u32 {
    ENUMERATE_SLOF_TOKEN_TYPES
};
#undef TOKEN_ENUMERATOR

class Token {
public:
    using literal_variant = std::variant<std::monostate, bool, u64, f64, std::string>;

    static std::unordered_map<std::string, TokenType> s_keyword_literal_to_keyword_type;

    explicit Token(TokenType type) : m_type(type) {}
    Token(TokenType type, literal_variant literal) : m_type(type), m_literal(literal) {}
    
    TokenType type() const { return m_type; }
    bool has_literal() const { return m_literal.index() != 0; }
    std::string stringify_literal() const;

    bool bool_literal() const { return std::get<bool>(m_literal); }
    u64 integer_literal() const { return std::get<u64>(m_literal); }
    f64 float_literal() const { return std::get<f64>(m_literal); }
    const std::string& string_literal() const { return std::get<std::string>(m_literal); }

private:
    TokenType m_type { TokenType::Invalid };
    literal_variant m_literal {};

};

std::string token_type_to_string(TokenType type);
std::ostream& operator<<(std::ostream&, const Token&);

} // namespace slof
