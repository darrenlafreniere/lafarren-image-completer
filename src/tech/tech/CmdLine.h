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
// Contains image related utilities.
//
#ifndef TECH_CMD_LINE_H
#define TECH_CMD_LINE_H

#include <wx/string.h>

namespace Lafarren
{
	//
	// Reads a command line into separate parameter objects.
	// See AddParam() and Read() methods for more info.
	//
	class CmdLine
	{
	public:
		// Abstract base parameter class.
		struct Param
		{
			const wxString shortName;
			const wxString longName;
			const wxString description;
			bool isSet;

			Param(const wxString& shortName, const wxString& longName, const wxString description);
			void Set();

			virtual bool IsOption() const = 0;
			virtual void ReadOption(const char* option) = 0;
		};

		// True/false switch parameters. True if arg is present.
		struct ParamSwitch : public Param
		{
			ParamSwitch(const wxString& shortName, const wxString& longName, const wxString description) : Param(shortName, longName, description) {}
			virtual bool IsOption() const { return false; }
			virtual void ReadOption(const char* option) {}
		};

		// Option parameters that contain a strongly typed value.
		template<typename T>
		struct ParamOption : public Param
		{
			T value;

			ParamOption(const wxString& shortName, const wxString& longName, const wxString description) : Param(shortName, longName, description) {}
			virtual bool IsOption() const { return true; }
			// Implement ReadOptions in a partial template specialization.
			virtual void ReadOption(const char* option) {}
		};

		// The passed in Param instance must be valid for the lifetime of this
		// CmdLine instance. Asserts/ignores parameters that have the same short
		// or long name as an existing parameter.
		void AddParam(Param& param);

		// Reads argc/argv by finding an Param instance with a matching
		// shortName or longName. When found, it sets the parameter, and if
		// the param is an option it delegates to the parameter instance to
		// read the very next argv element.
		//
		// If an unknown option was passed, or an expected option argv element
		// isn't present, the method stops reading and returns false.
		bool Read(int argc, char** argv, wxString& outError) const;

		// Generates a usage string based on the contained Param instances.
		wxString GetUsageString() const;
		inline void PrintfUsage() const { printf(GetUsageString().c_str()); }

	private:
		Param* GetParam(const char* arg) const;

		std::vector<Param*> m_params;
	};
}

//
// Include the inline implementations
//
#define INCLUDING_TECH_CMD_LINE_INL
#include "tech/CmdLine.inl"
#undef INCLUDING_TECH_CMD_LINE_INL

#endif
