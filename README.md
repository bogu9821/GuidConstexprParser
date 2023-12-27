# GuidParserConstexpr
## Example
```cpp
#include <algorithm>    //std::ranges::equal
#include <array>        //std::array

using namespace GuidParser::GuidLiteral;

static constexpr auto guid = "{20f892f5-b841-4c46-8547-7d2e7687a30a}"_guid;
int main() 
{
    static constexpr std::array<std::uint8_t,8> MyData4 = { 133, 71, 125, 46, 118, 135, 163, 10 };
    static_assert(guid.Data1 == 553161461 && guid.Data2 == 47169 && guid.Data3 == 19526 && std::ranges::equal(MyData4, guid.Data4));

    auto optionalGuid = GuidParser::StringToGuid("{474ccf9f-bad7-48e2-b7c6-df307b74e4a5}");

    if(optionalGuid.has_value())
    {
        return 0;
    }

    return -1;
}
```
