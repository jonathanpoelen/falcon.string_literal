# falcon.string_literal
High level constexpr interface to the string literals.

Similar to std::string_view.

``` cpp
using namespace falcon::make_string_literal_shortcut;

constexpr auto config_path = lit("project/config.ini");
constexpr auto dirname = config_path.substr<0, config_path.find('/')>();
constexpr auto new_config_path = dirname + "/new_config";
```
