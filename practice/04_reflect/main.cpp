#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>
#include <concepts>
#include <range/v3/view.hpp>

#include <boost/hana.hpp>
#include <boost/hana/adapt_struct.hpp>

// Reflection implementation using hana library
//  - Layer on top of hana should be implemented for ease of use
//  - This layer abstracts using reflection as well as ease of
//    implementation, which can be done by CRTP deriving from Refl class/
// Basically should implement all possible access functions
// for extracting values, also operator[] and much more
// to be nice set of tools for using in real world

#define REFL BOOST_HANA_ADAPT_STRUCT

namespace hana = boost::hana;
namespace rng = ranges;
namespace vw = ranges::views;

template <typename S>
consteval auto get_struct_keys() noexcept {
  return hana::transform(hana::accessors<S>(), hana::first);
}

template <typename D>
struct Refl {
    static consteval auto keys() noexcept {
        return hana::unpack(get_struct_keys<D>(), 
            [](auto ...ks) { 
                return std::array { ks.c_str()... };
            });
    }
    static std::size_t consteval num_of_fields() { 
        return keys().size();
    }
    // template <class Func>
    // constexpr auto values(Func &&f) const noexcept {
    //     return hana::unpack(hana::members(*static_cast<const D*>(this)),
    //         [&]<typename ...Vs>(Vs &&...vs)  {
    //             constexpr auto fst_t = hana::first(hana::tuple_t<Vs...>);
    //             if constexpr (std::is_same_v<void, std::invoke_result_t<Func, typename decltype(fst_t)::type>>) {
    //                 (std::invoke(std::forward<Func>(f), std::forward<Vs>(vs)), ...);
    //             } else {
    //                 return std::array { std::invoke(std::forward<Func>(f), std::forward<Vs>(vs)) ... };
    //             }
    //         });
    // }

    template <class Func>
    constexpr auto values(Func &&f) const noexcept {
        return hana::unpack(hana::members(*static_cast<const D*>(this)),
            [&](auto &&...xs) {
                return std::array { std::invoke(f, xs) ... };
            });
    }

    template <class AccVal, class Func>
    constexpr AccVal accumulate_fields(AccVal&& acc, Func &&f) noexcept {
        boost::hana::for_each(hana::members(*static_cast<D*>(this)), [&]<typename T>(T&& t) {
            acc = std::invoke(f, std::forward<AccVal>(acc), std::forward<T>(t));
        });
        return std::forward<AccVal>(acc);
    }
    // What if i want to edit values and not return as array
    // or print them just, and or/combine with keys
    // there is a lot to think about here
};

// TODO: It would be probably better to use specialization
// of REFL template, to make it non intrusive and more flexible
// also it would give us separation of REFL and non REFL thingies
struct User final : Refl<User> {
    uint64_t id;
    std::string firstname;
    std::string lastname;
};

REFL(User, id, firstname, lastname);

int main() 
{
    User u {
        .id = 0, 
        .firstname = "Nebojsa", 
        .lastname = "Koturovic"
    };

    fmt::print("{}\n", User::keys());
    fmt::print("{}\n", u.keys());

    constexpr std::array ks = User::keys();
    std::array vs = u.values([](const auto &x) {
        return fmt::format("{}", x);
    });

    fmt::print("{}\n", vs);

    for (const auto &[k,v] : vw::zip(ks, vs)) {
        fmt::print("{} : {}\n", k, v);
    }

    auto result = 
        u.accumulate_fields(std::array<std::pair<unsigned, std::string>, User::num_of_fields()>{},
            [i = 0u](auto &&acc, auto &&val) mutable {
                acc[i++] = {i, fmt::format("{}", val)};
                return std::move(acc);
            });
    
    fmt::print("{}\n", result);

    return 0;
}
