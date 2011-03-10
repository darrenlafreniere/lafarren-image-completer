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

#include "tech/StrUtils.h"
#include "SettingsUi.h"

#include "tech/DbgMem.h"

namespace Tech
{
	// CmdLine::ParamOption<CommandLineOptions::LowResolutionPassesMax> partial specialization.
	template<>
	void CmdLine::ParamOption<CommandLineOptions::LowResolutionPassesMax>::ReadOption(const char* option)
	{
		if (std::string(option).compare(SettingsUi::GetLowResolutionPassesAutoDescription().c_str()) == 0)
		{
			this->value = PriorityBp::Settings::LOW_RESOLUTION_PASSES_AUTO;
		}
		else
		{
			this->value = atoi(option);
		}
	}

	// CmdLine::ParamOption<PriorityBp::CompositorPatchType> partial specialization.
	template<>
	void CmdLine::ParamOption<PriorityBp::CompositorPatchType>::ReadOption(const char* option)
	{
		for (int e = PriorityBp::CompositorPatchTypeInvalid + 1; e < PriorityBp::CompositorPatchTypeNum; ++e)
		{
			const PriorityBp::CompositorPatchType compositorPatchType = PriorityBp::CompositorPatchType(e);
			const std::string desc(SettingsUi::GetEnumDescription(compositorPatchType));
			if (std::string(option).compare(desc.c_str()) == 0)
			{
				this->value = compositorPatchType;
				break;
			}
		}
	}

	// CmdLine::ParamOption<PriorityBp::CompositorPatchBlender> partial specialization.
	template<>
	void CmdLine::ParamOption<PriorityBp::CompositorPatchBlender>::ReadOption(const char* option)
	{
		for (int e = PriorityBp::CompositorPatchBlenderInvalid + 1; e < PriorityBp::CompositorPatchBlenderNum; ++e)
		{
			const PriorityBp::CompositorPatchBlender compositorPatchBlender = PriorityBp::CompositorPatchBlender(e);
			const std::string desc(SettingsUi::GetEnumDescription(compositorPatchBlender));
			if (std::string(option).compare(desc.c_str()) == 0)
			{
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
	: m_shouldDisplayUsage("-h", "--help", "Display this help text.")
	, m_inputImagePath("-ii", "--image-input", "The input image file path.")
	, m_maskImagePath("-im", "--image-mask", "The mask image file path.")
	, m_outputImagePath("-io", "--image-output", "The output image file path.")
	, m_shouldShowSettings("-ss", "--settings-show", "Show the settings for the input image and exit.")
	, m_debugLowResolutionPasses("-sd", "--settings-debug-low-res-passes", "Output separate images for each low resolution pass.")
	, m_lowResolutionPassesMax("-sp", "--settings-low-res-passes", std::string("Max low resolution passes to perform.\n\t(") + SettingsUi::GetLowResolutionPassesAutoDescription() + ", or any integer value greater than 0)")
	, m_numIterations("-si", "--settings-num-iterations", "Number of Priority-BP iterations per pass.")
	, m_latticeGapX("-sw", "--settings-lattice-width", "Width of each gap in the lattice.")
	, m_latticeGapY("-sh", "--settings-lattice-height", "Height of each gap in the lattice.")
	, m_postPruneLabelsMin("-smn", "--settings-patches-min", "Min patches after pruning.")
	, m_postPruneLabelsMax("-smx", "--settings-patches-max", "Max patches after pruning.")
	, m_compositorPatchType("-sct", "--settings-compositor-patch-type", std::string("Compositor patch source type.\n\t(") + SettingsUi::JoinEnumDescriptions<PriorityBp::CompositorPatchType>() + ")")
	, m_compositorPatchBlender("-scb", "--settings-compositor-patch-blender", std::string("Compositor patch blender style.\n\t(") + SettingsUi::JoinEnumDescriptions<PriorityBp::CompositorPatchBlender>() + ")")
#if ENABLE_PATCHES_INPUT_OUTPUT
	, m_inputPatchesPath("-pi", "--patches-input", "The input patches file path.")
	, m_outputPatchesPath("-po", "--patches-output", "The output patches file path.")
#endif // ENABLE_PATCHES_INPUT_OUTPUT
	, m_shouldRunImageCompletion(false)
	, m_isValid(false)
{
	Tech::CmdLine cmdLine;
	cmdLine.AddParam(m_shouldDisplayUsage);
	cmdLine.AddParam(m_inputImagePath);
	cmdLine.AddParam(m_maskImagePath);
	cmdLine.AddParam(m_outputImagePath);
	cmdLine.AddParam(m_shouldShowSettings);
	cmdLine.AddParam(m_debugLowResolutionPasses);
	cmdLine.AddParam(m_lowResolutionPassesMax);
	cmdLine.AddParam(m_numIterations);
	cmdLine.AddParam(m_latticeGapX);
	cmdLine.AddParam(m_latticeGapY);
	cmdLine.AddParam(m_postPruneLabelsMin);
	cmdLine.AddParam(m_postPruneLabelsMax);
	cmdLine.AddParam(m_compositorPatchType);
	cmdLine.AddParam(m_compositorPatchBlender);
#if ENABLE_PATCHES_INPUT_OUTPUT
	cmdLine.AddParam(m_inputPatchesPath);
	cmdLine.AddParam(m_outputPatchesPath);
#endif // ENABLE_PATCHES_INPUT_OUTPUT

	std::string error;
	m_isValid = cmdLine.Read(argc, argv, error);

	// If the args were valid and the user isn't asking for the help text,
	// validate the mandatory options.
	if (m_isValid && !m_shouldDisplayUsage.isSet)
	{
		if (!m_inputImagePath.isSet)
		{
			error += Tech::Str::Format("Missing input image path.\n");
			m_isValid = false;
		}

		// If the user isn't asking to display the auto-settings for the input
		// image, validate the other mandatory options.
		if (!m_shouldShowSettings.isSet)
		{
			if (!m_maskImagePath.isSet)
			{
				error += Tech::Str::Format("Missing mask image path.\n");
				m_isValid = false;
			}
			if (!m_outputImagePath.isSet)
			{
				error += Tech::Str::Format("Missing output image path.\n");
				m_isValid = false;
			}

			m_shouldRunImageCompletion = m_isValid;
		}
	}

	if (!m_isValid)
	{
		printf("\n");
		printf(error.c_str());
		m_shouldDisplayUsage.isSet = true;
	}

	if (m_shouldDisplayUsage.isSet)
	{
		cmdLine.PrintfUsage();

		// If the usage is printed for whatever reason, don't allow the process to continue.
		m_isValid = false;
	}
}
