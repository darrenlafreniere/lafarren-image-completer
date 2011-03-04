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

#ifndef COMMAND_LINE_OPTIONS_H
#define COMMAND_LINE_OPTIONS_H

#include "tech/CmdLine.h"
#include "PriorityBpTypes.h"

// Reading/writing a patches file is a development-only thing when doing
// compositor work. Enable this, and use -po patches.filename on the command
// line, and the next image completion will write its solved patches data to
// patches.filename. On the next run, use -pi patches.filename to skip the
// expensive process of image completion, and read the previously written
// patches data to send to the compositor.
#define ENABLE_PATCHES_INPUT_OUTPUT 1

// Pulls the command line options from the parser, validates them, and
// presents a strongly typed interface to their values.
class CommandLineOptions
{
public:
	CommandLineOptions(int argc, char** argv);

	inline bool IsValid() const { return m_isValid; }

	inline bool HasInputImagePath() const { return m_inputImagePath.isSet; }
	inline bool HasMaskImagePath() const { return m_maskImagePath.isSet; }
	inline bool HasOutputImagePath() const { return m_outputImagePath.isSet; }

	inline const std::string& GetInputImagePath() const { return m_inputImagePath.value; }
	inline const std::string& GetMaskImagePath() const { return m_maskImagePath.value; }
	inline const std::string& GetOutputImagePath() const { return m_outputImagePath.value; }

#if ENABLE_PATCHES_INPUT_OUTPUT
	inline bool HasInputPatchesPath() const { return m_inputPatchesPath.isSet; }
	inline bool HasOutputPatchesPath() const { return m_outputPatchesPath.isSet; }

	inline const std::string& GetInputPatchesPath() const { return m_inputPatchesPath.value; }
	inline const std::string& GetOutputPatchesPath() const { return m_outputPatchesPath.value; }
#endif // ENABLE_PATCHES_INPUT_OUTPUT

	inline bool ShouldShowSettings() const { return m_shouldShowSettings.isSet; }
	inline bool ShouldRunImageCompletion() const { return m_shouldRunImageCompletion; }

	inline bool DebugLowResolutionPasses() const { return m_debugLowResolutionPasses.isSet; }

	inline bool HasLowResolutionPassesMax() const { return m_lowResolutionPassesMax.isSet; }
	inline int GetLowResolutionPassesMax() const { return m_lowResolutionPassesMax.value; }

	inline bool HasNumIterations() const { return m_numIterations.isSet; }
	inline int GetNumIterations() const { return m_numIterations.value; }

	inline bool HasLatticeGapX() const { return m_latticeGapX.isSet; }
	inline int GetLatticeGapX() const { return m_latticeGapX.value; }

	inline bool HasLatticeGapY() const { return m_latticeGapY.isSet; }
	inline int GetLatticeGapY() const { return m_latticeGapY.value; }

	inline bool HasPostPruneLabelsMin() const { return m_postPruneLabelsMin.isSet; }
	inline int GetPostPruneLabelsMin() const { return m_postPruneLabelsMin.value; }

	inline bool HasPostPruneLabelsMax() const { return m_postPruneLabelsMax.isSet; }
	inline int GetPostPruneLabelsMax() const { return m_postPruneLabelsMax.value; }

	inline bool HasCompositorPatchType() const { return m_compositorPatchType.isSet; }
	inline PriorityBp::CompositorPatchType GetCompositorPatchType() const { return m_compositorPatchType.value; }

	inline bool HasCompositorPatchBlender() const { return m_compositorPatchBlender.isSet; }
	inline PriorityBp::CompositorPatchBlender GetCompositorPatchBlender() const { return m_compositorPatchBlender.value; }

private:
	// Solely for specialization.
	struct LowResolutionPassesMax
	{
		int value;
		LowResolutionPassesMax& operator=(int value) { this->value = value; return *this; }
		operator int() const { return value; }
	};

	Lafarren::CmdLine::ParamSwitch m_shouldDisplayUsage;
	Lafarren::CmdLine::ParamOption<std::string> m_inputImagePath;
	Lafarren::CmdLine::ParamOption<std::string> m_maskImagePath;
	Lafarren::CmdLine::ParamOption<std::string> m_outputImagePath;
	Lafarren::CmdLine::ParamSwitch m_shouldShowSettings;
	Lafarren::CmdLine::ParamSwitch m_debugLowResolutionPasses;
	Lafarren::CmdLine::ParamOption<LowResolutionPassesMax> m_lowResolutionPassesMax;
	Lafarren::CmdLine::ParamOption<int> m_numIterations;
	Lafarren::CmdLine::ParamOption<int> m_latticeGapX;
	Lafarren::CmdLine::ParamOption<int> m_latticeGapY;
	Lafarren::CmdLine::ParamOption<int> m_postPruneLabelsMin;
	Lafarren::CmdLine::ParamOption<int> m_postPruneLabelsMax;
	Lafarren::CmdLine::ParamOption<PriorityBp::CompositorPatchType> m_compositorPatchType;
	Lafarren::CmdLine::ParamOption<PriorityBp::CompositorPatchBlender> m_compositorPatchBlender;
#if ENABLE_PATCHES_INPUT_OUTPUT
	Lafarren::CmdLine::ParamOption<std::string> m_inputPatchesPath;
	Lafarren::CmdLine::ParamOption<std::string> m_outputPatchesPath;
#endif // ENABLE_PATCHES_INPUT_OUTPUT

	bool m_shouldRunImageCompletion;
	bool m_isValid;
};

#endif // COMMAND_LINE_OPTIONS_H
