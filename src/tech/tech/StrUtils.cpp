//
// Copyright 2010, Darren Lafreniere
// <http://www.lafarren.com/image-completer/>
// 
// This file is part of lafarren.com's Image Completer.
// 
// Image Completer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Image Completer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Image Completer, named License.txt. If not, see
// <http://www.gnu.org/licenses/>.
//

#include "Pch.h"
#include "tech/StrUtils.h"

#include "tech/DbgMem.h"

std::string LfnTech::Str::Format(const char* format, ...)
{
	static char buffer[1024 * 16];
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer, sizeof(buffer), format, argptr);
	va_end(argptr);
	return buffer;
}

std::string LfnTech::Str::Replace(const std::string& s, char replace, const char* with)
{
	std::string replaced;
	for (int i = 0, n = s.length(); i < n; ++i)
	{
		const char ch = s[i];
		if (ch == replace)
		{
			replaced += with;
		}
		else
		{
			replaced += ch;
		}
	}

	return replaced;
}
