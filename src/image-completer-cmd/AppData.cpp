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
#include "AppData.h"

#include "CommandLineOptions.h"
#include "SettingsText.h"

#include "tech/DbgMem.h"

// Trues true if the images are valid for image completion. Otherwise, logs an
// error and returns false.
static bool LoadAndValidateImage(const char* imageTypeName, const std::string& imagePath, AppWxImage& image)
{
	bool result = false;
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	if (!image.GetwxImage().LoadFile(imagePath))
	{
		// If LoadFile fails, it already prints an wxMessageOutput error for us.
	}
	else if (!image.IsValid())
	{
		msgOut.Printf("The %s image was invalid.\n", imageTypeName);
	}
	else if (image.GetWidth() > LfnIc::Settings::IMAGE_WIDTH_MAX || image.GetHeight() > LfnIc::Settings::IMAGE_HEIGHT_MAX)
	{
		msgOut.Printf("The %s image is too large. Max size: %dx%x.\n", imageTypeName, LfnIc::Settings::IMAGE_WIDTH_MAX, LfnIc::Settings::IMAGE_HEIGHT_MAX);
	}
	else
	{
		result = true;
	}

	return result;
}

AppCmdHost::AppCmdHost(const CommandLineOptions& options)
	: m_isValid(false)
{
	if (LoadAndValidateImage("input", options.GetInputImagePath(), m_inputImage))
	{
		LfnIc::SettingsConstruct(m_settings, m_inputImage);

		ApplyCommandLineOptionsToSettings(options);
		SettingsText::PrintInvalidMembers settingsTextPrintInvalidMembers;
		if (LfnIc::AreSettingsValid(m_settings, &settingsTextPrintInvalidMembers))
		{
			m_isValid = true;

			if (options.ShouldShowSettings())
			{
				// Nothing more to construct for simply displaying the settings.
			}

			if (options.ShouldRunImageCompletion())
			{
				if (!LoadAndValidateImage("mask", options.GetMaskImagePath(), m_maskImage))
				{
					m_isValid = false;
				}
				else
				{
					m_inputImage.SetFilePath(options.GetInputImagePath());
					m_maskImage.SetFilePath(options.GetMaskImagePath());
					m_outputImage.SetFilePath(options.GetOutputImagePath());

#if ENABLE_PATCHES_INPUT_OUTPUT
					if (options.HasInputPatchesPath())
					{
						m_patchesIstream.reset(new std::ifstream(options.GetInputPatchesPath().c_str(), std::ios::binary));
					}

					if (options.HasOutputPatchesPath())
					{
						m_patchesOstream.reset(new std::ofstream());
						m_patchesOstream->open(options.GetOutputPatchesPath().c_str(), std::ios::binary);
					}
#endif // ENABLE_PATCHES_INPUT_OUTPUT
				}
			}
		}
	}
}

bool AppCmdHost::IsValid() const
{
	return m_isValid;
}

const AppWxImage& AppCmdHost::GetInputImageImpl()
{
	return m_inputImage;
}

const AppWxImage& AppCmdHost::GetMaskImageImpl()
{
	return m_maskImage;
}

AppWxImage& AppCmdHost::GetOutputImageImpl()
{
	return m_outputImage;
}

const LfnIc::Settings& AppCmdHost::GetSettings()
{
	return m_settings;
}

const LfnIc::Image& AppCmdHost::GetInputImage()
{
	return m_inputImage;
}

const LfnIc::Image& AppCmdHost::GetMaskImage()
{
	return m_maskImage;
}

LfnIc::Image& AppCmdHost::GetOutputImage()
{
	return m_outputImage;
}

const LfnIc::Image& AppCmdHost::GetOutputImage() const
{
	return m_outputImage;
}

std::istream* AppCmdHost::GetPatchesIstream()
{
	return m_patchesIstream.get();
}

std::ostream* AppCmdHost::GetPatchesOstream()
{
	return m_patchesOstream.get();
}

void AppCmdHost::ApplyCommandLineOptionsToSettings(const CommandLineOptions& options)
{
	if (options.DebugLowResolutionPasses())
	{
		m_settings.debugLowResolutionPasses = true;
	}
	if (options.HasLowResolutionPassesMax())
	{
		m_settings.lowResolutionPassesMax = options.GetLowResolutionPassesMax();
	}
	if (options.HasNumIterations())
	{
		m_settings.numIterations = options.GetNumIterations();
	}
	if (options.HasLatticeGapX())
	{
		m_settings.latticeGapX = options.GetLatticeGapX();
	}
	if (options.HasLatticeGapY())
	{
		m_settings.latticeGapY = options.GetLatticeGapY();
	}
	if (options.HasPostPruneLabelsMin())
	{
		m_settings.postPruneLabelsMin = options.GetPostPruneLabelsMin();
	}
	if (options.HasPostPruneLabelsMax())
	{
		m_settings.postPruneLabelsMax = options.GetPostPruneLabelsMax();
	}
	if (options.HasCompositorPatchType())
	{
		m_settings.compositorPatchType = options.GetCompositorPatchType();
	}
	if (options.HasCompositorPatchBlender())
	{
		m_settings.compositorPatchBlender = options.GetCompositorPatchBlender();
	}
}
