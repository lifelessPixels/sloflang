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
        auto& current = *m_input.peek();

        if(std::isspace(current)) {
            m_input.consume_unchecked();
        } else if(current == '_' || std::isalpha(current)) {
            m_tokens.push_back(consume_identifier());
        } else {
            // FIXME: this is an error, but in the process of tokenizer
            //        implementation it is useful to just skip unwanted
            //        characters
            m_input.consume_unchecked();
        }
    }
}

Token Tokenizer::consume_identifier() {
    std::string identifier {};
    auto current = m_input.peek();

    while(current != nullptr && (*current == '_' || std::isalnum(*current))) {
        identifier += *current;
        m_input.consume_unchecked();
        current = m_input.peek();
    }

    return Token { TokenType::Identifier, std::move(identifier) };
}

} // namespace slof