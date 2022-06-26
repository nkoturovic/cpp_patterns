#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>
#include <range/v3/view.hpp>
#include <functional>
#include <variant>
#include <optional>

#include <boost/hana.hpp>
#include <boost/hana/adapt_struct.hpp>
#include <boost/hana/optional.hpp>

template <typename T>
struct print_type;


#define MTA_REFL BOOST_HANA_ADAPT_STRUCT
#define MTA_STR BOOST_HANA_STRING

namespace hana = boost::hana;
namespace rng = ranges;
namespace vw = ranges::views;


constexpr auto to_std_tuple(auto hana_tuple) {
    return hana::unpack(hana_tuple, [](auto ...xs) {
        return std::make_tuple(xs...);
    });
}

constexpr auto to_hana_tuple(auto std_tuple) {
    return std::apply([]<typename ...T>(T&& ...args) {
        return hana::make_tuple(args...);
    }, std_tuple);
}



template <typename TupleT>
struct TupleView {
    TupleT in;

    template <typename Fn>
    constexpr auto transform(Fn &&fn) {
        auto new_tuple = transform_tuple(in, std::forward<Fn>(fn));
        return TupleView<std::decay_t<decltype(new_tuple)>>(new_tuple);
    }

    constexpr TupleT to_tuple() {
        return in;
    }
};

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

template <typename TupleT, typename Predicate>
static consteval auto filter_tuple(TupleT t, Predicate &&predicate) {
    constexpr auto hana_tuple = to_hana_tuple(t);
    return to_std_tuple(hana::filter(hana_tuple, std::forward<Predicate>(predicate)));
}


// template <typename TupleT, typename ReduceFunc>
// static consteval auto reduce_tuple(TupleT t, ReduceFunc &&reduce_func, auto initial_value) {
//     constexpr auto hana_tuple = to_hana_tuple(t);
//     return to_std_tuple(hana::filter(hana_tuple, std::forward<Predicate>(predicate)));
// }

struct User {
    uint64_t id;
    std::string firstname;
    std::string lastname;
};

MTA_REFL(User, id, firstname, lastname);

// Reflection for classes
namespace mta {

template <typename ClassType, auto Name, auto MemPtr>
struct FieldDescriptor {
    using class_type = ClassType;
    using member_type = decltype(hana::typeid_(MemPtr))::type;
    constexpr static std::string_view name = Name;
    constexpr static const auto member_ptr = MemPtr;
};

// template <typename Callable>
// FieldDescriptor(std::string_view, Callable) -> FieldDescriptor<Callable>;

template <typename T>
class Refl {
public:
    consteval static auto get_fields() {
        return hana::unpack(hana::transform(hana::accessors<T>(), [](auto x) {
            return FieldDescriptor<T, hana::first(x).c_str(),  
                                   hana::second(x)>{};
        }),  []<class ...Xs>(Xs ...xs) { return std::tuple{xs...}; });
    }

    static consteval auto get_field(auto s) noexcept {
        constexpr auto acs = hana::accessors<T>();
        constexpr auto map = hana::unpack(acs, [](auto ...xs) {
                return hana::make_map(xs...);
        });
        return FieldDescriptor<T, s.c_str(), map[s]>{};
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

    using user_refl = mta::Refl<User>;
    constexpr static auto fields = user_refl::get_fields();

    for_each_tuple(fields, [&u]<typename F>(F) {
        fmt::print("{} -> {}\n", F::name, F::member_ptr(u));
               //f.member_ptr(u) = "Shone";
    });

    constexpr static auto filtered = filter_tuple(user_refl::get_fields(), []<typename F>(F) {
        if constexpr (F::name == std::string_view{"firstname"}) {
            return std::true_type{};
        } else {
            return std::false_type{};
       }
    });

    constexpr static auto res = transform_tuple(filtered, []<typename F>(F) {
        return F::name;
    });


    fmt::print("{}\n", res);
    fmt::print("{}\n", user_refl::get_field(MTA_STR("firstname")).name);
    constexpr auto ht = to_hana_tuple(res);
    fmt::print("{}\n", to_std_tuple(ht));

    constexpr auto tup1 = TupleView{.in = res}
                .transform([](auto sv) { return 2; })
                .transform([](auto num) { return num * num; })
                .to_tuple();

    fmt::print("{}\n", tup1);

    auto tup2 = TupleView{.in = res}
                .transform([](auto sv) { return std::string{"Hello "}.append(sv); })
                .transform([](auto num) { return num + " " + num; })
                .to_tuple();

    fmt::print("{}\n", tup2);

    return 0;
}

// std::apply([&u](auto ...fs) { 
//     (fmt::print("{} -> {}\n", fs.name, fs.member_ptr(u)), ...);
// }, fields);
// ===========================
// template <typename T>
// struct print_type;
// ===========================
//print_type<decltype(fields)>{};
// hana::for_each(fields, [&u](auto f) {
//    // fmt::print("{} -> {}\n", hana::first(f), hana::second(f)(u));
// });


