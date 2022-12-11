#pragma once
#include <string>
#include <variant>
#include <iostream>

#include <types.h>

namespace slof  {

enum class TokenType : u32 {
    Invalid
};

class Token {
public:
    using literal_variant = std::variant<std::monostate, bool, i64, f64, std::string>;

    explicit Token(TokenType type) : m_type(type) {}
    Token(TokenType type, literal_variant literal) : m_type(type), m_literal(literal) {}
    
    TokenType type() const { return m_type; }
    bool has_literal() const { return m_literal.index() == 0; }
    std::string stringify_literal() const;

    bool bool_literal() const { return std::get<bool>(m_literal); }
    i64 integer_literal() const { return std::get<i64>(m_literal); }
    f64 float_literal() const { return std::get<f64>(m_literal); }
    const std::string& string_literal() const { return std::get<std::string>(m_literal); }

private:
    TokenType m_type { TokenType::Invalid };
    literal_variant m_literal {};

};

std::ostream& operator<<(std::ostream&, const Token&);

} // namespace slof
