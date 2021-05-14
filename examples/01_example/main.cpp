#include <concepts>
#include <fmt/format.h>

/* Compile type concept or what is Validator */
template<typename C>
concept Constraint = requires(C c) {
    //c.validate(declval(auto));
    true;
};

template <std::unsigned_integral T>
struct Length {
    using value_type = T;
    T from = std::numeric_limits<T>::min(); 
    T to = std::numeric_limits<T>::max();

    template <class S>
    constexpr bool validate(S const& s) const noexcept 
        requires requires { {s.length() } -> std::convertible_to<T>; } 
    {
        return s.length() >= from && s.length() <= to;
    }

    constexpr bool validate(const char * s) const noexcept {
        auto len = strlen(s);
        return len >= from && len <= to;
    }
};

template <Constraint C>
struct ConstraintImpl {
    static constexpr auto describe(C const& c);
};

template <Constraint C>
std::string describe(C const& c) {
    return ConstraintImpl<C>::describe(c);
}

template <Constraint auto c>
struct S { 
    static constexpr Constraint auto constraint = c;
};

int main() {
    constexpr auto l = Length{.from = 1u, .to = 5u};
    S<l> s{};
    fmt::print("{}\n", s.constraint.validate(std::string("Hello")));
    fmt::print("{}\n", s.constraint.validate(std::string_view("Hello world")));
    fmt::print("{}\n", s.constraint.validate("Hello world"));
    fmt::print("Hello world\n");
    fmt::print("{}\n", describe(l));

    return 0;
}


template <std::unsigned_integral T>
struct ConstraintImpl<Length<T>> {
    static constexpr auto describe(Length<T> const& l) {
        return fmt::format("Length should be from {} to {}", l.from, l.to);
    }
};
