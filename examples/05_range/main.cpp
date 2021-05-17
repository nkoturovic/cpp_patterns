#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>
#include <concepts>
#include <range/v3/view.hpp>

#include <boost/hana.hpp>
#include <boost/hana/adapt_struct.hpp>

#define REFL BOOST_HANA_ADAPT_STRUCT

namespace hana = boost::hana;
namespace rng = ranges;
namespace vw = ranges::views;

template <typename S>
consteval auto get_struct_keys() noexcept {
  return hana::transform(hana::accessors<S>(), hana::first);
}


template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    using result_type = ReturnType;

    template <size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

// Idea: Make single tuple_view like type and 
// then just make all kind of members to
// return that type and get same interface

template <typename ...Ts>
struct tuple_view {
    hana::tuple<Ts...> &values;
};

//////////// ANOTHER IDEA /////////////

template <typename D>
struct Refl {

    template <class F>
    constexpr auto transform(F &&f) noexcept {
        constexpr auto array_if_not_void = []<typename T>(auto&& ...v) {
             if constexpr (!std::is_same_v<T, void>)
                 return std::array {std::forward<decltype(v)>(v)... };
             else 
                 return;
        };

        // First arg is member names only
        using traits = function_traits<F>;

        if constexpr (std::is_invocable_v<F, const char *>) {
            //using R = std::invoke_result_t<F, const char *>;
            //return array_if_not_void.template operator()<void>(hana::transform(get_struct_keys<D>(), std::forward<F>(f)));
        } else if constexpr (std::is_invocable_v<F, const char *, unsigned>) {
            //using R = std::invoke_result_t<F, const char *>;
                hana::for_each(get_struct_keys<D>(), [&, i=0u](auto &&x) mutable {
                        std::invoke(f, x.c_str(), i++);
                    }
                );
        } else {
            // auto fields = hana::to_map(*static_cast<D*>(this));
            // //using R = std::invoke_result_t<F, const char *, std::size_t, int>;
            // return array_if_not_void.template operator()<void>(
            //     hana::transform(std::move(fields), [&](auto &&p) {
            //         return std::invoke(std::forward<F>(f), std::move(hana::first(p)), std::move(hana::second(p)));
            //     })
            // );
            // ERROR
        }
    }
};

struct User final : Refl<User> {
    uint64_t id;
    std::string firstname;
    std::string lastname;
};
REFL(User, id, firstname, lastname);

int main() 
{
    User u {
        .id = 0u,
        .firstname = "Nebojsa",
        .lastname = "Koturovic",
    };

    u.transform([](const char * name, unsigned i) {
        fmt::print("{}, {}", name, i);
    });

    return 0;
}
