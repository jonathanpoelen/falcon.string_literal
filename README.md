# falcon.string_literal

High level constexpr interface similar to string_view for literal strings.


``` cpp
using namespace falcon::make_string_literal_shortcut;

constexpr auto config_path = lit("project/config.ini");
constexpr auto dirname = config_path.substr<0, config_path.find('/')>();
constexpr auto new_config_path = dirname + "/new_config";
```
