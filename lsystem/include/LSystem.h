#pragma once
#include "LSysConfig.h"

#include <string>
#include <vector>
#include <memory>

namespace LSystem {
	class LSystem {
	private:
		std::string axiom;
		std::string current;
		//std::vector<std::shared_ptr<LSysConfig::LProduction>> productions;
		std::shared_ptr<LSysConfig::LSystem> lsystem;
	public:
		LSystem(const std::string& axiom, std::shared_ptr<LSysConfig::LSystem> lsystem) : axiom(axiom), current(axiom), lsystem(lsystem){}
		LSystem(const std::string& axiom, const std::string& productions);
		LSystem(const std::string& axiom, const std::vector<std::string>& productions);

		std::string getCurrent()const;

		std::string iterate();
		std::string iterate(uint32_t n);
		void reset();
	};
}
