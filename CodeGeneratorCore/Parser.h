#pragma once
#include "Structs.h"
#include <memory>
#include <optional>
#include "rapidxml-1.13/rapidxml.hpp"

class Parser
{
public:
	std::optional<std::map<std::string, std::unique_ptr<Block>>> parse(const std::string& file_path);
private:
    static std::string get_attr(rapidxml::xml_node<>* node, const char* name);
    static std::vector<int> parse_int_list(const std::string& s);
    static void parseAdd(rapidxml::xml_node<>* node, Block* block);
    static void parseGain(rapidxml::xml_node<>* node, Block* block);
    static void parseUnitDelay(rapidxml::xml_node<>* node, Block* block);
    static void splitDelays(std::map<std::string, std::unique_ptr<Block>>& blocks);
    static bool parseDstBranch(rapidxml::xml_node<>* node, std::vector<Dep>& dsts, const std::map<std::string, std::unique_ptr<Block>>& blocks);
};