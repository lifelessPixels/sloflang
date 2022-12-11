#include <token.h>

namespace slof {

std::string Token::stringify_literal() const {
    if(!has_literal())
        return "";
    switch(m_literal.index()) {
        case 1: return std::to_string(std::get<bool>(m_literal));
        case 2: return std::to_string(std::get<i64>(m_literal));
        case 3: return std::to_string(std::get<f64>(m_literal));
        case 4: return "\"" + std::get<std::string>(m_literal) + "\"";
        default: return "<invalid literal value>";
    }
}

std::string token_type_to_string(TokenType type) {
    switch(type) {
        #define TOKEN_ENUMERATOR(x) \
            case TokenType::x:      \
                return #x;
        ENUMERATE_SLOF_TOKEN_TYPES
        #undef TOKEN_ENUMERATOR
        default:
            return "<unknown token type>";
    }
}

std::ostream& operator<<(std::ostream& stream, const Token& token) {
    stream << "Token type=" << token_type_to_string(token.type());
    if(token.has_literal()) {
        stream << ", literal=" << token.stringify_literal();
    }
    return stream;
}

} // namespace slof