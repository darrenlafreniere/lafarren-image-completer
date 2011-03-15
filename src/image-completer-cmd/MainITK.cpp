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

#include "AppData.h"
#include "CommandLineOptions.h"
#include "LfnIc.h"
#include "SettingsText.h"
#include "ITKImage.h"
#include "ITKMask.h"

#include "tech/DbgMem.h"

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
            ITKImage inputImage;
            inputImage.LoadAndValidate(options.GetInputImagePath());
            ITKMask mask;
            mask.LoadAndValidate(options.GetMaskImagePath());
            ITKImage outputImage;

			AppData appData(options, &inputImage, &mask, &outputImage);
			if (appData.IsValid())
			{
				if (options.ShouldShowSettings())
				{
					SettingsText::Print(appData.GetSettings());
					succeeded = true;
				}

				if (options.ShouldRunImageCompletion())
				{
					succeeded = LfnIc::Complete(
						appData.GetSettings(),
						*(appData.GetInputImage()),
						*(appData.GetMask()),
						*(appData.GetOutputImage()),
						appData.GetPatchesIstream(),
						appData.GetPatchesOstream());

					if (succeeded)
					{
						appData.GetOutputImage()->Save();
						wxMessageOutput::Get()->Printf("Completed image and wrote %s.\n", appData.GetOutputImage()->GetFilePath().c_str());
					}
					else
					{
						wxMessageOutput::Get()->Printf("Could not complete the image.\n");
					}
				}
			}
		}
	}

	return succeeded ? 0 : 1;
}
