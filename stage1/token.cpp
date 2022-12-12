#include <algorithm>

#include <token.h>

namespace slof {

static std::string string_to_lower(std::string string) {
    auto to_lower = [](c8 c) { return (c >= 'A' && c <='Z') ? c + ('a' - 'A') : c; };
    std::transform(string.begin(), string.end(), string.begin(), to_lower);
    return string;
}

static std::string keyword_type_literal_to_keyword_literal(std::string keyword_type_literal) {
    return string_to_lower(keyword_type_literal.substr(0, keyword_type_literal.find("Keyword")));
}

std::unordered_map<std::string, TokenType> Token::s_keyword_literal_to_keyword_type = {
    #define TOKEN_ENUMERATOR(x) \
        { keyword_type_literal_to_keyword_literal(#x), TokenType::x },
    ENUMERATE_SLOF_KEYWORD_TOKEN_TYPES
    #undef TOKEN_ENUMERATOR
};

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