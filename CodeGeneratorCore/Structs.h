#include <map>
#include <string>
#include <vector>

struct Dep
{
    struct Block* block = nullptr;
    int port = 0;
};

struct Block
{
    std::string type;
    std::string name;
    std::string sid;
    std::string meta;
    std::map<int, Dep> indeps;
    std::vector<Dep> outdeps;
};