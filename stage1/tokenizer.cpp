#include <tokenizer.h>

namespace slof {

Tokenizer::Tokenizer(const std::string& text_to_tokenize) {
    tokenize(text_to_tokenize);
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

void Tokenizer::tokenize(const std::string& input) {
    (void)(input);
    // TODO: implement tokenizer
}

} // namespace slof