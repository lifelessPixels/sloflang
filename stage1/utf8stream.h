#pragma once
#include <string>
#include <functional>

#include <stream.h>
#include <types.h>

namespace slof {

using utf8_codepoint = u32;

class Utf8Stream : public Stream<utf8_codepoint> {
public:
    explicit Utf8Stream(const std::string& string);

    // ^Stream<Token>
    virtual bool eos() const override;
    virtual usz remaining_items() const override;

    virtual const utf8_codepoint* peek(usz offset = 0) const override;
    virtual utf8_codepoint consume_unchecked() override;
    virtual std::optional<utf8_codepoint> consume() override;
    virtual std::optional<utf8_codepoint> consume_if(element_predicate predicate) override;

private:
    using byte_getter = std::function<std::optional<u8>()>;

    std::optional<utf8_codepoint> get_next_codepoint(byte_getter get_next_byte);
    std::optional<utf8_codepoint> decode_from_bytes(u8 bytes[], u8 count);

    std::vector<utf8_codepoint> m_codepoints {};
    usz m_stream_position { 0 };

};

} // namespace slof