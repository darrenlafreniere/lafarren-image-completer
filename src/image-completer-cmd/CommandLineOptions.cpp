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
#include "tech/StrUtils.h"

#include "tech/DbgMem.h"

//
// CommandLineOptions::Option
//
CommandLineOptions::Option::Option(
	OptionType optionType,
	const wxString& shortName,
	const wxString& longName,
	const wxString& description,
	int id,
	wxCmdLineParamType argumentType,
	wxCmdLineEntryFlags flags,
	const wxString& strValue)
	: optionType(optionType)
	, shortName(shortName)
	, longName(longName)
	, description(description)
	, id(id)
	, argumentType(argumentType)
	, flags(flags)
	, wasFound(false)
{
}

//
// CommandLineOptions::TypedOption and partial specializations.
//
template<typename T>
CommandLineOptions::TypedOption<T>::TypedOption(
	const T& value,
	OptionType optionType,
	const wxString& shortName,
	const wxString& longName,
	const wxString& description,
	int id,
	wxCmdLineParamType argumentType,
	wxCmdLineEntryFlags flags,
	const wxString& strValue)
	: Option(optionType, shortName, longName, description, id, argumentType, flags, strValue)
	, value(value)
{
}

template<typename T>
void CommandLineOptions::TypedOption<T>::Find(const wxCmdLineParser& parser)
{
	this->wasFound = parser.Found(this->shortName, &this->value);
}

template<>
void CommandLineOptions::TypedOption<bool>::Find(const wxCmdLineParser& parser)
{
	this->wasFound = parser.Found(this->shortName);
	this->value = this->wasFound;
}

