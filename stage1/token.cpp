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

std::ostream& operator<<(std::ostream& stream, const Token& token) {
    // NOTE: if we compiled the project with C++23 support, there would be std::to_underlying()
    //       however it is very fresh standard so simple static_cast<u32>() will do the thing
    // FIXME: stringify TokenType instead of using integer representing enum state
    stream << "Token type=" << static_cast<u32>(token.type());
    if(token.has_literal()) {
        stream << ", literal=" << token.stringify_literal();
    }
    return stream;
}

} // namespace slof