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
#include "CommandLineOptions.h"

#include <wx/cmdline.h>
#include "SettingsUi.h"

#include "tech/DbgMem.h"

//
// Constants
//
static const wxString CMD_IMAGE_INPUT                               = "ii";
static const wxString CMD_IMAGE_MASK                                = "im";
static const wxString CMD_IMAGE_OUTPUT                              = "io";

static const wxString CMD_SETTINGS_SHOW                             = "ss";
static const wxString CMD_SETTINGS_LOW_RESOLUTION_PASSES_MAX        = "sp";
static const wxString CMD_SETTINGS_DEBUG_LOW_RESOLUTION_PASSES      = "sd";
static const wxString CMD_SETTINGS_NUM_ITERATIONS                   = "si";
static const wxString CMD_SETTINGS_LATTICE_GAP_WIDTH                = "sw";
static const wxString CMD_SETTINGS_LATTICE_GAP_HEIGHT               = "sh";
static const wxString CMD_SETTINGS_POST_PRUNE_PATCHES_MIN           = "smn";
static const wxString CMD_SETTINGS_POST_PRUNE_PATCHES_MAX           = "smx";
static const wxString CMD_SETTINGS_COMPOSITOR_PATCH_TYPE            = "sct";
static const wxString CMD_SETTINGS_COMPOSITOR_PATCH_BLENDER         = "scb";

#if ENABLE_PATCHES_INPUT_OUTPUT
static const wxString CMD_PATCHES_INPUT                             = "pi";
static const wxString CMD_PATCHES_OUTPUT                            = "po";
#endif // ENABLE_PATCHES_INPUT_OUTPUT

// Lets our wxCmdLineEntryDesc descriptions reference the char* buffer of an
// inline constructed wxString, without worrying about the scope of that
// wxString. Also provides some pre and post formatting for consistency.
class Desc
{
public:
	Desc(const wxString& desc)
	{
		wxASSERT(m_staticStringsSize < MAX);
		m_index = m_staticStringsSize++;
		m_staticStrings[m_index] = wxString::Format("\n%s%s\n", Indent(), desc.c_str());
	}

	operator const char*() const
	{
		return m_staticStrings[m_index].c_str();
	}

	inline static const char* Indent() { return "   "; }

private:
	static const int MAX = 256;
	static wxString m_staticStrings[MAX];
	static int m_staticStringsSize;
	int m_index;
};
wxString Desc::m_staticStrings[MAX];
int Desc::m_staticStringsSize;

//
// CommandLineOptions::ValueFinder and partial specializations.
//
template<typename T>
void CommandLineOptions::ValueFinder<T>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wasFound = parser.Found(name, &value);
}

template<>
void CommandLineOptions::ValueFinder<CommandLineOptions::LowResolutionPassesMax>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
	{
		if (stringValue.CmpNoCase(SettingsUi::GetLowResolutionPassesAutoDescription()) == 0)
		{
			this->wasFound = true;
			value = PriorityBp::Settings::LOW_RESOLUTION_PASSES_AUTO;
		}
		else if (stringValue.IsNumber())
		{
			long longValue = 0;
			stringValue.ToLong(&longValue);

			this->wasFound = true;
			this->value = longValue;
		}
	}
}

template<>
void CommandLineOptions::ValueFinder<PriorityBp::CompositorPatchType>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
	{
		for (int e = PriorityBp::CompositorPatchTypeInvalid + 1; e < PriorityBp::CompositorPatchTypeNum; ++e)
		{
			const PriorityBp::CompositorPatchType compositorPatchType = PriorityBp::CompositorPatchType(e);
			const wxString desc(SettingsUi::GetEnumDescription(compositorPatchType));
			if (stringValue.CmpNoCase(desc) == 0)
			{
				this->wasFound = true;
				this->value = compositorPatchType;
				break;
			}
		}
	}
}

