#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>
#include <concepts>
#include <range/v3/view.hpp>
#include <functional>
#include <variant>

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

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


// Should all fields be const or not, what if I want to move field values??
template <typename D>
struct Refl {
    static std::size_t consteval num_of_fields() { 
        return keys_array().size();
    }

    // template <class Func>
    // constexpr auto key_value_array(Func &&f) const noexcept {
    //     const D * d = static_cast<const D*>(this);
    //     return hana::unpack(hana::to_map(*d),
    //         [&]<class ...Ps>(Ps&& ...ps) { 
    //             return std::array { std::make_pair(hana::first(std::forward<Ps>(ps)).c_str(), std::invoke(std::forward<Func>(f), hana::second(std::forward<Ps>(ps))))...};
    //         });
    // }

    constexpr auto key_value_tuple() const noexcept {
        const D * d = static_cast<const D*>(this);
        return hana::unpack(hana::to_map(*d),
            [&](auto ...ps) { 
                return std::tuple { std::make_pair(hana::first(ps).c_str(), hana::second(ps))...};
            });
    }

    static consteval auto keys_array()  noexcept {
        return hana::unpack(get_struct_keys<D>(), 
            [](auto ...ks) { 
                return std::array { ks.c_str()... };
            });
    }

    template <class Func>
    constexpr auto values_array(Func &&f) const noexcept {
        return hana::unpack(hana::members(*static_cast<const D*>(this)),
            [&]<class ...Xs>(Xs &&...xs) {
                return std::array { std::invoke(std::forward<Func>(f), std::forward<Xs>(xs)) ... };
            });
    }

    constexpr auto values_tuple() const noexcept {
        return hana::unpack(hana::members(*static_cast<const D*>(this)),
            [&]<class ...Xs>(Xs &&...xs) {
                return std::tuple{ std::forward<Xs>(xs)... };
            });
    }

    template <class Func>
    constexpr void for_each_key_value(Func &&f) const noexcept {
        boost::hana::for_each(hana::to_map(*static_cast<const D*>(this)), [&]<typename T>(T&& t) {
            std::invoke(f, hana::first(std::forward<T>(t)).c_str(), hana::second(std::forward<T>(t)));
        });
    }

    static consteval auto field_getters() {
        return hana::transform(hana::accessors<D>(), [](auto x) {
            //return std::pair{std::string_view{hana::first(x).c_str()}, hana::second(x)};
            return hana::pair{std::string_view{hana::first(x).c_str()}, hana::second(x)};
        });
    }

    constexpr static auto getter_for(std::string_view s) {
        constexpr auto fg = D::field_getters();
        constexpr auto fgeters = hana::transform(fg, hana::second);
        constexpr auto var_t = hana::unpack(fgeters, []<class ...Xs>(Xs .../*xs*/) {
                return hana::type_c<std::variant<Xs...>>;
        });
        typename decltype(var_t)::type result;

        hana::for_each(fg, [&](auto p) {
            if (hana::first(p) == s)
                result = hana::second(p);
        });
            return result;
    }

    auto constexpr get_value_for(std::string_view s) noexcept {

        constexpr auto types = decltype(hana::unpack(std::declval<D>(),
              hana::on(hana::make_tuple, hana::compose(hana::typeid_, hana::second)))){};

        constexpr auto unique_types = hana::unique(types);

        constexpr auto fg = D::field_getters();
        constexpr auto var_t = hana::unpack(unique_types, []<class ...Xs>(Xs .../*xs*/) {
                return hana::type_c<std::variant<typename Xs::type...>>;
        });
        typename decltype(var_t)::type result;
        //std::string result;

        hana::for_each(fg, [&](auto p) {
            if (hana::first(p) == s) {
                auto fn = (hana::second(p));
                auto ref = (*static_cast<D*>(this));
                result = std::invoke(fn, ref);
            }

        });
            return result;
    }

    // auto constexpr get_value_for(std::string_view s) noexcept {

    //     constexpr auto types = decltype(hana::unpack(std::declval<D>(),
    //           hana::on(hana::make_tuple, hana::compose(hana::typeid_, hana::second)))){};

    //     constexpr auto unique_types = hana::unique(types);

    //     constexpr auto fg = D::field_getters();
    //     constexpr auto var_t = hana::unpack(unique_types, []<class ...Xs>(Xs .../*xs*/) {
    //             return hana::type_c<std::variant<typename Xs::type...>>;
    //     });
    //     typename decltype(var_t)::type result;
    //     //std::string result;

    //     hana::for_each(fg, [&](auto p) {
    //         if (hana::first(p) == s) {
    //             auto fn = (hana::second(p));
    //             auto ref = (*static_cast<D*>(this));
    //             result = std::invoke(fn, ref);
    //         }

    //     });
    //         return result;
    // }


    static consteval auto getter_for_comptime(auto s) noexcept {
        constexpr auto acs = hana::accessors<D>();
        constexpr auto map = hana::unpack(acs, [](auto ...xs) {
                return hana::make_map(xs...);
        });
        return map[s];
    }

     //constexpr auto operator[](std::string_view str) {
     //    constexpr auto acs = hana::accessors<D>();
     //    constexpr auto new_acs = hana::transform(acs, [](auto ... xs) {
     //        return (hana::make_pair(std::string_view{hana::first(xs).c_str()}, hana::second(xs)), ...);
     //    });

         //return hana::to_map(*static_cast<D*>(this))[BOOST_HANA_STRING(str)];
    // }
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
        .id = 2u,
        .firstname = "Nebojsa",
        .lastname = "nebojsa",
    };

    u.for_each_key_value([](const char * name, const auto &val) {
        fmt::print("{}, {}\n", name, val);
    });

    std::array ks = User::keys_array();
    std::array vs = u.values_array(overloaded {
        [](const std::string &s) {
            return s;
        },
        [](uint64_t x) {
            return std::to_string(x*x);
        }
    });

    for (const auto &[k, v] : vw::zip(ks, vs)) {
        fmt::print("{}, {}\n", k, v);
    }

    // for (const auto &[k, v] : 
    //         u.key_value_array([](const auto &x) { return fmt::format("{}", x); })) {
    //     fmt::print("{}, {}\n", k, v);
    // }

    fmt::print("{}\n", u.key_value_tuple());
    fmt::print("{}\n", u.values_tuple());

    //fmt::print("{}", u["firstname"]);
    //hana::for_each(u.field_getters(), [&u](auto g) {
    //    fmt::print("{}\n", g.first);
    //});
    std::visit([&](auto f) { fmt::print("{}\n", f(u)); }, User::getter_for("id"));
    auto vnt = u.get_value_for("firstname");
    std::visit([&](auto val) { fmt::print("{}\n", val); }, vnt);

    constexpr auto fn_get_id = User::getter_for_comptime(BOOST_HANA_STRING("id"));
    constexpr auto fn_get_firstname = User::getter_for_comptime(BOOST_HANA_STRING("firstname"));

    fmt::print("<{}, {}>\n", fn_get_id(u), fn_get_firstname(u));

    return 0;
}
