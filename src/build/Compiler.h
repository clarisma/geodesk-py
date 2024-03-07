// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/alloc/ReusableBlock.h>
#include <common/thread/TaskEngine.h>

class Compiler;
class GolBuilder;

class CompilerContext
{
public:
	CompilerContext(Compiler* compiler);
	void processTask(int pile);
	void afterTasks() {}
	void harvestResults() {}

private:
	Compiler* compiler_;
	ReusableBlock data_;
};

class CompilerOutputTask
{
	
};


class Compiler : public TaskEngine<Compiler, CompilerContext, int, CompilerOutputTask>
{
public:
	Compiler(GolBuilder* builder);

	void compile();
	void processTask(CompilerOutputTask& task);

private:
	GolBuilder* builder_;
	double workPerTile_;

	friend class CompilerContext;
};
