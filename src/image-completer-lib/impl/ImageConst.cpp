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
#include "ImageConst.h"

#include "tech/DbgMem.h"


bool LfnIc::ImageConst::Init(int width, int height)
{
	// Not supported, and not expected to be called.
	wxASSERT(false);
	return false;
}

bool LfnIc::ImageConst::IsValid() const
{
	return true;
}

const std::string& LfnIc::ImageConst::GetFilePath() const
{
	static const std::string empty("");
	return empty;
}

LfnIc::Image::Pixel* LfnIc::ImageConst::GetData()
{
	// Not supported, and not expected to be called.
	wxASSERT(false);
	return NULL;
}
