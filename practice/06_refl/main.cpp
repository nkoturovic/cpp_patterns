#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>
#include <range/v3/view.hpp>
#include <functional>
#include <variant>

#include <boost/hana.hpp>
#include <boost/hana/adapt_struct.hpp>

// for_each over tuple elements
template <typename TupleT, typename Fn>
constexpr void for_each_tuple(TupleT&& tp, Fn&& fn) {
    std::apply
    (
        [&fn]<typename ...T>(T&& ...args)
        {
            (fn(std::forward<T>(args)), ...);
        }, std::forward<TupleT>(tp)
    );
}

template <typename TupleT, typename Fn>
constexpr auto transform_tuple(TupleT&& tp, Fn&& fn) {
    return std::apply(
        [&fn]<typename ...T>(T&& ...args)
        {
            return std::tuple { std::forward<Fn>(fn)(std::forward<T>(args))...};
        }, std::forward<TupleT>(tp)
    );
}


#define MTA_REFL BOOST_HANA_ADAPT_STRUCT

namespace hana = boost::hana;
namespace rng = ranges;
namespace vw = ranges::views;

struct User {
    uint64_t id;
    std::string firstname;
    std::string lastname;
};

MTA_REFL(User, id, firstname, lastname);

// Reflection for classes
namespace mta {

template <typename Callable>
struct FieldDescriptor {
    // using value_type = 
    std::string_view name;
    Callable value_getter;
};

// template <typename Callable>
// FieldDescriptor(std::string_view, Callable) -> FieldDescriptor<Callable>;

template <typename T>
struct Refl {
    consteval auto fields() const {
        return hana::unpack(hana::transform(hana::accessors<T>(), [](auto x) {
            return FieldDescriptor{.name = std::string_view{hana::first(x).c_str()}, 
                                   .value_getter = [x](T& t) -> decltype(auto) { return hana::second(x)(t); }};
        }),  []<class ...Xs>(Xs ...xs) { return std::tuple{xs...}; });
    }
};
} // namespace mta

int main() 
{
    User u {
        .id = 2u,
        .firstname = "Nebojsa",
        .lastname = "Koturovic",
    };

    constexpr static auto user_refl = mta::Refl<User>{};
    constexpr static auto fields = user_refl.fields();

    for_each_tuple(fields, [&u](auto fd) {
        fmt::print("{} -> {}\n", fd.name, fd.value_getter(u));
               //fd.value_getter(u) = "Shone";
    });

    constexpr static auto res = transform_tuple(fields, [](auto fd) {
        // if constexpr (fd.name == std::string_view{"name"}) {
        //      return std::string_view{"name"};
        //} else {
            return fd.name;
       // }
    });

    fmt::print("{}\n", res);

    return 0;
}

// std::apply([&u](auto ...fs) { 
//     (fmt::print("{} -> {}\n", fs.name, fs.value_getter(u)), ...);
// }, fields);
// ===========================
// template <typename T>
// struct print_type;
// ===========================
//print_type<decltype(fields)>{};
// hana::for_each(fields, [&u](auto f) {
//    // fmt::print("{} -> {}\n", hana::first(f), hana::second(f)(u));
// });


