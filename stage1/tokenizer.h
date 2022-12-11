#pragma once
#include <vector>

#include <stream.h>
#include <token.h>
#include <utf8stream.h>

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
    void tokenize(Utf8Stream& input);

    bool m_tokenization_failed { false };
    std::vector<Token> m_tokens {};
    usz m_stream_position { 0 };

};

} // namespace slof