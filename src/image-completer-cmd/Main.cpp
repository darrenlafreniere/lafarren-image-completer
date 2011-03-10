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

#include "tech/Profile.h"

#include "CommandLineOptions.h"
#include "PriorityBp.h"
#include "PriorityBpHost.h"
#include "PriorityBpSettings.h"
#include "SettingsUi.h"

#include "tech/DbgMem.h"

//
// PriorityBp host interface implementations
//

// Make sure that wxWidgets wxImage::RGBValue and PriorityBp::HostImage::Rgb
// have identical size, since we perform no conversion.
wxCOMPILE_TIME_ASSERT(sizeof(wxImage::RGBValue) == sizeof(PriorityBp::HostImage::Rgb), INVALID_RGB_SIZE);
#ifdef __WXDEBUG__
class AssertIdenticalRgbLayout
{
public:
	AssertIdenticalRgbLayout()
	{
		wxImage::RGBValue rgb1(0, 0, 0);
		PriorityBp::HostImage::Rgb& rgb2 = reinterpret_cast<PriorityBp::HostImage::Rgb&>(rgb1);
		rgb2.red = 1;
		rgb2.green = 2;
		rgb2.blue = 3;
		wxASSERT(rgb1.red == 1);
		wxASSERT(rgb1.green == 2);
		wxASSERT(rgb1.blue == 3);
	}
};
static const AssertIdenticalRgbLayout g_assertIdenticalRgbLayout;
#endif

class AppCmdHostImage : public PriorityBp::HostImage
{
public:
	// AppCmdHostImage interface
	void SetFilePath(const std::string& filePath);
	wxImage& GetwxImage();
	const wxImage& GetwxImage() const;

	// PriorityBp::HostImage interface
	virtual bool Init(int width, int height);
	virtual bool IsValid() const;
	virtual const std::string& GetFilePath() const;
	virtual Rgb* GetRgb();
	virtual const Rgb* GetRgb() const;
	virtual int GetWidth() const;
	virtual int GetHeight() const;

private:
	// Internal data
	std::string m_filePath;
	wxImage m_wxImage;
};

class AppCmdHost : public PriorityBp::Host
{
public:
	// AppCmdHost interface
	AppCmdHost(const CommandLineOptions& options);
	bool IsValid() const;
	const AppCmdHostImage& GetInputImageImpl();
	const AppCmdHostImage& GetMaskImageImpl();
	AppCmdHostImage& GetOutputImageImpl();

	// PriorityBp::Host interface
	virtual const PriorityBp::Settings& GetSettings();
	virtual const PriorityBp::HostImage& GetInputImage();
	virtual const PriorityBp::HostImage& GetMaskImage();
	virtual PriorityBp::HostImage& GetOutputImage();
	virtual const PriorityBp::HostImage& GetOutputImage() const;
	virtual std::istream* GetPatchesIstream();
	virtual std::ostream* GetPatchesOstream();

private:
	void ApplyCommandLineOptionsToSettings(const CommandLineOptions& options);

	// Internal data
	AppCmdHostImage m_inputImage;
	AppCmdHostImage m_maskImage;
	AppCmdHostImage m_outputImage;

	PriorityBp::Settings m_settings;

	std::auto_ptr<std::ifstream> m_patchesIstream;
	std::auto_ptr<std::ofstream> m_patchesOstream;

	bool m_isValid;
};

