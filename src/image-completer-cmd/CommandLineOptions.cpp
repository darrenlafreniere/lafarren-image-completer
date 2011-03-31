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

Option& CommandLineOptions::FindOptionById(size_t id)
{
  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    if(m_Options[i].m_id == id)
    {
      return m_Options[i];
    }
  }
  std::cerr << "Option not found!" << std::endl;
}

Option& CommandLineOptions::FindOptionByShortFlag(wxString shortFlag)
{
  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    if(m_Options[i].m_shortFlag == shortFlag)
    {
      return m_Options[i];
    }
  }
  std::cerr << "Option not found!" << std::endl;
}

  


//
// CommandLineOptions::ValueFinder and partial specializations.
//
template<typename T>
void CommandLineOptions::ValueFinder<T>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	this->wasFound = parser.Found(name, &this->value);
}

template<>
void CommandLineOptions::ValueFinder<std::string>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	this->wasFound = parser.Found(name, &stringValue);
	if (this->wasFound)
	{
		this->value = stringValue.c_str();
	}
}

template<>
void CommandLineOptions::ValueFinder<CommandLineOptions::LowResolutionPassesMax>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
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
void CommandLineOptions::ValueFinder<LfnIc::CompositorPatchType>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
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
void CommandLineOptions::ValueFinder<LfnIc::CompositorPatchBlender>::Find(const wxCmdLineParser& parser, const wxString& name)
{
	wxString stringValue;
	if (parser.Found(name, &stringValue))
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
	, m_debugLowResolutionPasses(false)
	, m_isValid(false)
{

  Image_Input = Option("ii", "image-input", "The input image file path.", false, -1);
  m_Options.push_back(Image_Input);
  
  Image_Mask = Option("im", "image-mask", "The mask image file path.", false, -1);
  m_Options.push_back(Image_Mask);
  
  Image_Output = Option("im", "image-output", "The mask image file path.", false, -1);
  m_Options.push_back(Image_Output);

  Settings_Show = Option("ss", "settings-show", "Show the settings and exit.", true, -1);
  m_Options.push_back(Settings_Show);

  Debug_Low_Resolution_Passes = Option("sd", "settings-debug-low-res-passes", "Output separate images for each low resolution pass.", true, -1);
  m_Options.push_back(Debug_Low_Resolution_Passes);

  Low_Resolution_Passes_Max = Option("sp", "settings-low-res-passes", "Output separate images for each low resolution pass.", true, offsetof(LfnIc::Settings, lowResolutionPassesMax));
  //{ wxCMD_LINE_OPTION, CMD_SETTINGS_LOW_RESOLUTION_PASSES_MAX, "settings-low-res-passes", Desc(wxString("Max low resolution passes to perform.\n") + Desc::Indent() + "(" + SettingsText::GetLowResolutionPassesAutoDescription().c_str() + ", or any integer value greater than 0)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
  m_Options.push_back(Low_Resolution_Passes_Max);

  Num_Iterations = Option("si", "settings-num-iterations", "Number of Priority-BP iterations per pass.", true, offsetof(LfnIc::Settings, lowResolutionPassesMax));
  m_Options.push_back(Num_Iterations);
  
  Lattice_Width = Option("sw", "settings-lattice-width", "Width of each gap in the lattice.", false, offsetof(LfnIc::Settings, latticeGapX)); // Should these be X/Y or Width/Height?
  m_Options.push_back(Lattice_Width);

  Lattice_Height = Option("sh", "settings-lattice-height", "Height of each gap in the lattice.", false, offsetof(LfnIc::Settings, latticeGapY));
  m_Options.push_back(Lattice_Height);

  Patches_Min = Option("smn", "settings-patches-min", "Min patches after pruning.", false, offsetof(LfnIc::Settings, postPruneLabelsMin)); // These should be called Labels instead of Patches to match SettingsText.cpp
  m_Options.push_back(Patches_Min);
  
  Patches_Max = Option("smx", "settings-patches-max", "Max patches after pruning.", false, offsetof(LfnIc::Settings, postPruneLabelsMax));
  m_Options.push_back(Patches_Max);

  Compositor_Patch_Type = Option("sct", "settings-compositor-patch-type", "Compositor patch source type.", false, -1);
  m_Options.push_back(Compositor_Patch_Type);
  
  Compositor_Patch_Blender = Option("scb", "settings-compositor-patch-blender", "Compositor patch blender style.", false, -1);
  m_Options.push_back(Compositor_Patch_Blender);

  #if ENABLE_PATCHES_INPUT_OUTPUT
  Patches_Input = Option("pi", "patches-input", "The input patches file path.", false, -1);
  m_Options.push_back(Patches_Input);
  
  Patches_Output = Option("po", "patches-output", "The output patches file path.", false, -1);
  m_Options.push_back(Patches_Output);
  #endif // ENABLE_PATCHES_INPUT_OUTPUT

    
  wxCmdLineEntryDesc CMD_LINE_DESC[m_Options.size()];

  for(unsigned int i = 0; i < m_Options.size(); i++)
  {
    Option option = m_Options[i];
    wxCmdLineEntryDesc entry = { wxCMD_LINE_OPTION, option.m_shortFlag, option.m_longFlag, option.m_description, wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY };
    CMD_LINE_DESC[i] = entry;
  }

	wxCmdLineParser parser(argc, argv);
	parser.SetLogo("\nlafarren.com\nImage Completion Using Efficient Belief Propagation\n");
	parser.SetSwitchChars("-");
	parser.SetDesc(CMD_LINE_DESC);
	if (parser.Parse() == 0)
	{
		m_inputImagePath.Find(parser, Image_Input.m_shortFlag);
		m_maskImagePath.Find(parser, Image_Mask.m_shortFlag);
		m_outputImagePath.Find(parser, Image_Output.m_shortFlag);

#if ENABLE_PATCHES_INPUT_OUTPUT
		m_inputPatchesPath.Find(parser, Patches_Input.m_shortFlag);
		m_outputPatchesPath.Find(parser, Patches_Output.m_shortFlag);
#endif // ENABLE_PATCHES_INPUT_OUTPUT

		m_shouldShowSettings = parser.Found(Settings_Show.m_shortFlag);

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
					errorMessage = wxString::Format("\nMissing mask and output image paths. Please specify:\n\n\t-%s path/to/maskimage.ext -%s path/to/outputimage.ext\n", Image_Mask.m_shortFlag, Image_Output.m_shortFlag);
					m_isValid = false;
				}
				else if (!HasMaskImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask image path. Please specify:\n\n\t-%s path/to/maskimage.ext\n", Image_Mask.m_shortFlag);
					m_isValid = false;
				}
				else if (!HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing output image path. Please specify:\n\n\t-%s path/to/outputimage.ext\n", Image_Output.m_shortFlag);
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
				m_debugLowResolutionPasses = parser.Found(Debug_Low_Resolution_Passes.m_shortFlag);
				m_lowResolutionPassesMax.Find(parser, Low_Resolution_Passes_Max.m_shortFlag);
				m_numIterations.Find(parser, Num_Iterations.m_shortFlag);
				m_latticeGapX.Find(parser, Lattice_Width.m_shortFlag);
				m_latticeGapY.Find(parser, Lattice_Height.m_shortFlag);
				m_postPruneLabelsMin.Find(parser, Patches_Min.m_shortFlag);
				m_postPruneLabelsMax.Find(parser, Patches_Max.m_shortFlag);
				m_compositorPatchType.Find(parser, Compositor_Patch_Type.m_shortFlag);
				m_compositorPatchBlender.Find(parser, Compositor_Patch_Blender.m_shortFlag);

			}
		}
	}
}
