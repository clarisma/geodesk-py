// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Compiler.h"
#include "GolBuilder.h"

CompilerContext::CompilerContext(Compiler* compiler) :
	compiler_(compiler)
{
}

void CompilerContext::processTask(int pile)
{
	// TODO
	compiler_->builder_->featurePiles().load(pile);
	CompilerOutputTask output;
	compiler_->postOutput(std::move(output));
}

Compiler::Compiler(GolBuilder* builder) :
	TaskEngine<Compiler, CompilerContext, int, CompilerOutputTask>(
		builder->threadCount()),
	builder_(builder),
	workPerTile_(builder->phaseWork(GolBuilder::Phase::COMPILE)
		/ builder->tileCatalog().tileCount())
{

}

void Compiler::compile()
{
	for (int i = 0; i < builder_->tileCatalog().tileCount(); i++)
	{
		postWork(std::move(i+1));
		// Pile numbers start at 1, not 0
	}
}

void Compiler::processTask(CompilerOutputTask& task)
{
	// TODO: save tile
	builder_->progress(workPerTile_);
}
