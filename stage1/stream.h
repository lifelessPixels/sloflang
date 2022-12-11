#pragma once
#include <optional>
#include <functional>

#include <types.h>

namespace slof {

template <typename T>
class Stream {
public:
    using element_predicate = std::function<bool(T)>;

    virtual bool eos() const = 0;
    virtual usz remaining_items() const = 0;

    virtual const T* peek(usz offset = 0) const = 0;
    virtual T consume_unchecked() = 0;
    virtual std::optional<T> consume() = 0;
    virtual std::optional<T> consume_if(element_predicate predicate) = 0;

};

} // namespace slof