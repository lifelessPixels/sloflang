#include <string>

#include <ctype.h>

#include <tokenizer.h>

namespace slof {

bool Tokenizer::InputStream::eos() const {
    return m_stream_position == m_string.length();
}

usz Tokenizer::InputStream::remaining_items() const {
    return m_string.length() - m_stream_position;
}

const c8* Tokenizer::InputStream::peek(usz offset) const {
    if(offset >= remaining_items())
        return nullptr;
    return &m_string[m_stream_position + offset];
}

c8 Tokenizer::InputStream::consume_unchecked() {
    return m_string[m_stream_position++];
}

std::optional<c8> Tokenizer::InputStream::consume() {
    if(eos())
        return {};
    return consume_unchecked();
}

std::optional<c8> Tokenizer::InputStream::consume_if(Stream::element_predicate predicate) {
    if(eos())
        return {};
    if(!predicate(*peek()))
        return {};
    return consume_unchecked();
}

bool Tokenizer::TokenStream::eos() const {
    return m_stream_position == m_tokens.size();
}

usz Tokenizer::TokenStream::remaining_items() const {
    return m_tokens.size() - m_stream_position;
}

const Token* Tokenizer::TokenStream::peek(usz offset) const {
    if(offset >= remaining_items())
        return nullptr;
    return &m_tokens[m_stream_position + offset];
}

Token Tokenizer::TokenStream::consume_unchecked() {
    return m_tokens[m_stream_position++];
}

std::optional<Token> Tokenizer::TokenStream::consume() {
    if(eos())
        return {};
    return consume_unchecked();
}

std::optional<Token> Tokenizer::TokenStream::consume_if(element_predicate predicate) {
    if(eos())
        return {};
    auto const& token = *peek();
    if(predicate(token))
        return consume_unchecked();
    else 
        return {};
}

Tokenizer::TokenizationResult Tokenizer::tokenize(std::string text_to_tokenize) {
    InputStream input_stream { text_to_tokenize };
    std::string error_message {};
    std::vector<Token> result_tokens {};

    while(!input_stream.eos()) {
        auto& current = *input_stream.peek();

        if(std::isspace(current)) {
            input_stream.consume_unchecked();
        } else if(current == '_' || std::isalpha(current)) {
            result_tokens.push_back(consume_identifier_or_keyword(input_stream));
        } else if(std::isdigit(current)) {
            result_tokens.push_back(consume_number(input_stream));
        } else {
            // FIXME: this is an error, but in the process of tokenizer
            //        implementation it is useful to just skip unwanted
            //        characters
            input_stream.consume_unchecked();
        }
        
        // check for invalid tokens - if such a tokern is pushed into the result
        // it means that error occured and the tokenization should bail out
        if(result_tokens.size() > 0) {
            auto& last_token = result_tokens[result_tokens.size() - 1];
            if(last_token.type() == TokenType::Invalid)
                return last_token.string_literal();
        }
    }

    return TokenStream(std::move(result_tokens));
}

std::string Tokenizer::consume_until(consume_predicate predicate, InputStream& input_stream) {
    std::string result {};

    auto current = input_stream.peek();
    while(current != nullptr && predicate(*current)) {
        result += *current;
        input_stream.consume_unchecked();
        current = input_stream.peek();
    }

    return result;
}

Token Tokenizer::consume_identifier_or_keyword(InputStream& input_stream) {
    auto identifier_predicate = [](c8 character) { 
        return character == '_' || std::isalnum(character); 
    };
    auto identifier = consume_until(identifier_predicate, input_stream);

    if(Token::s_keyword_literal_to_keyword_type.contains(identifier))
        return Token { Token::s_keyword_literal_to_keyword_type.at(identifier) };
    return Token { TokenType::Identifier, std::move(identifier) };
}

Token Tokenizer::consume_number(InputStream& input_stream) {
    auto number_predicate = [](c8 character) { return std::isdigit(character); };
    // FIXME: if first_number_part is exactly 0 and next character is from set {b, o, x}
    //        it means non-decimal based literal and it should also be parsed 
    auto first_number_part = consume_until(number_predicate, input_stream);

    if(input_stream.remaining_items() >= 2) {
        auto maybe_dot = *input_stream.peek();
        auto maybe_digit = *input_stream.peek(1);

        if(maybe_dot == '.' && std::isdigit(maybe_digit)) {
            input_stream.consume_unchecked(); // consume the dot
            auto second_number_part = consume_until(number_predicate, input_stream);
            auto combined_number = first_number_part + "." + second_number_part;

            try {
                f64 float_number_value = std::stod(combined_number);
                return Token { TokenType::FloatLiteral, float_number_value };
            } catch(const std::exception&) {
                std::string error_message = "float literal '" + combined_number + "' could not be converted to float value";
                return Token { TokenType::Invalid, std::move(error_message) };
            }
        }
    }

    try {
        u64 integer_number_value = std::stoull(first_number_part);
        return Token { TokenType::IntegerLiteral, integer_number_value };
    } catch(const std::exception&) {
        std::string error_message = "integral literal '" + first_number_part + "' could not be converted to integral value";
        return Token { TokenType::Invalid, std::move(error_message) };
    }
}

} // namespace slof