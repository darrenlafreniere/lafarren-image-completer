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

#include <vector>

#include <wx/cmdline.h>
#include "SettingsText.h"

#include "tech/DbgMem.h"

//
// Options
//

CommandLineOptions::Option::Option(wxString shortFlag, wxString longFlag, wxString description, bool hasDefault, int id, wxCmdLineParamType wxArgumentType, wxCmdLineEntryFlags wxMandatory = wxCMD_LINE_PARAM_OPTIONAL, wxString strValue = "") 
  : m_shortFlag(shortFlag), m_longFlag(longFlag), m_id(id), m_wxArgumentType(wxArgumentType), m_wxMandatory(wxMandatory), m_OptionType(COMPLETER_OPTION_TYPE)
{
  m_description = wxString::Format("\n%s%s\n", Indent(), description.c_str());
}



CommandLineOptions::Option* CommandLineOptions::FindOptionById(size_t id) const
{
  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    if(m_Options[i]->m_id == id)
    {
      return m_Options[i];
    }
  }
  std::cerr << "Option " << id << " not found!" << std::endl;
  return NULL;
}

CommandLineOptions::Option* CommandLineOptions::FindOptionByShortFlag(wxString shortFlag) const
{
  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    if(m_Options[i]->m_shortFlag == shortFlag)
    {
      return m_Options[i];
    }
  }
  std::cerr << "Option " << shortFlag << " not found!" << std::endl;
  return NULL;
}

CommandLineOptions::Option* CommandLineOptions::GetOption(unsigned int i) const
{
  return m_Options[i];
}

std::vector<CommandLineOptions::Option*> CommandLineOptions::GetOptionsByType(Option::OptionTypeEnum optionType) const
{
  std::vector<CommandLineOptions::Option*> options;

  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    if(m_Options[i]->m_OptionType == optionType)
    {
        options.push_back(m_Options[i]);
    }
  }
  return options;
}

//
// CommandLineOptions::ValueFinder and partial specializations.
//
template<typename T>
void CommandLineOptions::TypedOption<T>::Find(const wxCmdLineParser& parser)
{
	this->wasFound = parser.Found(m_shortFlag, &this->value);
}

template<>
void CommandLineOptions::TypedOption<std::string>::Find(const wxCmdLineParser& parser)
{
	wxString stringValue;
	this->wasFound = parser.Found(m_shortFlag, &stringValue);
	if (this->wasFound)
	{
		this->value = stringValue.c_str();
	}
}

