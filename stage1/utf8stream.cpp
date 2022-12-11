#include <iostream>

#include <utf8stream.h>

namespace slof {

Utf8Stream::Utf8Stream(const std::string& string) {
    usz current_position = 0;
    auto get_next_byte = [&]() -> std::optional<u8> {
        if(current_position == string.length())
            return {};
        return static_cast<u8>(string[current_position++]);
    };

    while(true) {
        auto maybe_next_codepoint = get_next_codepoint(get_next_byte);
        if(!maybe_next_codepoint.has_value())
            break;
        m_codepoints.push_back(*maybe_next_codepoint);
    }
}

bool Utf8Stream::eos() const {
    return m_codepoints.size() == m_stream_position;
}

usz Utf8Stream::remaining_items() const {
    return m_codepoints.size() - m_stream_position;
}

const utf8_codepoint* Utf8Stream::peek(usz offset) const {
    if(offset >= remaining_items())
        return nullptr;
    return &m_codepoints[m_stream_position + offset];
}

utf8_codepoint Utf8Stream::consume_unchecked() {
    return m_codepoints[m_stream_position++];
}

std::optional<utf8_codepoint> Utf8Stream::consume() {
    if(eos())
        return {};
    return consume_unchecked();
}

std::optional<utf8_codepoint> Utf8Stream::consume_if(element_predicate predicate) {
    if(eos())
        return {};
    auto const& codepoint = *peek();
    if(predicate(codepoint))
        return consume_unchecked();
    return {};
}

std::optional<utf8_codepoint> Utf8Stream::get_next_codepoint(byte_getter get_next_byte) {
    // TODO: make this safer and more spec-like
    auto maybe_first_byte = get_next_byte();
    if(!maybe_first_byte.has_value())
        return {};
    u8 bytes[6] = {0};
    bytes[0] = *maybe_first_byte;

    u8 number_of_additional_bytes = 0;
    for(i32 i = 7; i >= 0; i--) {
        if(bytes[0] & (1 << i))
            number_of_additional_bytes++;
        else break;
    }

    if(number_of_additional_bytes > 0)
        number_of_additional_bytes -= 1;
    if(number_of_additional_bytes > 5)
        return {}; // NOTE: this is an error, UTF-8 has maximum of 6 bytes
    
    for(u8 i = 0; i < number_of_additional_bytes; i++) {
        auto maybe_next_byte = get_next_byte();
        if(!maybe_next_byte.has_value())
            return {};
        auto next_byte = *maybe_next_byte;
        if((next_byte & (1 << 7)) == 0 || (next_byte & (1 << 6)) != 0)
            return {};
        bytes[i + 1] = next_byte;
    }

    return decode_from_bytes(bytes, number_of_additional_bytes + 1);
}

std::optional<utf8_codepoint> Utf8Stream::decode_from_bytes(u8 bytes[], u8 count) {
    if(count == 0)
        return {};
    if(count == 1)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b01111111));
    if(count == 2)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b00011111) << 6)
             | (static_cast<utf8_codepoint>(bytes[1] & 0b00111111) << 0);
    if(count == 3)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b00001111) << 12)
             | (static_cast<utf8_codepoint>(bytes[1] & 0b00111111) << 6)    
             | (static_cast<utf8_codepoint>(bytes[2] & 0b00111111) << 0);
    if(count == 4)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b00000111) << 18)
             | (static_cast<utf8_codepoint>(bytes[1] & 0b00111111) << 12)    
             | (static_cast<utf8_codepoint>(bytes[2] & 0b00111111) << 6)    
             | (static_cast<utf8_codepoint>(bytes[3] & 0b00111111) << 0);
    if(count == 5)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b00000011) << 24)
             | (static_cast<utf8_codepoint>(bytes[1] & 0b00111111) << 18)    
             | (static_cast<utf8_codepoint>(bytes[2] & 0b00111111) << 12)    
             | (static_cast<utf8_codepoint>(bytes[3] & 0b00111111) << 6)    
             | (static_cast<utf8_codepoint>(bytes[4] & 0b00111111) << 0);    
    if(count == 6)
        return (static_cast<utf8_codepoint>(bytes[0] & 0b00000001) << 30)
             | (static_cast<utf8_codepoint>(bytes[1] & 0b00111111) << 24)    
             | (static_cast<utf8_codepoint>(bytes[2] & 0b00111111) << 18)    
             | (static_cast<utf8_codepoint>(bytes[3] & 0b00111111) << 12)    
             | (static_cast<utf8_codepoint>(bytes[4] & 0b00111111) << 6)    
             | (static_cast<utf8_codepoint>(bytes[5] & 0b00111111) << 0);   
    return 0; // NOTE: should never happen
}

} // namespace slof