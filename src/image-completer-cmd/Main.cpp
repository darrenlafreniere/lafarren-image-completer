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
#include "SettingsText.h"

#include "tech/DbgMem.h"

//
// PriorityBp host interface implementations
//

// Make sure that wxWidgets wxImage::RGBValue and LfnIc::Image::Rgb
// have identical size, since we perform no conversion.
wxCOMPILE_TIME_ASSERT(sizeof(wxImage::RGBValue) == sizeof(LfnIc::Image::Rgb), INVALID_RGB_SIZE);
#ifdef __WXDEBUG__
class AssertIdenticalRgbLayout
{
public:
	AssertIdenticalRgbLayout()
	{
		wxImage::RGBValue rgb1(0, 0, 0);
		LfnIc::Image::Rgb& rgb2 = reinterpret_cast<LfnIc::Image::Rgb&>(rgb1);
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

class AppWxImage : public LfnIc::Image
{
public:
	// AppWxImage interface
	void SetFilePath(const std::string& filePath);
	wxImage& GetwxImage();
	const wxImage& GetwxImage() const;

	// LfnIc::Image interface
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

class AppCmdHost
{
public:
	// AppCmdHost interface
	AppCmdHost(const CommandLineOptions& options);
	bool IsValid() const;
	const AppWxImage& GetInputImageImpl();
	const AppWxImage& GetMaskImageImpl();
	AppWxImage& GetOutputImageImpl();

	// LfnIc::Host interface
	const LfnIc::Settings& GetSettings();
	const LfnIc::Image& GetInputImage();
	const LfnIc::Image& GetMaskImage();
	LfnIc::Image& GetOutputImage();
	const LfnIc::Image& GetOutputImage() const;
	std::istream* GetPatchesIstream();
	std::ostream* GetPatchesOstream();

private:
	void ApplyCommandLineOptionsToSettings(const CommandLineOptions& options);

	// Internal data
	AppWxImage m_inputImage;
	AppWxImage m_maskImage;
	AppWxImage m_outputImage;

	LfnIc::Settings m_settings;

	std::auto_ptr<std::ifstream> m_patchesIstream;
	std::auto_ptr<std::ofstream> m_patchesOstream;

	bool m_isValid;
};

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

void AppWxImage::SetFilePath(const std::string& filePath)
{
	m_filePath = filePath;
}

wxImage& AppWxImage::GetwxImage()
{
	return m_wxImage;
}

const wxImage& AppWxImage::GetwxImage() const
{
	return m_wxImage;
}

bool AppWxImage::IsValid() const
{
	return m_wxImage.Ok();
}

const std::string& AppWxImage::GetFilePath() const
{
	return m_filePath;
}

bool AppWxImage::Init(int width, int height)
{
	return m_wxImage.Create(width, height, false);
}

LfnIc::Image::Rgb* AppWxImage::GetRgb()
{
	return reinterpret_cast<LfnIc::Image::Rgb*>(m_wxImage.GetData());
}

const LfnIc::Image::Rgb* AppWxImage::GetRgb() const
{
	return reinterpret_cast<const LfnIc::Image::Rgb*>(m_wxImage.GetData());
}

int AppWxImage::GetWidth() const
{
	return m_wxImage.GetWidth();
}

int AppWxImage::GetHeight() const
{
	return m_wxImage.GetHeight();
}

//
// Functions
//
int main(int argc, char** argv)
{
	bool succeeded = false;
	printf("\nlafarren.com\nImage Completion Using Efficient Belief Propagation\n");

	wxInitializer initializer;
	if (!initializer.IsOk())
	{
		printf("ERROR: couldn't initialize wxWidgets!\n");
	}
	else
	{
		TECH_MEM_PROFILE("main");
		TECH_TIME_PROFILE("main");

		// These arguments must be heap allocated, because wxUninitialize
		// (called by wxInitializer's destructor) will delete them.
		wxMessageOutput::Set(new wxMessageOutputStderr());
		wxLog::SetActiveTarget(new wxLogStderr());

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
					SettingsText::Print(host.GetSettings());
				}

				if (options.ShouldRunImageCompletion())
				{
					if (LfnIc::Complete(host.GetSettings(), host.GetInputImage(), host.GetMaskImage(), host.GetOutputImage(), host.GetPatchesIstream(), host.GetPatchesOstream()))
					{
						AppWxImage& outputImage = host.GetOutputImageImpl();
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
	}

	return succeeded ? 0 : 1;
}