template<>
void CommandLineOptions::ValueFinder<PriorityBp::CompositorPatchBlender>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
	{
		for (int e = PriorityBp::CompositorPatchBlenderInvalid + 1; e < PriorityBp::CompositorPatchBlenderNum; ++e)
		{
			const PriorityBp::CompositorPatchBlender compositorPatchBlender = PriorityBp::CompositorPatchBlender(e);
			const wxString desc(SettingsUi::GetEnumDescription(compositorPatchBlender));
			if (stringValue.CmpNoCase(desc) == 0)
			{
				this->wasFound = true;
				this->value = compositorPatchBlender;
				break;
			}
		}
	}
}

//
// CommandLineOptions
//
CommandLineOptions::CommandLineOptions(int argc, char** argv)
	: m_isValid(false)
	, m_shouldShowSettings(false)
	, m_shouldRunImageCompletion(false)
	, m_debugLowResolutionPasses(false)
{
	// Create lists of PriorityBp::CompositorPatchType and
	// PriorityBp::CompositorPatchBlender options
	wxString compositorPatchTypeOptions;
	wxString compositorPatchBlenderOptions;
	{
		for (int e = PriorityBp::CompositorPatchTypeInvalid + 1; e < PriorityBp::CompositorPatchTypeNum; ++e)
		{
			if (!compositorPatchTypeOptions.IsEmpty())
			{
				compositorPatchTypeOptions += ", ";
			}
			compositorPatchTypeOptions += SettingsUi::GetEnumDescription(PriorityBp::CompositorPatchType(e));
		}

		for (int e = PriorityBp::CompositorPatchBlenderInvalid + 1; e < PriorityBp::CompositorPatchBlenderNum; ++e)
		{
			if (!compositorPatchBlenderOptions.IsEmpty())
			{
				compositorPatchBlenderOptions += ", ";
			}
			compositorPatchBlenderOptions += SettingsUi::GetEnumDescription(PriorityBp::CompositorPatchBlender(e));
		}
	}

	const wxCmdLineEntryDesc CMD_LINE_DESC[] =
	{
		{ wxCMD_LINE_OPTION, CMD_IMAGE_INPUT, "image-input", Desc("The input image file path."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
		{ wxCMD_LINE_OPTION, CMD_IMAGE_MASK, "image-mask", Desc("The mask image file path."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_IMAGE_OUTPUT, "image-output", Desc("The output image file path."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },

		{ wxCMD_LINE_SWITCH, CMD_SETTINGS_SHOW, "settings-show", Desc("Show the settings and exit.") },

		{ wxCMD_LINE_SWITCH, CMD_SETTINGS_DEBUG_LOW_RESOLUTION_PASSES, "settings-debug-low-res-passes", Desc("Output separate images for each low resolution pass.") },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_LOW_RESOLUTION_PASSES_MAX, "settings-low-res-passes", Desc(wxString("Max low resolution passes to perform.\n") + Desc::Indent() + "(" + SettingsUi::GetLowResolutionPassesAutoDescription() + ", or any integer value greater than 0)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_NUM_ITERATIONS, "settings-num-iterations", Desc("Number of Priority-BP iterations per pass."), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_LATTICE_GAP_WIDTH, "settings-lattice-width", Desc("Width of each gap in the lattice."), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_LATTICE_GAP_HEIGHT, "settings-lattice-height", Desc("Height of each gap in the lattice."), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_POST_PRUNE_PATCHES_MIN, "settings-patches-min", Desc("Min patches after pruning."), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_POST_PRUNE_PATCHES_MAX, "settings-patches-max", Desc("Max patches after pruning."), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_COMPOSITOR_PATCH_TYPE, "settings-compositor-patch-type", Desc(wxString("Compositor patch source type.\n") + Desc::Indent() + "(" + compositorPatchTypeOptions + ")"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_SETTINGS_COMPOSITOR_PATCH_BLENDER, "settings-compositor-patch-blender", Desc(wxString("Compositor patch blender style.\n") + Desc::Indent() + "(" + compositorPatchBlenderOptions + ")"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },

#if ENABLE_PATCHES_INPUT_OUTPUT
		{ wxCMD_LINE_OPTION, CMD_PATCHES_INPUT, "patches-input", Desc("The input patches file path."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
		{ wxCMD_LINE_OPTION, CMD_PATCHES_OUTPUT, "patches-output", Desc("The output patches file path."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
#endif // ENABLE_PATCHES_INPUT_OUTPUT

		{ wxCMD_LINE_NONE }
	};

	wxCmdLineParser parser(argc, argv);
	parser.SetLogo("\nlafarren.com\nImage Completion Using Efficient Belief Propagation\n");
	parser.SetSwitchChars("-");
	parser.SetDesc(CMD_LINE_DESC);
	if (parser.Parse() == 0)
	{
		parser.Found(CMD_IMAGE_INPUT, &m_inputImagePath);
		parser.Found(CMD_IMAGE_MASK, &m_maskImagePath);
		parser.Found(CMD_IMAGE_OUTPUT, &m_outputImagePath);

#if ENABLE_PATCHES_INPUT_OUTPUT
		parser.Found(CMD_PATCHES_INPUT, &m_inputPatchesPath);
		parser.Found(CMD_PATCHES_OUTPUT, &m_outputPatchesPath);
#endif // ENABLE_PATCHES_INPUT_OUTPUT

		m_shouldShowSettings = parser.Found(CMD_SETTINGS_SHOW);

		// As of now, don't run image completion if the user wanted the
		// settings displayed. Maybe change this later.
		m_shouldRunImageCompletion = !m_shouldShowSettings;

		m_isValid = true;

		// Invalidate if something is missing.
		{
			wxString errorMessage;

			// wxparser::Parse() should've failed if the input image
			// wasn't present, since it's set to wxCMD_LINE_OPTION_MANDATORY.
			wxASSERT(HasInputImagePath());

			if (m_shouldShowSettings)
			{
				// Nothing besides the input image is needed to display the settings.
			}

			if (m_shouldRunImageCompletion)
			{
				// Running image completion requires mask and output images.
				if (!HasMaskImagePath() && !HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask and output image paths. Please specify:\n\n\t-%s path/to/maskimage.ext -%s path/to/outputimage.ext\n", CMD_IMAGE_MASK.c_str(), CMD_IMAGE_OUTPUT.c_str());
					m_isValid = false;
				}
				else if (!HasMaskImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask image path. Please specify:\n\n\t-%s path/to/maskimage.ext\n", CMD_IMAGE_MASK.c_str());
					m_isValid = false;
				}
				else if (!HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing output image path. Please specify:\n\n\t-%s path/to/outputimage.ext\n", CMD_IMAGE_OUTPUT.c_str());
					m_isValid = false;
				}
			}

			if (!m_isValid)
			{
				parser.Usage();
				wxMessageOutput::Get()->Printf(errorMessage);
			}
			else
			{
				// These options are optional. If anything is invalid,
				// Priority::Settings::IsValid() will catch it later.
				m_debugLowResolutionPasses = parser.Found(CMD_SETTINGS_DEBUG_LOW_RESOLUTION_PASSES);
				m_lowResolutionPassesMax.Find(parser, CMD_SETTINGS_LOW_RESOLUTION_PASSES_MAX);
				m_numIterations.Find(parser, CMD_SETTINGS_NUM_ITERATIONS);
				m_latticeGapX.Find(parser, CMD_SETTINGS_LATTICE_GAP_WIDTH);
				m_latticeGapY.Find(parser, CMD_SETTINGS_LATTICE_GAP_HEIGHT);
				m_postPruneLabelsMin.Find(parser, CMD_SETTINGS_POST_PRUNE_PATCHES_MIN);
				m_postPruneLabelsMax.Find(parser, CMD_SETTINGS_POST_PRUNE_PATCHES_MAX);
				m_compositorPatchType.Find(parser, CMD_SETTINGS_COMPOSITOR_PATCH_TYPE);
				m_compositorPatchBlender.Find(parser, CMD_SETTINGS_COMPOSITOR_PATCH_BLENDER);
			}
		}
	}
}
