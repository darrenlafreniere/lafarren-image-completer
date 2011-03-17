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

#ifdef USE_WX
#include "AppImageWx.h"
#include "LfnIcSettings.h"

bool AppImageWx::LoadAndValidate(const std::string& imagePath)
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

void AppImageWx::Save()
{
	m_wxImage.SaveFile(m_filePath);
}

bool AppImageWx::IsValid() const
{
	return m_wxImage.Ok();
}

const std::string& AppImageWx::GetFilePath() const
{
	return m_filePath;
}

bool AppImageWx::Init(int width, int height)
{
	return m_wxImage.Create(width, height, false);
}

LfnIc::Image::Pixel* AppImageWx::GetData()
{
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_wxImage.GetData());
}

const LfnIc::Image::Pixel* AppImageWx::GetData() const
{
	return reinterpret_cast<const LfnIc::Image::Pixel*>(m_wxImage.GetData());
}

int AppImageWx::GetWidth() const
{
	return m_wxImage.GetWidth();
}

int AppImageWx::GetHeight() const
{
	return m_wxImage.GetHeight();
}

#endif // USE_WX
