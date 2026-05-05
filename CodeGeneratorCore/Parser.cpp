#include "Parser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <charconv>
#include "Helpers.h"

std::optional<std::map<std::string, std::unique_ptr<Block>>> Parser::parse(const std::string& file_path)
{
    std::map<std::string, std::unique_ptr<Block>> blocks;
    // Load file into memory
    std::ifstream file(file_path);
    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return {};
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    buffer.push_back('\0');

    try
    {
        std::unique_ptr<rapidxml::xml_document<char>> doc = std::make_unique<rapidxml::xml_document<char>>();

        doc->parse<0>(&buffer[0]);

        rapidxml::xml_node<>* root = doc->first_node("System");

        rapidxml::xml_node<>* block_node = root->first_node("Block");
        for (; block_node; block_node = block_node->next_sibling("Block"))
        {
            std::unique_ptr<Block> block = std::make_unique<Block>();
            block->type = get_attr(block_node, "BlockType");
            block->name = helpers::removeWhitespace(get_attr(block_node, "Name"));
            block->sid = get_attr(block_node, "SID");
            if (block->type == "Sum")
            {
                parseAdd(block_node, block.get());
            }
            else if (block->type == "Gain")
            {
                parseGain(block_node, block.get());
            }
            else if (block->type == "UnitDelay")
            {
                parseUnitDelay(block_node, block.get());
            }
            blocks.emplace(block->sid, std::move(block));
        }
        for (rapidxml::xml_node<>* line_node = root->first_node("Line"); line_node; line_node = line_node->next_sibling("Line"))
        {
            Dep src;
            std::vector<Dep> dsts;
            for (rapidxml::xml_node<>* p_node = line_node->first_node("P"); p_node; p_node = p_node->next_sibling("P"))
            {
                if (get_attr(p_node, "Name") == "Src")
                {
                    int src_sid = -1;
                    int src_port = -1;
                    auto [p, e] = std::from_chars(p_node->value(), p_node->value() + p_node->value_size(), src_sid);

                    if (e != std::errc())
                        return {};
                    constexpr int offset = std::string_view("#out:").size();
                    auto [p1, e1] = std::from_chars(p + offset, p_node->value() + p_node->value_size(), src_port);

                    if (e1 != std::errc())
                        return {};

                    auto block = blocks.find(std::to_string(src_sid));
                    if (block != blocks.end())
                    {
                        src.block = block->second.get();
                        src.port = src_port;
                    }
                }
                if (get_attr(p_node, "Name") == "Dst")
                {
                    int dst_sid = -1;
                    int dst_port = -1;
                    auto [p, e] = std::from_chars(p_node->value(), p_node->value() + p_node->value_size(), dst_sid);

                    if (e != std::errc())
                        return {};
                    constexpr int offset = std::string_view("#in:").size();
                    auto [p1, e1] = std::from_chars(p + offset, p_node->value() + p_node->value_size(), dst_port);

                    if (e1 != std::errc())
                        return {};

                    auto block = blocks.find(std::to_string(dst_sid));
                    if (block != blocks.end())
                    {
                        auto& dst = dsts.emplace_back();
                        dst.block = block->second.get();
                        dst.port = dst_port;
                    }
                }
            }
            if (dsts.empty())
            {
                for (rapidxml::xml_node<>* br_node = line_node->first_node("Branch"); br_node; br_node = br_node->next_sibling("Branch"))
                {
                    for (rapidxml::xml_node<>* p_node = br_node->first_node("P"); p_node; p_node = p_node->next_sibling("P"))
                    {
                        if (get_attr(p_node, "Name") == "Dst")
                        {
                            int dst_sid = -1;
                            int dst_port = -1;
                            auto [p, e] = std::from_chars(p_node->value(), p_node->value() + p_node->value_size(), dst_sid);

                            if (e != std::errc())
                                return {};
                            constexpr int offset = std::string_view("#in:").size();
                            auto [p1, e1] = std::from_chars(p + offset, p_node->value() + p_node->value_size(), dst_port);

                            if (e1 != std::errc())
                                return {};

                            auto block = blocks.find(std::to_string(dst_sid));
                            if (block != blocks.end())
                            {
                                auto& dst = dsts.emplace_back();
                                dst.block = block->second.get();
                                dst.port = dst_port;
                            }
                        }
                    }
                }
            }
            for (auto& dst : dsts)
            {
                dst.block->indeps.emplace(dst.port, Dep{ src.block, src.port });
                src.block->outdeps.push_back(dst);
            }
        }
    }
    catch (const rapidxml::parse_error& e)
    {
        std::cerr << "Parse error: " << e.what() << "\n";
        return {};
    }
    splitDelays(blocks);
    return std::optional(std::move(blocks));
}

