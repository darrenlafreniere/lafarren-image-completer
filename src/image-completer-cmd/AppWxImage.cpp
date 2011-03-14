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

LfnIc::Image::Pixel* AppWxImage::GetRgb()
{
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_wxImage.GetData());
}

const LfnIc::Image::Pixel* AppWxImage::GetRgb() const
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
