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
#include "AppWxImage.h"
#include "LfnIcSettings.h"

// Make sure that wxWidgets wxImage::RGBValue and LfnIc::Image::Rgb
// have identical size, since we perform no conversion.
wxCOMPILE_TIME_ASSERT(sizeof(wxImage::RGBValue) == sizeof(LfnIc::Image::Pixel), INVALID_RGB_SIZE);
#ifdef __WXDEBUG__
class AssertIdenticalRgbLayout
{
public:
	AssertIdenticalRgbLayout()
	{
		wxImage::RGBValue rgb1(0, 0, 0);
		LfnIc::Image::Pixel& rgb2 = reinterpret_cast<LfnIc::Image::Pixel&>(rgb1);
		rgb2.channel[0] = 1;
		rgb2.channel[1] = 2;
		rgb2.channel[2] = 3;
		wxASSERT(rgb1.red == 1);
		wxASSERT(rgb1.green == 2);
		wxASSERT(rgb1.blue == 3);
	}
};
static const AssertIdenticalRgbLayout g_assertIdenticalRgbLayout;
#endif

bool AppWxImage::LoadAndValidate(const std::string& imagePath)
{
	bool result = false;
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	if (!m_wxImage.LoadFile(imagePath))
	{
		// If LoadFile fails, it already prints an wxMessageOutput error for us.
	}
	else if (!m_wxImage.IsOk())
	{
		msgOut.Printf("The image was invalid.\n");
	}
	else if (m_wxImage.GetWidth() > LfnIc::Settings::IMAGE_WIDTH_MAX || m_wxImage.GetHeight() > LfnIc::Settings::IMAGE_HEIGHT_MAX)
	{
		msgOut.Printf("The image is too large. Max size: %dx%x.\n", LfnIc::Settings::IMAGE_WIDTH_MAX, LfnIc::Settings::IMAGE_HEIGHT_MAX);
	}
	else
	{
		result = true;
	}

	return result;
}

void AppWxImage::Save()
{
	m_wxImage.SaveFile(m_filePath);
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

LfnIc::Image::Pixel* AppWxImage::GetData()
{
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_wxImage.GetData());
}

const LfnIc::Image::Pixel* AppWxImage::GetData() const
{
	return reinterpret_cast<const LfnIc::Image::Pixel*>(m_wxImage.GetData());
}

int AppWxImage::GetWidth() const
{
	return m_wxImage.GetWidth();
}

int AppWxImage::GetHeight() const
{
	return m_wxImage.GetHeight();
}