std::string Parser::get_attr(rapidxml::xml_node<>* node, const char* name)
{
    if (auto* attr = node->first_attribute(name))
        return attr->value();
    return "";
}

std::vector<int> Parser::parse_int_list(const std::string& s)
{
    std::vector<int> result;

    const char* ptr = s.data();
    const char* end = ptr + s.size();

    // Skip '['
    if (ptr < end && *ptr == '[') ++ptr;

    while (ptr < end)
    {
        // Skip spaces and commas
        while (ptr < end && (*ptr == ' ' || *ptr == ',')) ++ptr;

        if (ptr >= end || *ptr == ']')
            break;

        int value = 0;
        auto res = std::from_chars(ptr, end, value);

        if (res.ec != std::errc())
            break; // or handle error

        result.push_back(value);
        ptr = res.ptr;
    }

    return result;
}

void Parser::parseAdd(rapidxml::xml_node<>* node, Block* block)
{
    for (rapidxml::xml_node<>* p_node = node->first_node("P"); p_node; p_node = p_node->next_sibling("P"))
    {
        if (get_attr(p_node, "Name") == "Inputs")
        {
            block->meta = p_node->value();
            return;
        }
    }
    block->meta = "++";
}

void Parser::parseGain(rapidxml::xml_node<>* node, Block* block)
{
    for (rapidxml::xml_node<>* p_node = node->first_node("P"); p_node; p_node = p_node->next_sibling("P"))
    {
        if (get_attr(p_node, "Name") == "Gain")
        {
            block->meta = p_node->value();
            return;
        }
    }
    block->meta = "1";
}

void Parser::parseUnitDelay(rapidxml::xml_node<>* node, Block* block)
{
    block->meta = "0,";
    for (rapidxml::xml_node<>* p_node = node->first_node("P"); p_node; p_node = p_node->next_sibling("P"))
    {
        
        if (get_attr(p_node, "Name") == "InitialCondition")
        {
            block->meta = p_node->value();
            block->meta += ',';
            continue;
        }
        if (get_attr(p_node, "Name") == "SampleTime")
        {
            block->meta += p_node->value();
            return;
        }
    }
    block->meta += "0.2";
}

void Parser::splitDelays(std::map<std::string, std::unique_ptr<Block>>& blocks)
{
    std::vector<std::unique_ptr<Block>> new_blocks;
    for (auto& bl : blocks)
    {
        if (bl.second->type == "UnitDelay")
        {
            auto& new_block = new_blocks.emplace_back(std::make_unique<Block>());
            new_block->meta = bl.second->meta;
            new_block->name = bl.second->name;
            new_block->sid = bl.second->sid + "in";
            new_block->type = "UnitDelayIn";
            for(auto& dep : bl.second->outdeps)
            {
                dep.block->indeps[dep.port].block = new_block.get();
            }
            new_block->outdeps = std::move(bl.second->outdeps);
            bl.second->outdeps.clear();
            bl.second->type = "UnitDelayOut";
        }
    }
    for (auto& bl : new_blocks)
    {
        auto str = bl->sid;
        blocks.emplace(str, std::move(bl));
    }
}
