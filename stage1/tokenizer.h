#pragma once
#include <vector>
#include <variant>

#include <stream.h>
#include <token.h>

namespace slof {

class Tokenizer {
public:
    class TokenStream : public Stream<Token> {
    public:
        explicit TokenStream(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

        // ^Stream<Token>
        virtual bool eos() const override;
        virtual usz remaining_items() const override;

        virtual const Token* peek(usz offset = 0) const override;
        virtual Token consume_unchecked() override;
        virtual std::optional<Token> consume() override;
        virtual std::optional<Token> consume_if(element_predicate predicate) override;

    private:
        std::vector<Token> m_tokens {};
        usz m_stream_position { 0 };
    };

    // TODO: create a richer type for error reporting - string is kinda ok for now
    class TokenizationResult {
    public:
        TokenizationResult(TokenStream stream) : m_result(std::move(stream)) {}
        TokenizationResult(std::string error_message) : m_result(std::move(error_message)) {}

        bool is_token_stream() const { return m_result.index() == 1; }
        bool is_error() const { return m_result.index() == 2; }

        const std::string& error_message() const { return std::get<std::string>(m_result); }
        TokenStream& token_stream() { return std::get<TokenStream>(m_result); } 

    private:
        std::variant<std::monostate, TokenStream, std::string> m_result;

    };

    static TokenizationResult tokenize(std::string text_to_tokenize);

private:
    using consume_predicate = std::function<bool(c8)>;

    class InputStream : public Stream<c8> {
    public:
        explicit InputStream(const std::string& string) : m_string(string) {};

        // ^Stream<c8>
        virtual bool eos() const override;
        virtual usz remaining_items() const override;

        virtual const c8* peek(usz offset = 0) const override;
        virtual c8 consume_unchecked() override;
        virtual std::optional<c8> consume() override;
        virtual std::optional<c8> consume_if(element_predicate predicate) override;

    private:
        const std::string& m_string;
        usz m_stream_position { 0 };

    };

    static std::string consume_until(consume_predicate predicate, InputStream& input_stream);

    static Token consume_identifier_or_keyword(InputStream& input_stream);
    static Token consume_number(InputStream& input_stream);

};

} // namespace slof