// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Console.h"
#include <unistd.h> // For write and ioctl
#include <sys/ioctl.h> // For ioctl to get terminal size
#include <stdio.h> // For printf

void Console::init()
{
    // Get console width
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) 
    {
        if(w.ws_col != 0) consoleWidth_ = w.ws_col;
    }
        
    // Setting the terminal to UTF-8 and cursor visibility can be done via
    // terminal commands if necessary. Typically, cursor visibility and UTF-8
    // settings should be managed by the terminal emulator settings.

    // Hide the cursor via ANSI control code
    printf("\e[?25l");
}

void Console::restore()
{
    // Re-enable the cursor via ANSI control code
    printf("\e[?25h");
}

void Console::print(const char* s, size_t len)
{
    // Directly write to STDOUT_FILENO
    write(STDOUT_FILENO, s, len);
}

