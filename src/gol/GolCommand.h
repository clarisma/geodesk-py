// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/cli/CliCommand.h>
#include <common/io/File.h>

class GolCommand : CliCommand
{
public:
	GolCommand();

	void run(char* argv[]) { CliCommand::run(argv); }

	static std::string pathWithExtension(std::string_view path, const char* ext)
	{
		return (*File::extension(path) != 0) ?
			std::string(path) : std::string(path) + ext;
	}

	static std::string golPath(std::string_view path)
	{
		return pathWithExtension(path, ".gol");
	}

private:
	void setParam(int number, std::string_view value) override;
	void setOption(std::string_view name, std::string_view value) override;

	std::string golPath_;
};
