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

//
// Contains inline implementations of CmdLine.h definitions.
//
#ifndef TECH_CMD_LINE_INL
#define TECH_CMD_LINE_INL

#ifndef INCLUDING_TECH_CMD_LINE_INL
#error "CmdLine.inl must only be included by CmdLine.h"
#endif

namespace Tech
{
	// CmdLine::ParamOption<std::string> partial specialization.
	template<>
	void CmdLine::ParamOption<std::string>::ReadOption(const char* option)
	{
		value = option;
	}

	// CmdLine::ParamOption<int> partial specialization.
	template<>
	void CmdLine::ParamOption<int>::ReadOption(const char* option)
	{
		value = atoi(option);
	}
}

#endif
