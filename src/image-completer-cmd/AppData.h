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

#ifndef APP_DATA_H
#define APP_DATA_H

#include "LfnIcImage.h"
#include "LfnIcMask.h"
#include "LfnIcSettings.h"

class CommandLineOptions;

//
// Prepares and stores the data needed to run LfnIc::Complete().
//
class AppData
{
public:
	AppData(const CommandLineOptions& options, LfnIc::Image* inputImage, LfnIc::Mask* mask, LfnIc::Image* outputImage);
	bool IsValid() const;
	LfnIc::Image* GetOutputImage();

	const LfnIc::Settings& GetSettings();
	const LfnIc::Image* GetInputImage();
	const LfnIc::Mask* GetMask();
	const LfnIc::Image* GetOutputImage() const;
	std::istream* GetPatchesIstream();
	std::ostream* GetPatchesOstream();

private:
	void ApplyCommandLineOptionsToSettings(const CommandLineOptions& options);

	// Internal data
	LfnIc::Image* m_inputImage;
	LfnIc::Mask* m_mask;
	LfnIc::Image* m_outputImage;

	LfnIc::Settings m_settings;

	std::auto_ptr<std::ifstream> m_patchesIstream;
	std::auto_ptr<std::ofstream> m_patchesOstream;

	bool m_isValid;
};

#endif
