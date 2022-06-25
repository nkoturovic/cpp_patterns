#include <concepts>
#include <fmt/format.h>
#include <string>

template<typename C>
concept Constraint = requires(C c) {
    //c.is_satisfied(declval(auto));
    true;
};

template <std::unsigned_integral T>
struct Length {
    using value_type = T;
    T from = std::numeric_limits<T>::min(); 
    T to = std::numeric_limits<T>::max();

    template <class S>
    constexpr bool is_satisfied(S const& s) const noexcept 
        requires requires { {s.length() } -> std::convertible_to<T>; } 
    {
        return s.length() >= from && s.length() <= to;
    }

    constexpr bool is_satisfied(const char * s) const noexcept {
        auto len = strlen(s);
        return len >= from && len <= to;
    }
};


// template <std::unsigned_integral T>
// Length(T t1, T t2) -> Length<T>;

enum class Lang { EN_US, RS_LATIN, RS_CYRILIC };

template <Lang lang>
struct DescribeConstraint {
    static constexpr std::string describe(Constraint auto const&);
};

template <auto lang = Lang::EN_US>
std::string describe(Constraint auto const& c) {
    return DescribeConstraint<lang>::describe(c);
}
int main() {
    constexpr auto ctr = Length{.from=1u, .to=5u};
    fmt::print("{}\n", ctr.is_satisfied(std::string("Hello")));
    fmt::print("{}\n", ctr.is_satisfied(std::string_view("Hello world")));
    fmt::print("{}\n", ctr.is_satisfied("Hello world"));
    fmt::print("{}\n", describe(ctr));
    fmt::print("{}\n", describe<Lang::RS_LATIN>(ctr));
    fmt::print("{}\n", describe<Lang::RS_CYRILIC>(ctr));
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
