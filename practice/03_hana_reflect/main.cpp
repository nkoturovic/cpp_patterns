#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <array>

#include <boost/hana.hpp>
#include <boost/hana/adapt_struct.hpp>

namespace hana = boost::hana;

struct User {
    uint64_t id;
    std::string firstname;
    std::string lastname;
};

BOOST_HANA_ADAPT_STRUCT(User, id, firstname, lastname);

int main() 
{
    User u {
        .id = 0, 
        .firstname = "Nebojsa", 
        .lastname = "Koturovic"
    };

    constexpr auto keys = hana::keys(u);
    constexpr auto keys_arr = 
        hana::unpack(
            keys, 
            [](auto ...ns) { 
                return std::array { ns.c_str()... };
            });

    fmt::print("{}\n", keys_arr);

    auto umap = hana::to_map(u); // not known at compile time because of std::string field
    hana::for_each(umap, [](const auto& p) {
            fmt::print("{} -> {}\n", hana::first(p).c_str(), hana::second(p));
    });

    return 0;
}