// Trues true if the images are valid for image completion. Otherwise, logs an
// error and returns false.
static bool LoadAndValidateImage(const char* imageTypeName, const std::string& imagePath, AppCmdHostImage& image)
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
	else if (image.GetWidth() > PriorityBp::Settings::IMAGE_WIDTH_MAX || image.GetHeight() > PriorityBp::Settings::IMAGE_HEIGHT_MAX)
	{
		msgOut.Printf("The %s image is too large. Max size: %dx%x.\n", imageTypeName, PriorityBp::Settings::IMAGE_WIDTH_MAX, PriorityBp::Settings::IMAGE_HEIGHT_MAX);
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
		PriorityBp::SettingsConstruct(m_settings, m_inputImage);

		ApplyCommandLineOptionsToSettings(options);
		SettingsUi::PrintInvalidMembers settingsUiPrintInvalidMembers;
		if (PriorityBp::AreSettingsValid(m_settings, &settingsUiPrintInvalidMembers))
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

const AppCmdHostImage& AppCmdHost::GetInputImageImpl()
{
	return m_inputImage;
}

const AppCmdHostImage& AppCmdHost::GetMaskImageImpl()
{
	return m_maskImage;
}

AppCmdHostImage& AppCmdHost::GetOutputImageImpl()
{
	return m_outputImage;
}

const PriorityBp::Settings& AppCmdHost::GetSettings()
{
	return m_settings;
}

const PriorityBp::HostImage& AppCmdHost::GetInputImage()
{
	return m_inputImage;
}

const PriorityBp::HostImage& AppCmdHost::GetMaskImage()
{
	return m_maskImage;
}

PriorityBp::HostImage& AppCmdHost::GetOutputImage()
{
	return m_outputImage;
}

const PriorityBp::HostImage& AppCmdHost::GetOutputImage() const
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

void AppCmdHostImage::SetFilePath(const std::string& filePath)
{
	m_filePath = filePath;
}

wxImage& AppCmdHostImage::GetwxImage()
{
	return m_wxImage;
}

const wxImage& AppCmdHostImage::GetwxImage() const
{
	return m_wxImage;
}

bool AppCmdHostImage::IsValid() const
{
	return m_wxImage.Ok();
}

const std::string& AppCmdHostImage::GetFilePath() const
{
	return m_filePath;
}

bool AppCmdHostImage::Init(int width, int height)
{
	return m_wxImage.Create(width, height, false);
}

PriorityBp::HostImage::Rgb* AppCmdHostImage::GetRgb()
{
	return reinterpret_cast<PriorityBp::HostImage::Rgb*>(m_wxImage.GetData());
}

const PriorityBp::HostImage::Rgb* AppCmdHostImage::GetRgb() const
{
	return reinterpret_cast<const PriorityBp::HostImage::Rgb*>(m_wxImage.GetData());
}

int AppCmdHostImage::GetWidth() const
{
	return m_wxImage.GetWidth();
}

int AppCmdHostImage::GetHeight() const
{
	return m_wxImage.GetHeight();
}

//
// Functions
//
int main(int argc, char** argv)
{
    wxInitialize();

	bool succeeded = false;
	printf("\nlafarren.com\nImage Completion Using Efficient Belief Propagation\n");

#if defined(_MSC_VER)
	TECH_MEM_PROFILE("main");
	TECH_TIME_PROFILE("main");
#endif

	wxMessageOutputStderr output;
	wxMessageOutput::Set(&output);

	wxLogStderr log;
	wxLog::SetActiveTarget(&log);

	wxInitAllImageHandlers();

	const CommandLineOptions options(argc, argv);
	if (options.IsValid())
	{
		AppCmdHost host(options);
		if (host.IsValid())
		{
			succeeded = true;

			if (options.ShouldShowSettings())
			{
				SettingsUi::Print(host.GetSettings());
			}

			if (options.ShouldRunImageCompletion())
			{
				if (PriorityBp::Complete(host))
				{
					AppCmdHostImage& outputImage = host.GetOutputImageImpl();
					outputImage.GetwxImage().SaveFile(outputImage.GetFilePath());
					wxMessageOutput::Get()->Printf("Completed image and wrote %s.\n", outputImage.GetFilePath().c_str());
				}
				else
				{
					wxMessageOutput::Get()->Printf("Could not complete the image.\n");
					succeeded = false;
				}
			}
		}
	}

	return succeeded ? 0 : 1;
}
