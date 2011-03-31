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

#ifndef SETTINGS_TEXT_H
#define SETTINGS_TEXT_H

#include "LfnIcSettings.h"
#include "CommandLineOptions.h"

class SettingsText
{
public:
	// Prints the user-tweakable settings to wxMessageOutput.
	static void Print(const LfnIc::Settings& settings, CommandLineOptions commandLineOptions);

	// Returns the command line option used to indicate
	// LfnIc::Settings::LOW_RESOLUTION_PASSES_AUTO.
	static std::string GetLowResolutionPassesAutoDescription();

	// Returns a description string for an enum value.
	static std::string GetEnumDescription(LfnIc::CompositorPatchType e);
	static std::string GetEnumDescription(LfnIc::CompositorPatchBlender e);

	template<typename T>
	static std::string JoinEnumDescriptions()
	{
		std::string joined;
		for (int e = TypeInfo<T>::First; e <= TypeInfo<T>::Last; ++e)
		{
			if (!joined.empty())
			{
				joined += ", ";
			}
			joined += SettingsText::GetEnumDescription(T(e));
		}

		return joined;
	}

	// An instance of this class can be passed to LfnIc::Settings::IsValid. // should this be AreSettingsValid?
	// It'll display info for an invalid settings member(s).
	class PrintInvalidMembers : public LfnIc::SettingsInvalidMemberHandler
	{
	public:
		PrintInvalidMembers();
		virtual void OnInvalidMemberDetected(const LfnIc::Settings& settings, int memberOffset, const char* message);
        const CommandLineOptions* m_commandLineOptions;
	private:
		wxMessageOutput& m_msgOut;
		bool m_hasPrintedHeader;

	};
};

#endif // SETTINGS_TEXT_H
