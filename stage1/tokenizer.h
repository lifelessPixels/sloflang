#pragma once
#include <vector>

#include <stream.h>
#include <token.h>

namespace slof {

class Tokenizer : public Stream<Token> {
public:
    explicit Tokenizer(const std::string& text_to_tokenize);

    bool tokenization_failed() const { return m_tokenization_failed; }

    // ^Stream<Token>
    virtual bool eos() const override;
    virtual usz remaining_items() const override;

    virtual const Token* peek(usz offset = 0) const override;
    virtual Token consume_unchecked() override;
    virtual std::optional<Token> consume() override;
    virtual std::optional<Token> consume_if(element_predicate predicate) override;

private:
    using consume_predicate = std::function<bool(c8)>;

    class TextInput : public Stream<c8> {
    public:
        explicit TextInput(const std::string& string);

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

    void tokenize();

    std::string consume_until(consume_predicate predicate);

    Token consume_identifier_or_keyword();
    Token consume_number();

    TextInput m_input;
    bool m_tokenization_failed { false };
    std::vector<Token> m_tokens {};
    usz m_stream_position { 0 };

};

} // namespace slof