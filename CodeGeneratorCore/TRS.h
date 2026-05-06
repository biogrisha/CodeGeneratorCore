#pragma once
#include "Structs.h"
#include <string>
#include <map>
#include <memory>

class TRSOptimizer
{
public:
	TRSOptimizer(std::map<std::string, std::unique_ptr<Term>>& egraph)
		: egraph(egraph){ }
	void optimize();
private:
	void saturate(const std::vector<Identity>& ids, int iterations);
	void reduce(const std::vector<Identity>& ids);
	void minimize();
	std::map<std::string, std::unique_ptr<Term>>& egraph;
};