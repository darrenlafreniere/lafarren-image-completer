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
#include "SettingsText.h"

#include "tech/DbgMem.h"

//
// SettingsText
//
std::string SettingsText::GetLowResolutionPassesAutoDescription()
{
	return "auto";
}

std::string SettingsText::GetEnumDescription(LfnIc::CompositorPatchType e)
{
	std::string desc;
	switch (e)
	{
	case LfnIc::CompositorPatchTypeNormal:     desc = "normal"; break;
#ifdef POISSON_COMPOSITING
	case LfnIc::CompositorPatchTypePoisson:    desc = "poisson"; break;
#endif
	case LfnIc::CompositorPatchTypeDebugOrder: desc = "debug-patch-order"; break;
	default:                                   desc = "unknown"; break;
	}

	return desc;
}

std::string SettingsText::GetEnumDescription(LfnIc::CompositorPatchBlender e)
{
	std::string desc;
	switch (e)
	{
	case LfnIc::CompositorPatchBlenderPriority: desc = "priority"; break;
	case LfnIc::CompositorPatchBlenderNone:     desc = "none"; break;
	default:                                    desc = "unknown"; break;
	}

	return desc;
}

//
// SettingsText::PrintInvalidMembers
//
SettingsText::PrintInvalidMembers::PrintInvalidMembers(const CommandLineOptions& commandLineOptions)
	: m_commandLineOptions(commandLineOptions)
	, m_msgOut(*wxMessageOutput::Get())
	, m_hasPrintedHeader(false)
{
}

void SettingsText::PrintInvalidMembers::OnInvalidMemberDetected(const LfnIc::Settings& settings, int memberOffset, const char* message)
{
	if (!m_hasPrintedHeader)
	{
		m_msgOut.Printf("Invalid settings:\n");
		m_hasPrintedHeader = true;
	}

	const CommandLineOptions::Option* option = m_commandLineOptions.FindOptionById(memberOffset);
	m_msgOut.Printf("\t- %s %s\n", option->longName.c_str(), message);
}
