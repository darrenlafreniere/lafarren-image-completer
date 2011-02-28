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

#ifndef SETTINGS_UI_H
#define SETTINGS_UI_H

#include "PriorityBpSettings.h"

class SettingsUi
{
public:
	// Prints the user-tweakable settings to wxMessageOutput.
	static void Print(const PriorityBp::Settings& settings);

	// Returns a user friendly string describing the specified settings
	// member. settingsMemberOffset is the offset in byte of the settings
	// member (e.g., offsetof(PriorityBp::Settings, latticeGapX))
	static wxString GetMemberDescription(int settingsMemberOffset);

	// Returns the command line option used to indicate
	// PriorityBp::Settings::LOW_RESOLUTION_PASSES_AUTO.
	static wxString GetLowResolutionPassesAutoDescription();

	// Returns a description string for an enum value.
	static wxString GetEnumDescription(PriorityBp::CompositorPatchType e);
	static wxString GetEnumDescription(PriorityBp::CompositorPatchBlender e);

	// An instance of this class can be passed to PriorityBp::Settings::IsValid.
	// It'll display info for an invalid settings member(s).
	class PrintInvalidMembers : public PriorityBp::Settings::InvalidMemberHandler
	{
	public:
		PrintInvalidMembers();
		virtual void OnInvalidMemberDetected(int memberOffset, const char* message);

	private:
		wxMessageOutput& m_msgOut;
		bool m_hasPrintedHeader;
	};
};

#endif // SETTINGS_UI_H
