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
    TOKEN_ENUMERATOR(FuncKeyword)

#define ENUMERATE_SLOF_TOKEN_TYPES \
    ENUMERATE_SLOF_KEYWORD_TOKEN_TYPES \
    TOKEN_ENUMERATOR(Identifier) \
    TOKEN_ENUMERATOR(IntegerLiteral) \
    TOKEN_ENUMERATOR(FloatLiteral) \
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
