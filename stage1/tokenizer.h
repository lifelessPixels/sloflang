#pragma once
#include <vector>

#include <stream.h>
#include <token.h>

namespace slof {

class Tokenizer : Stream<Token> {
public:
    explicit Tokenizer(const std::string& text_to_tokenize);

    // ^Stream<Token>
    virtual bool eos() const override;
    virtual usz remaining_items() const override;

    virtual const Token* peek(usz offset = 0) const override;
    virtual Token consume_unchecked() override;
    virtual std::optional<Token> consume() override;
    virtual std::optional<Token> consume_if(element_predicate predicate) override;

private:
    void tokenize(const std::string& input);

    std::vector<Token> m_tokens {};
    usz m_stream_position { 0 };

};

} // namespace slof