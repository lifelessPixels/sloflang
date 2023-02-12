#include <string>
#include <sstream>

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
    std::vector<Token> result_tokens {};

    while(!input_stream.eos()) {
        auto& current = *input_stream.peek();

        if(std::isspace(current)) {
            input_stream.consume_unchecked();
        } else if(current == '_' || std::isalpha(current)) {
            result_tokens.push_back(consume_identifier_or_keyword(input_stream));
        } else if(std::isdigit(current)) {
            result_tokens.push_back(consume_number(input_stream));
        } else if(current == '"') {
            result_tokens.push_back(consume_string(input_stream));
        } else {
            // assume that it must be a symbolic token or gibberish
            result_tokens.push_back(consume_symbolic_token_or_comment(input_stream));
        }
        
        // check for invalid tokens (if such a tokern is pushed into the result
        // it means that error occured and the tokenization should bail out) 
        // and at the same time remove added comments
        if(result_tokens.size() > 0) {
            auto& last_token = result_tokens[result_tokens.size() - 1];
            if(last_token.type() == TokenType::Invalid)
                return last_token.string_literal();

            // if last added token was comment, remove it
            if(last_token.type() == TokenType::Comment)
                result_tokens.pop_back();
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

Token Tokenizer::consume_string(InputStream& input_stream) {
    std::string string_contents {};
    input_stream.consume_unchecked(); // consume opening '"'
    while(!input_stream.eos()) {
        // FIXME: perform proper escaping of backslash escape sequences (\n, \t, \r, \x00 etc.)
        auto current = *input_stream.peek();
        if(current == '"')
            break;

        string_contents += current;
        input_stream.consume_unchecked();
    }

    if(input_stream.eos())
        return Token { TokenType::Invalid, "could not consume string literal, unexpected end of file reached" };
    input_stream.consume_unchecked(); // consume closing '"'
    return Token { TokenType::StringLiteral, std::move(string_contents) };
}

Token Tokenizer::consume_symbolic_token_or_comment(InputStream& input_stream) {
    c8 first_character = input_stream.consume_unchecked();
    switch(first_character) {
        // tokens starting with a dot ('.'): 
        //   * '.' - standard member access operator
        //   * '..' - range expression like 0..100 (end value not included)
        //   * '..=' - range expression like 0..=100 (end value included)
        case '.': {
            if(!input_stream.eos() && *input_stream.peek() == '.') {
                input_stream.consume_unchecked(); // skip second dot
                if(!input_stream.eos() && *input_stream.peek() == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::TwoDotsEquals };
                } else return Token { TokenType::TwoDots };
            } else return Token { TokenType::Dot };
        }

        // tokens starting with a comma (','):
        //   * ',' - standard comma used for splitting arguments in function call,
        //           splitting arguments in function definition, separating
        //           generic arguments etc.
        case ',': return Token { TokenType::Comma };

        // tokens starting with a colon (':'):
        //   * ':' - colon punctuator used for type declarations and loop labels
        case ':': return Token { TokenType::Colon };

        // tokens starting with a colon (';'):
        //   * ':' - semicolon punctuator used for statement termination
        case ';': return Token { TokenType::Semicolon };

        // tokens starting with a left bracket ('('):
        //   * '(' - left bracket
        case '(': return Token { TokenType::LeftBracket };

        // tokens starting with a right bracket (')'):
        //   * ')' - right bracket
        case ')': return Token { TokenType::RightBracket };

        // tokens starting with a left square bracket ('['):
        //   * '[' - left square bracket
        case '[': return Token { TokenType::LeftSquareBracket };

        // tokens starting with a right square bracket (']'):
        //   * ']' - right square bracket
        case ']': return Token { TokenType::RightSquareBracket };

        // tokens starting with a left curly bracket ('{'):
        //   * '{' - left curly bracket
        case '{': return Token { TokenType::LeftCurlyBracket };

        // tokens starting with a right curly bracket ('}'):
        //   * '}' - right curly bracket
        case '}': return Token { TokenType::RightCurlyBracket };

        // tokens starting with a less than symbol ('<'):
        //   * '<' - less than operator or left angle bracket
        //   * '<<' - bitshift left operator
        //   * '<=' - less than or equal operator
        //   * '<<=' - bitshift left and assign back operator
        case '<': {
            if(!input_stream.eos()) {
                c8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::LessThanEquals };
                } else if(next == '<') {
                    input_stream.consume_unchecked(); // skip second less than symbol
                    if(!input_stream.eos() && *input_stream.peek() == '=') {
                        input_stream.consume_unchecked(); // skip equals sign
                        return Token { TokenType::LessThanLessThanEquals };
                    } else return Token { TokenType::LessThanLessThan };
                }
            }
            return Token { TokenType::LessThan };
        }

        
        // tokens starting with a greater than symbol ('>'):
        //   * '>' - greater than operator or right angle bracket
        //   * '>>' - bitshift right operator
        //   * '>=' - greater than or equal operator
        //   * '>>=' - bitshift right and assign back operator
        case '>': {
            if(!input_stream.eos()) {
                c8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::GreaterThanEquals };
                } else if(next == '>') {
                    input_stream.consume_unchecked(); // skip second greater than symbol
                    if(!input_stream.eos() && *input_stream.peek() == '=') {
                        input_stream.consume_unchecked(); // skip equals sign
                        return Token { TokenType::GreaterThanGreaterThanEquals };
                    } else return Token { TokenType::GreaterThanGreaterThan };
                }
            }
            return Token { TokenType::GreaterThan };
        }

        // tokens starting with an equals sign ('='):
        //   * '=' - assignment operator
        //   * '==' - equality comparison operator
        //   * '=>' - fat arrow operator
        case '=': {
            if(!input_stream.eos()) {
                c8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip second equals sign
                    return Token { TokenType::EqualsEquals };
                } else if(next == '>') {
                    input_stream.consume_unchecked(); // skip greater than symbol
                    return Token { TokenType::EqualsGreaterThan };
                }
            }
            return Token { TokenType::Equals };
        }

        // tokens starting with a plus ('+'):
        //   * '+' - addition operator and unary plus operator
        //   * '+=' - add and assign back operator
        case '+': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::PlusEquals };
            } else return Token { TokenType::Plus };
        }

        // tokens starting with a minus ('-'):
        //   * '-' - subtraction operator and unary minus operator
        //   * '-=' - subtract and assign back operator
        //   * '->' - thin arrow operator
        case '-': {
            if(!input_stream.eos()) {
                u8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::MinusEquals };
                } else if(next == '>') {
                    input_stream.consume_unchecked(); // skip greater than symbol
                    return Token { TokenType::MinusGreaterThan };
                }
            } 
            return Token { TokenType::Minus };
        }

        // tokens starting with a star ('*'):
        //   * '*' - multiplication operator
        //   * '**' - exponentiation operator
        //   * '*=' - multiply and assign back operator
        case '*': {
            if(!input_stream.eos()) {
                u8 next = *input_stream.peek();
                if(next == '*') {
                    input_stream.consume_unchecked(); // skip second star
                    return Token { TokenType::StarStar };
                } else if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::StarEquals };
                }
            } 
            return Token { TokenType::Star };
        }

        // tokens starting with a slash ('/')
        //   * '/' - division operator
        //   * '/=' - divide and assign back operator
        //   * '//' - start of the comment (comment lasts until the end of line)
        case '/': {
            if(!input_stream.eos()) {
                c8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::SlashEquals };
                } else if(next == '/') {
                    input_stream.consume_unchecked(); // skip second slash
                    auto comment_text = consume_until([](c8 character) { return character != '\n'; }, input_stream);
                    if(!input_stream.eos())
                        input_stream.consume_unchecked(); // skip new line ('\n') 
                    return Token { TokenType::Comment, std::move(comment_text) };
                }
            } 
            return Token { TokenType::Slash };
        }

        // tokens starting with a percent sign ('%'):
        //   * '%' - modulo operator
        //   * '%=' - modulo and assign back operator
        case '%': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::PercentEquals };
            } else return Token { TokenType::Percent };
        }

        // tokens starting with and symbol ('&'):
        //   * '&' - binary and operator
        //   * '&=' - binary and and assign back operator
        case '&': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::AndEquals };
            } else return Token { TokenType::And };
        }

        // tokens starting with pipe symbol ('|'):
        //   * '|' - binary or operator
        //   * '|=' - binary or and assign back operator
        case '|': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::PipeEquals };
            } else return Token { TokenType::Pipe };
        }

        // tokens starting with caret symbol ('^'):
        //   * '^' - binary xor operator
        //   * '^=' - binary xor and assign back operator
        case '^': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::CaretEquals };
            } else return Token { TokenType::Caret };
        }

        // tokens starting with exclamation point ('!'):
        //   * '!' - force unwrap Result<T> operator
        //   * '!=' - inequality comparison operator
        case '!': {
            if(!input_stream.eos() && *input_stream.peek() == '=') {
                input_stream.consume_unchecked(); // skip equals sign
                return Token { TokenType::ExclamationEquals };
            } else return Token { TokenType::ExclamationPoint };
        }

        // tokens starting with question mark ('?'):
        //   * '?' - tetriary operator and conditional chain operator
        //   * '?=' - conditional (none) assignment operator
        //   * '??' - none-coalescing operator
        case '?': {
            if(!input_stream.eos()) {
                u8 next = *input_stream.peek();
                if(next == '=') {
                    input_stream.consume_unchecked(); // skip equals sign
                    return Token { TokenType::QuestionMarkEquals };
                } else {
                    input_stream.consume_unchecked(); // skip second question mark
                    return Token { TokenType::QuestionMarkQuestionMark };
                }
            } 
            return Token { TokenType::QuestionMark };
        }

        default: {
            std::stringstream error_message_builder {};
            error_message_builder << "unexpected character '" << first_character << "' (code: " << static_cast<int>(first_character) 
                                  << ") found while tokenizing the input";
            return Token { TokenType::Invalid, error_message_builder.str() };
        }
    }
}

} // namespace slof