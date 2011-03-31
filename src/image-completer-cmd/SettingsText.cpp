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

#include "tech/StrUtils.h"

#include "tech/DbgMem.h"

//
// SettingsText
//
void SettingsText::Print(const LfnIc::Settings& settings, CommandLineOptions commandLineOptions)
{
	wxASSERT(LfnIc::AreSettingsValid(settings));
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	std::string lowResolutionPassesMaxString;
	if (settings.lowResolutionPassesMax == LfnIc::Settings::LOW_RESOLUTION_PASSES_AUTO)
	{
		lowResolutionPassesMaxString = GetLowResolutionPassesAutoDescription();
	}
	else
	{
		lowResolutionPassesMaxString = LfnTech::Str::Format("%d", settings.lowResolutionPassesMax);
	}

#define VAL_W "17" /* TODO: auto-size this as well? */
#define VAL_X(fmt, x) LfnTech::Str::Format("%" VAL_W fmt, x)
#define VAL_S(s) VAL_X("s", s)
#define VAL_I(i) VAL_X("d", i)

	commandLineOptions.Low_Resolution_Passes_Max.m_strValue = VAL_S(lowResolutionPassesMaxString.c_str());
	commandLineOptions.Num_Iterations.m_strValue = VAL_I(settings.numIterations);
	commandLineOptions.Lattice_Width.m_strValue = VAL_I(settings.latticeGapX);
	commandLineOptions.Lattice_Height.m_strValue = VAL_I(settings.latticeGapY);
	commandLineOptions.Patches_Min.m_strValue = VAL_I(settings.postPruneLabelsMin);
	commandLineOptions.Patches_Max.m_strValue = VAL_I(settings.postPruneLabelsMax);

	commandLineOptions.Compositor_Patch_Type.m_strValue = VAL_S(GetEnumDescription(settings.compositorPatchType).c_str());
	commandLineOptions.Compositor_Patch_Blender.m_strValue = VAL_S(GetEnumDescription(settings.compositorPatchBlender).c_str());
/*
	// Determine the description column width.
	int descWidth = 0;
	{
		const std::vector<Member>* memberVectors[] =
		{
			&completerMembers,
			&compositorMembers,
		};
		const int memberVectorNum = sizeof(memberVectors) / sizeof(memberVectors[0]);

		for (int memberVectorIdx = 0; memberVectorIdx < memberVectorNum; ++memberVectorIdx)
		{
			const std::vector<Member>& members = *memberVectors[memberVectorIdx];
			for (int i = 0, n = members.size(); i < n; ++i)
			{
				const int len = members[i].desc.length();
				if (descWidth < len)
				{
					descWidth = len;
				}
			}
		}
	}
*/

/*
	// Display the members
	{
		msgOut.Printf("\nImage completer settings\n");
		for (int i = 0, n = completerMembers.size(); i < n; ++i)
		{
			completerMembers[i].Print(msgOut, descWidth);
		}

		msgOut.Printf("Compositor settings\n");
		for (int i = 0, n = compositorMembers.size(); i < n; ++i)
		{
			compositorMembers[i].Print(msgOut, descWidth);
		}
	}
*/
}

std::string SettingsText::GetLowResolutionPassesAutoDescription()
{
	return "auto";
}

std::string SettingsText::GetEnumDescription(LfnIc::CompositorPatchType e)
{
	std::string desc;
	switch (e)
	{
	case LfnIc::CompositorPatchTypeNormal:        desc = "normal"; break;
#ifdef USE_POISSON
	case LfnIc::CompositorPatchTypePoisson:       desc = "poisson"; break;
#endif
	case LfnIc::CompositorPatchTypeDebugOrder:    desc = "debug-patch-order"; break;
	default:                                           desc = "unknown"; break;
	}

	return desc;
}

std::string SettingsText::GetEnumDescription(LfnIc::CompositorPatchBlender e)
{
	std::string desc;
	switch (e)
	{
	case LfnIc::CompositorPatchBlenderPriority:   desc = "priority"; break;
	case LfnIc::CompositorPatchBlenderNone:       desc = "none"; break;
	default:                                           desc = "unknown"; break;
	}

	return desc;
}

//
// SettingsText::PrintInvalidMembers
//
SettingsText::PrintInvalidMembers::PrintInvalidMembers()
	: m_msgOut(*wxMessageOutput::Get())
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

	const std::string member(SettingsText::GetMemberDescription(memberOffset));
	m_msgOut.Printf("\t- %s %s\n", member.c_str(), message);
}
