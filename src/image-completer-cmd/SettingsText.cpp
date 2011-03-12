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


// Gather member descriptions and values before printing them, so we can
// size the desc column to an ideal width.
struct Member
{
	std::string desc;
	std::string value;

	Member(const std::string& desc, const std::string& value) : desc(desc), value(value) {}

	void Print(wxMessageOutput& msgOut, int descWidth) const
	{
		const std::string descSpacing(' ', descWidth - desc.length());
		msgOut.Printf("\t%s:%s %s\n", desc.c_str(), descSpacing.c_str(), value.c_str());
	}
};

//
// SettingsText
//
void SettingsText::Print(const PriorityBp::Settings& settings)
{
	wxASSERT(PriorityBp::AreSettingsValid(settings));
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	std::string lowResolutionPassesMaxString;
	if (settings.lowResolutionPassesMax == PriorityBp::Settings::LOW_RESOLUTION_PASSES_AUTO)
	{
		lowResolutionPassesMaxString = GetLowResolutionPassesAutoDescription();
	}
	else
	{
		lowResolutionPassesMaxString = Tech::Str::Format("%d", settings.lowResolutionPassesMax);
	}

#define DESC(member) GetMemberDescription(offsetof(PriorityBp::Settings, member))
#define VAL_W "17" /* TODO: auto-size this as well? */
#define VAL_X(fmt, x) Tech::Str::Format("%" VAL_W fmt, x)
#define VAL_S(s) VAL_X("s", s)
#define VAL_I(i) VAL_X("d", i)

	std::vector<Member> completerMembers;
	completerMembers.push_back(Member(DESC(lowResolutionPassesMax), VAL_S(lowResolutionPassesMaxString.c_str())));
	completerMembers.push_back(Member(DESC(numIterations), VAL_I(settings.numIterations)));
	completerMembers.push_back(Member(DESC(latticeGapX), VAL_I(settings.latticeGapX)));
	completerMembers.push_back(Member(DESC(latticeGapY), VAL_I(settings.latticeGapY)));
	completerMembers.push_back(Member(DESC(postPruneLabelsMin), VAL_I(settings.postPruneLabelsMin)));
	completerMembers.push_back(Member(DESC(postPruneLabelsMax), VAL_I(settings.postPruneLabelsMax)));

	std::vector<Member> compositorMembers;
	compositorMembers.push_back(Member(DESC(compositorPatchType), VAL_S(GetEnumDescription(settings.compositorPatchType).c_str())));
	compositorMembers.push_back(Member(DESC(compositorPatchBlender), VAL_S(GetEnumDescription(settings.compositorPatchBlender).c_str())));

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
}

// Returns a user friendly string describing the specified settings
// member. settingsMemberOffset is the offset in byte of the settings
// member (e.g., offsetof(PriorityBp::Settings, latticeGapX))
std::string SettingsText::GetMemberDescription(int settingsMemberOffset)
{
	std::string desc;

	if (settingsMemberOffset == offsetof(PriorityBp::Settings, lowResolutionPassesMax))
	{
		desc = "low resolution passes";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, numIterations))
	{
		desc = "number of iterations";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, latticeGapX))
	{
		desc = "lattice gap width";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, latticeGapY))
	{
		desc = "lattice gap height";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, patchWidth))
	{
		desc = "patch width";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, patchHeight))
	{
		desc = "patch height";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, confidenceBeliefThreshold))
	{
		desc = "confidence belief threshold";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, pruneBeliefThreshold))
	{
		desc = "prune belief threshold";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, pruneEnergySimilarThreshold))
	{
		desc = "prune energy similarity threshold";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, postPruneLabelsMin))
	{
		desc = "post-prune patches min";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, postPruneLabelsMax))
	{
		desc = "post-prune patches max";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, compositorPatchType))
	{
		desc = "patch type";
	}
	else if (settingsMemberOffset == offsetof(PriorityBp::Settings, compositorPatchBlender))
	{
		desc = "patch blender";
	}
	else
	{
		desc = "unknown property";
	}

	return desc;
}

std::string SettingsText::GetLowResolutionPassesAutoDescription()
{
	return "auto";
}

std::string SettingsText::GetEnumDescription(PriorityBp::CompositorPatchType e)
{
	std::string desc;
	switch (e)
	{
	case PriorityBp::CompositorPatchTypeNormal:        desc = "normal"; break;
#ifdef USE_POISSON
	case PriorityBp::CompositorPatchTypePoisson:       desc = "poisson"; break;
#endif
	case PriorityBp::CompositorPatchTypeDebugOrder:    desc = "debug-patch-order"; break;
	default:                                           desc = "unknown"; break;
	}

	return desc;
}

std::string SettingsText::GetEnumDescription(PriorityBp::CompositorPatchBlender e)
{
	std::string desc;
	switch (e)
	{
	case PriorityBp::CompositorPatchBlenderPriority:   desc = "priority"; break;
	case PriorityBp::CompositorPatchBlenderNone:       desc = "none"; break;
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

void SettingsText::PrintInvalidMembers::OnInvalidMemberDetected(const PriorityBp::Settings& settings, int memberOffset, const char* message)
{
	if (!m_hasPrintedHeader)
	{
		m_msgOut.Printf("Invalid settings:\n");
		m_hasPrintedHeader = true;
	}

	const std::string member(SettingsText::GetMemberDescription(memberOffset));
	m_msgOut.Printf("\t- %s %s\n", member.c_str(), message);
}