template<>
void CommandLineOptions::TypedOption<std::string>::Find(const wxCmdLineParser& parser)
{
	wxString stringValue;
	this->wasFound = parser.Found(this->shortName, &stringValue);
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
	if (parser.Found(this->shortName, &stringValue))
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
	if (parser.Found(this->shortName, &stringValue))
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
	if (parser.Found(this->shortName, &stringValue))
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
	: m_optImageInput("", Option::COMPLETER_OPTION_TYPE, "ii", "image-input", "The input image file path.", -1, wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY)
	, m_optImageMask("", Option::COMPLETER_OPTION_TYPE, "im", "image-mask", "The mask image file path.", -1, wxCMD_LINE_VAL_STRING)
	, m_optImageOutput("", Option::COMPLETER_OPTION_TYPE, "io", "image-output", "The mask image file path.", -1, wxCMD_LINE_VAL_STRING)
	, m_optSettingsShow(false, Option::COMPLETER_OPTION_TYPE, "ss", "settings-show", "Show the settings and exit.", -1, wxCMD_LINE_VAL_NONE)
	, m_optDebugLowResolutionPasses(false, Option::COMPLETER_OPTION_TYPE, "sd", "settings-debug-low-res-passes", "Output separate images for each low resolution pass.", -1, wxCMD_LINE_VAL_NONE)
	, m_optLowResolutionPassesMax(0, Option::COMPLETER_OPTION_TYPE, "sp", "settings-low-res-passes", std::string("Max low resolution passes to perform.\n") + Option::Indent() + std::string("(") + SettingsText::GetLowResolutionPassesAutoDescription() + std::string(", or any integer value greater than 0)"), offsetof(LfnIc::Settings, lowResolutionPassesMax), wxCMD_LINE_VAL_STRING)
	, m_optNumIterations(LfnIc::Settings::NUM_ITERATIONS_DEFAULT, Option::COMPLETER_OPTION_TYPE, "si", "settings-num-iterations", "Number of Priority-BP iterations per pass.", offsetof(LfnIc::Settings, numIterations), wxCMD_LINE_VAL_NUMBER)
	, m_optLatticeWidth(0, Option::COMPLETER_OPTION_TYPE, "sw", "settings-lattice-width", "Width of each gap in the lattice.", offsetof(LfnIc::Settings, latticeGapX), wxCMD_LINE_VAL_NUMBER)
	, m_optLatticeHeight(0, Option::COMPLETER_OPTION_TYPE, "sh", "settings-lattice-height", "Height of each gap in the lattice.", offsetof(LfnIc::Settings, latticeGapY), wxCMD_LINE_VAL_NUMBER)
	, m_optPatchesMin(0, Option::COMPLETER_OPTION_TYPE, "smn", "settings-patches-min", "Min patches after pruning.", offsetof(LfnIc::Settings, postPruneLabelsMin), wxCMD_LINE_VAL_NUMBER) // These should be called Labels instead of Patches to match SettingsText.cpp
	, m_optPatchesMax(0, Option::COMPLETER_OPTION_TYPE, "smx", "settings-patches-max", "Max patches after pruning.", offsetof(LfnIc::Settings, postPruneLabelsMax), wxCMD_LINE_VAL_NUMBER)
	, m_optCompositorPatchType(LfnIc::CompositorPatchTypeDefault, Option::COMPOSITOR_OPTION_TYPE, "sct", "settings-compositor-patch-type", "Compositor patch source type.", -1, wxCMD_LINE_VAL_STRING)
	, m_optCompositorPatchBlender(LfnIc::CompositorPatchBlenderDefault, Option::COMPOSITOR_OPTION_TYPE, "scb", "settings-compositor-patch-blender", "Compositor patch blender style.", -1, wxCMD_LINE_VAL_STRING)
#if ENABLE_PATCHES_INPUT_OUTPUT
	, m_optPatchesInput("", Option::COMPLETER_OPTION_TYPE, "pi", "patches-input", "The input patches file path.", -1, wxCMD_LINE_VAL_STRING)
	, m_optPatchesOutput("", Option::COMPLETER_OPTION_TYPE, "po", "patches-output", "The output patches file path.", -1, wxCMD_LINE_VAL_STRING)
#endif
	, m_shouldRunImageCompletion(false)
	, m_isValid(false)
{
	m_options.push_back(&m_optImageInput);
	m_options.push_back(&m_optImageMask);
	m_options.push_back(&m_optImageOutput);
	m_options.push_back(&m_optSettingsShow);
	m_options.push_back(&m_optDebugLowResolutionPasses);
	m_options.push_back(&m_optLowResolutionPassesMax);
	m_options.push_back(&m_optNumIterations);
	m_options.push_back(&m_optLatticeWidth);
	m_options.push_back(&m_optLatticeHeight);
	m_options.push_back(&m_optPatchesMin);
	m_options.push_back(&m_optPatchesMax);
#if ENABLE_PATCHES_INPUT_OUTPUT
	m_options.push_back(&m_optPatchesInput);
	m_options.push_back(&m_optPatchesOutput);
#endif
	m_options.push_back(&m_optCompositorPatchType);
	m_options.push_back(&m_optCompositorPatchBlender);

	// Completer options which depend on the input image
	// http://arnout.engelen.eu/~wxwindows/xmldocs/applications/docbook/output/html/x13114.html
	wxCmdLineParser parser(argc, argv);
	for (int i = 0, n = m_options.size(); i < n; ++i)
	{
		const Option& option = *m_options[i];
		const std::string& descriptionForUsageText = LfnTech::Str::Format("\n%s%s\n", Option::Indent(), static_cast<const char*>(option.description));
		if (option.argumentType == wxCMD_LINE_VAL_NONE)
		{
			parser.AddSwitch(option.shortName, option.longName, descriptionForUsageText, option.flags);
		}
		else
		{
			parser.AddOption(option.shortName, option.longName, descriptionForUsageText, option.argumentType, option.flags);
		}
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

		m_optSettingsShow.Find(parser);

		// As of now, don't run image completion if the user wanted the
		// settings displayed. Maybe change this later.
		m_shouldRunImageCompletion = !m_optSettingsShow.value;

		m_isValid = true;

		// Invalidate if something is missing.
		{
			wxString errorMessage;

			// wxparser::Parse() should've failed if the input image
			// wasn't present, since it's set to wxCMD_LINE_OPTION_MANDATORY.
			wxASSERT(HasInputImagePath());

			if (m_optSettingsShow.value)
			{
				// Nothing besides the input image is needed to display the settings.
			}

			if (m_shouldRunImageCompletion)
			{
				// Running image completion requires mask and output images.
				if (!HasMaskImagePath() && !HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask and output image paths. Please specify:\n\n\t-%s path/to/maskimage.ext -%s path/to/outputimage.ext\n", m_optImageMask.shortName, m_optImageOutput.shortName);
					m_isValid = false;
				}
				else if (!HasMaskImagePath())
				{
					errorMessage = wxString::Format("\nMissing mask image path. Please specify:\n\n\t-%s path/to/maskimage.ext\n", m_optImageMask.shortName);
					m_isValid = false;
				}
				else if (!HasOutputImagePath())
				{
					errorMessage = wxString::Format("\nMissing output image path. Please specify:\n\n\t-%s path/to/outputimage.ext\n", m_optImageOutput.shortName);
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
				m_optDebugLowResolutionPasses.Find(parser);
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

const CommandLineOptions::Option* CommandLineOptions::FindOptionById(size_t id) const
{
	for (int i = 0, n = m_options.size(); i < n; ++i)
	{
		if (m_options[i]->id == id)
		{
			return m_options[i];
		}
	}

	std::cerr << "Option " << id << " not found!" << std::endl;
	return NULL;
}

const CommandLineOptions::Option* CommandLineOptions::FindOptionByShortName(const wxString& shortName) const
{
	for (int i = 0, n = m_options.size(); i < n; ++i)
	{
		if (m_options[i]->shortName == shortName)
		{
			return m_options[i];
		}
	}

	std::cerr << "Option " << shortName << " not found!" << std::endl;
	return NULL;
}

const CommandLineOptions::Option* CommandLineOptions::GetOption(unsigned int i) const
{
	return m_options[i];
}

std::vector<const CommandLineOptions::Option*> CommandLineOptions::GetOptionsByType(Option::OptionType optionType) const
{
	std::vector<const Option*> options;

	for (int i = 0, n = m_options.size(); i < n; ++i)
	{
		if (m_options[i]->optionType == optionType)
		{
			options.push_back(m_options[i]);
		}
	}

	return options;
}

void CommandLineOptions::PrintSettingsThatHaveCommandLineOptions(const LfnIc::Settings& settings) const
{
	wxASSERT(LfnIc::AreSettingsValid(settings));
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	std::string lowResolutionPassesMaxString;
	if (settings.lowResolutionPassesMax == LfnIc::Settings::LOW_RESOLUTION_PASSES_AUTO)
	{
		lowResolutionPassesMaxString = SettingsText::GetLowResolutionPassesAutoDescription();
	}
	else
	{
		lowResolutionPassesMaxString = LfnTech::Str::Format("%d", settings.lowResolutionPassesMax);
	}

#define VAL_W "17" /* TODO: auto-size this as well? */
#define VAL_X(fmt, x) LfnTech::Str::Format("%" VAL_W fmt, x)
#define VAL_S(s) VAL_X("s", s)
#define VAL_I(i) VAL_X("d", i)

	typedef std::map<const Option*, std::string> OptionStrValueMap;
	OptionStrValueMap optionStrValues;
	optionStrValues[&m_optLowResolutionPassesMax] = VAL_S(lowResolutionPassesMaxString.c_str());
	optionStrValues[&m_optNumIterations] = VAL_I(settings.numIterations);
	optionStrValues[&m_optLatticeWidth] = VAL_I(settings.latticeGapX);
	optionStrValues[&m_optLatticeHeight] = VAL_I(settings.latticeGapY);
	optionStrValues[&m_optPatchesMin] = VAL_I(settings.postPruneLabelsMin);
	optionStrValues[&m_optPatchesMax] = VAL_I(settings.postPruneLabelsMax);

	optionStrValues[&m_optCompositorPatchType] = VAL_S(SettingsText::GetEnumDescription(settings.compositorPatchType).c_str());
	optionStrValues[&m_optCompositorPatchBlender] = VAL_S(SettingsText::GetEnumDescription(settings.compositorPatchBlender).c_str());

	// Determine the name column width.
	int nameWidth = 0;
	for (int optionIndex = 0; optionIndex < GetNumberOfOptions(); ++optionIndex)
	{
		const int len = GetOption(optionIndex)->longName.Len();
		if (nameWidth < len)
		{
			nameWidth = len;
		}
	}

	// Display the members
	const std::vector<const Option*>& completerOptions = GetOptionsByType(Option::COMPLETER_OPTION_TYPE);
	msgOut.Printf("\nImage completer settings\n");
	for (int i = 0, n = completerOptions.size(); i < n; ++i)
	{
		const Option& option = *completerOptions[i];
		const OptionStrValueMap::const_iterator iter = optionStrValues.find(&option);
		if (iter != optionStrValues.end())
		{
			option.Print(msgOut, nameWidth, iter->second);
		}
	}

	const std::vector<const Option*>& compositorOptions = GetOptionsByType(Option::COMPOSITOR_OPTION_TYPE);
	msgOut.Printf("\nCompositor settings\n");
	for (int i = 0, n = compositorOptions.size(); i < n; ++i)
	{
		const Option& option = *compositorOptions[i];
		const OptionStrValueMap::const_iterator iter = optionStrValues.find(&option);
		if (iter != optionStrValues.end())
		{
			option.Print(msgOut, nameWidth, iter->second);
		}
	}

	msgOut.Printf("\n");
}
