#include <concepts>
#include <fmt/format.h>
#include <string>

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

enum class Lang { EN_US, RS_LATIN, RS_CYRILIC };

template <Lang lang>
struct DescribeConstraint {
    static constexpr std::string describe(Constraint auto const&);
};

template <auto lang = Lang::EN_US>
std::string describe(Constraint auto const& c) {
    return DescribeConstraint<lang>::describe(c);
}
 
template <Constraint auto c>
struct S { 
    static constexpr Constraint auto constraint = c;
};

int main() {
    S<Length{.from=1u, .to=5u}> s{};
    fmt::print("{}\n", s.constraint.validate(std::string("Hello")));
    fmt::print("{}\n", s.constraint.validate(std::string_view("Hello world")));
    fmt::print("{}\n", s.constraint.validate("Hello world"));
    fmt::print("{}\n", describe(s.constraint));
    fmt::print("{}\n", describe<Lang::RS_LATIN>(s.constraint));
    fmt::print("{}\n", describe<Lang::RS_CYRILIC>(s.constraint));
    return 0;
}

template <>
struct DescribeConstraint<Lang::EN_US> {
    template <std::unsigned_integral T>
    static std::string describe(Length<T> const& l) {
        return fmt::format("Length should be from {} to {}", l.from, l.to);
    }
};

template <>
struct DescribeConstraint<Lang::RS_LATIN> {
    template <std::unsigned_integral T>
    static std::string describe(Length<T> const& l) {
        return fmt::format("Dužina mora biti između {} i {}", l.from, l.to);
    }
};

template <>
struct DescribeConstraint<Lang::RS_CYRILIC> {
    template <std::unsigned_integral T>
    static std::string describe(Length<T> const& l) {
        return fmt::format("Дужина мора бити између {} и {}", l.from, l.to);
    }
};
