#include <string>

#include <ctype.h>

#include <tokenizer.h>

namespace slof {

Tokenizer::TextInput::TextInput(const std::string& string) : m_string(string) {}

bool Tokenizer::TextInput::eos() const {
    return m_stream_position == m_string.length();
}

usz Tokenizer::TextInput::remaining_items() const {
    return m_string.length() - m_stream_position;
}

const c8* Tokenizer::TextInput::peek(usz offset) const {
    if(offset >= remaining_items())
        return nullptr;
    return &m_string[m_stream_position + offset];
}

c8 Tokenizer::TextInput::consume_unchecked() {
    return m_string[m_stream_position++];
}

std::optional<c8> Tokenizer::TextInput::consume() {
    if(eos())
        return {};
    return consume_unchecked();
}

std::optional<c8> Tokenizer::TextInput::consume_if(Stream::element_predicate predicate) {
    if(eos())
        return {};
    if(!predicate(*peek()))
        return {};
    return consume_unchecked();
}

Tokenizer::Tokenizer(const std::string& text_to_tokenize) 
    : m_input(text_to_tokenize) {
    tokenize();
}

bool Tokenizer::eos() const {
    return m_stream_position == m_tokens.size();
}

usz Tokenizer::remaining_items() const {
    return m_tokens.size() - m_stream_position;
}

const Token* Tokenizer::peek(usz offset) const {
    if(offset >= remaining_items())
        return nullptr;
    return &m_tokens[m_stream_position + offset];
}

Token Tokenizer::consume_unchecked() {
    return m_tokens[m_stream_position++];
}

std::optional<Token> Tokenizer::consume() {
    if(eos())
        return {};
    return consume_unchecked();
}

std::optional<Token> Tokenizer::consume_if(element_predicate predicate) {
    if(eos())
        return {};
    auto const& token = *peek();
    if(predicate(token))
        return consume_unchecked();
    else 
        return {};
}

void Tokenizer::tokenize() {
    while(!m_input.eos()) {
        if(m_tokenization_failed)
            break;

        auto& current = *m_input.peek();
        if(std::isspace(current)) {
            m_input.consume_unchecked();
        } else if(current == '_' || std::isalpha(current)) {
            m_tokens.push_back(consume_identifier_or_keyword());
        } else if(std::isdigit(current)) {
            m_tokens.push_back(consume_number());
        } else {
            // FIXME: this is an error, but in the process of tokenizer
            //        implementation it is useful to just skip unwanted
            //        characters
            m_input.consume_unchecked();
        }
    }
}

std::string Tokenizer::consume_until(consume_predicate predicate) {
    std::string result {};

    auto current = m_input.peek();
    while(current != nullptr && predicate(*current)) {
        result += *current;
        m_input.consume_unchecked();
        current = m_input.peek();
    }

    return result;
}

Token Tokenizer::consume_identifier_or_keyword() {
    auto identifier_predicate = [](c8 character) { 
        return character == '_' || std::isalnum(character); 
    };
    auto identifier = consume_until(identifier_predicate);

    if(Token::s_keyword_literal_to_keyword_type.contains(identifier))
        return Token { Token::s_keyword_literal_to_keyword_type.at(identifier) };
    return Token { TokenType::Identifier, std::move(identifier) };
}

Token Tokenizer::consume_number() {
    auto number_predicate = [](c8 character) { return std::isdigit(character); };
    // FIXME: if first_number_part is exactly 0 and next character is from set {b, o, x}
    //        it means non-decimal based literal and it should also be parsed 
    auto first_number_part = consume_until(number_predicate);

    if(m_input.remaining_items() >= 2) {
        auto maybe_dot = *m_input.peek();
        auto maybe_digit = *m_input.peek(1);

        if(maybe_dot == '.' && std::isdigit(maybe_digit)) {
            m_input.consume_unchecked(); // consume the dot
            auto second_number_part = consume_until(number_predicate);
            auto combined_number = first_number_part + "." + second_number_part;

            try {
                f64 float_number_value = std::stod(combined_number);
                return Token { TokenType::FloatLiteral, float_number_value };
            } catch(const std::exception&) {
                // FIXME: add proper error reporting
                m_tokenization_failed = true;
                return Token { TokenType::Invalid, std::move(combined_number) };
            }
        }
    }

    try {
        u64 integer_number_value = std::stoull(first_number_part);
        return Token { TokenType::IntegerLiteral, integer_number_value };
    } catch(const std::exception&) {
        // FIXME: add proper error reporting
        m_tokenization_failed = true;
        return Token { TokenType::Invalid, std::move(first_number_part) };
    }
}

} // namespace slof