template<>
//void CommandLineOptions::TypedOption<CommandLineOptions::LowResolutionPassesMax>::Find(const wxCmdLineParser& parser, const wxString& name)
void CommandLineOptions::TypedOption<int>::Find(const wxCmdLineParser& parser)
{
	wxString stringValue;
	if (parser.Found(m_shortFlag, &stringValue))
	{
		if (stringValue.CmpNoCase(SettingsText::GetLowResolutionPassesAutoDescription()) == 0)
		{
			this->wasFound = true;
			value = LfnIc::Settings::LOW_RESOLUTION_PASSES_AUTO;
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
void CommandLineOptions::TypedOption<LfnIc::CompositorPatchType>::Find(const wxCmdLineParser& parser)
{
	wxString stringValue;
	if (parser.Found(m_shortFlag, &stringValue))
	{
		for (int e = LfnIc::CompositorPatchTypeInvalid + 1; e < LfnIc::CompositorPatchTypeNum; ++e)
		{
			const LfnIc::CompositorPatchType compositorPatchType = LfnIc::CompositorPatchType(e);
			const wxString desc(SettingsText::GetEnumDescription(compositorPatchType));
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
void CommandLineOptions::TypedOption<LfnIc::CompositorPatchBlender>::Find(const wxCmdLineParser& parser)
{
	wxString stringValue;
	if (parser.Found(m_shortFlag, &stringValue))
	{
		for (int e = LfnIc::CompositorPatchBlenderInvalid + 1; e < LfnIc::CompositorPatchBlenderNum; ++e)
		{
			const LfnIc::CompositorPatchBlender compositorPatchBlender = LfnIc::CompositorPatchBlender(e);
			const wxString desc(SettingsText::GetEnumDescription(compositorPatchBlender));
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
	: m_shouldShowSettings(false)
	, m_shouldRunImageCompletion(false)
	, m_isValid(false)
{
    // Create options and set defaults.

    // General options
    m_optSettingsShow = TypedOption<bool>("ss", "settings-show", "Show the settings and exit.", true, -1, wxCMD_LINE_VAL_NONE);
    m_optSettingsShow.value = false;
    m_Options.push_back(&m_optSettingsShow);

    m_optDebugLowResolutionPasses = TypedOption<bool>("sd", "settings-debug-low-res-passes", "Output separate images for each low resolution pass.", true, -1, wxCMD_LINE_VAL_NONE);
    m_optDebugLowResolutionPasses.value = false;
    m_Options.push_back(&m_optDebugLowResolutionPasses);

    // Completer options with hard default values
    m_optImageInput = TypedOption<std::string>("ii", "image-input", "The input image file path.", false, -1, wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY);
    m_optImageInput.value = "";
    m_Options.push_back(&m_optImageInput);

    m_optImageMask = TypedOption<std::string>("im", "image-mask", "The mask image file path.", false, -1, wxCMD_LINE_VAL_STRING);
    m_optImageMask.value = "";
    m_Options.push_back(&m_optImageMask);

    m_optImageOutput = TypedOption<std::string>("io", "image-output", "The mask image file path.", false, -1, wxCMD_LINE_VAL_STRING);
    m_optImageOutput.value = "";
    m_Options.push_back(&m_optImageOutput);

    m_optLowResolutionPassesMax = TypedOption<int>("sp", "settings-low-res-passes", std::string("Max low resolution passes to perform.\n") + CommandLineOptions::Option::Indent() + std::string("(") + SettingsText::GetLowResolutionPassesAutoDescription() + std::string(", or any integer value greater than 0)"), true, offsetof(LfnIc::Settings, lowResolutionPassesMax), wxCMD_LINE_VAL_STRING);
    m_optLowResolutionPassesMax.value = 6; // what should this be?
    m_Options.push_back(&m_optLowResolutionPassesMax);

    m_optNumIterations = TypedOption<long>("si", "settings-num-iterations", "Number of Priority-BP iterations per pass.", true, offsetof(LfnIc::Settings, numIterations), wxCMD_LINE_VAL_NUMBER);
    m_optNumIterations.value = LfnIc::Settings::NUM_ITERATIONS_DEFAULT;
    m_Options.push_back(&m_optNumIterations);

    m_optLatticeWidth = TypedOption<long>("sw", "settings-lattice-width", "Width of each gap in the lattice.", false, offsetof(LfnIc::Settings, latticeGapX), wxCMD_LINE_VAL_NUMBER); // Should these be X/Y or Width/Height?
    m_optLatticeWidth.value = 5; // what should this be?
    m_Options.push_back(&m_optLatticeWidth);

    m_optLatticeHeight = TypedOption<long>("sh", "settings-lattice-height", "Height of each gap in the lattice.", false, offsetof(LfnIc::Settings, latticeGapY), wxCMD_LINE_VAL_NUMBER);
    m_optLatticeHeight.value = 5; // what should this be?
    m_Options.push_back(&m_optLatticeHeight);

    m_optPatchesMin = TypedOption<long>("smn", "settings-patches-min", "Min patches after pruning.", false, offsetof(LfnIc::Settings, postPruneLabelsMin), wxCMD_LINE_VAL_NUMBER); // These should be called Labels instead of Patches to match SettingsText.cpp
    m_optPatchesMin.value = 5; // what should this be?
    m_Options.push_back(&m_optPatchesMin);

    m_optPatchesMax = TypedOption<long>("smx", "settings-patches-max", "Max patches after pruning.", false, offsetof(LfnIc::Settings, postPruneLabelsMax), wxCMD_LINE_VAL_NUMBER);
    m_optPatchesMax.value = 5; // what should this be?
    m_Options.push_back(&m_optPatchesMax);

    #if ENABLE_PATCHES_INPUT_OUTPUT
    m_optPatchesInput = TypedOption<std::string>("pi", "patches-input", "The input patches file path.", false, -1, wxCMD_LINE_VAL_STRING);
    m_Options.push_back(&m_optPatchesInput);

    m_optPatchesOutput = TypedOption<std::string>("po", "patches-output", "The output patches file path.", false, -1, wxCMD_LINE_VAL_STRING);
    m_Options.push_back(&m_optPatchesOutput);
    #endif // ENABLE_PATCHES_INPUT_OUTPUT


    // Compositor options
    m_optCompositorPatchType = TypedOption<LfnIc::CompositorPatchType>("sct", "settings-compositor-patch-type", "Compositor patch source type.", false, -1, wxCMD_LINE_VAL_STRING);
    m_optCompositorPatchType.m_OptionType = CommandLineOptions::Option::COMPOSITOR_OPTION_TYPE;
    m_optCompositorPatchType.value = LfnIc::CompositorPatchTypeDefault;
    m_Options.push_back(&m_optCompositorPatchType);

    m_optCompositorPatchBlender = TypedOption<LfnIc::CompositorPatchBlender>("scb", "settings-compositor-patch-blender", "Compositor patch blender style.", false, -1, wxCMD_LINE_VAL_STRING);
    m_optCompositorPatchBlender.m_OptionType = CommandLineOptions::Option::COMPOSITOR_OPTION_TYPE;
    m_optCompositorPatchBlender.value = LfnIc::CompositorPatchBlenderDefault;
    m_Options.push_back(&m_optCompositorPatchBlender);

    // Completer options which depend on the input image (are there any?)

    // http://arnout.engelen.eu/~wxwindows/xmldocs/applications/docbook/output/html/x13114.html
    wxCmdLineParser parser(argc, argv);
    for(unsigned int i = 0; i < m_Options.size(); i++)
    {
      Option* option = m_Options[i];
      parser.AddOption(option->m_shortFlag, option->m_longFlag, option->m_description, option->m_wxArgumentType, option->m_wxMandatory);
    }



	parser.SetLogo("\nlafarren.com\nImage Completion Using Efficient Belief Propagation\n");
	parser.SetSwitchChars("-");

	if (parser.Parse() == 0)
	{
		m_optImageInput.Find(parser);
		m_optImageMask.Find(parser);
		m_optImageOutput.Find(parser);

#if ENABLE_PATCHES_INPUT_OUTPUT
		m_optPatchesInput.Find(parser);
		m_optPatchesOutput.Find(parser);
#endif // ENABLE_PATCHES_INPUT_OUTPUT

		m_shouldShowSettings = parser.Found(m_optSettingsShow.m_shortFlag);

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
					errorMessage = wxString::Format("\nMissing mask and output image paths. Please specify:\n\n\t-%s path/to/maskimage.ext -%s path/to/outputimage.ext\n", m_optImageMask.m_shortFlag, m_optImageOutput.m_shortFlag);
					m_isValid = false;
				}
				else if (!HasMaskImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask image path. Please specify:\n\n\t-%s path/to/maskimage.ext\n", m_optImageMask.m_shortFlag);
					m_isValid = false;
				}
				else if (!HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing output image path. Please specify:\n\n\t-%s path/to/outputimage.ext\n", m_optImageOutput.m_shortFlag);
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
				//m_debugLowResolutionPasses = parser.Found(m_optDebugLowResolutionPasses.m_shortFlag);
				m_optLowResolutionPassesMax.Find(parser);
				m_optNumIterations.Find(parser);
				m_optLatticeWidth.Find(parser);
				m_optLatticeHeight.Find(parser);
				m_optPatchesMin.Find(parser);
				m_optPatchesMax.Find(parser);
				m_optCompositorPatchType.Find(parser);
				m_optCompositorPatchBlender.Find(parser);

			}
		}
	}
}
