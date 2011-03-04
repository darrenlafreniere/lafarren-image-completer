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

#ifndef PRIORITY_BP_HOST_H
#define PRIORITY_BP_HOST_H

#include <iostream>

namespace PriorityBp
{
	// Forward declarations
	class Host;
	class HostImage;
	class Settings;

	//
	// Host interface, to be implemented by the host and passed to
	// PriorityBp::Complete.
	//
	class Host
	{
	public:
		// Must return a const reference to the settings object for the
		// completion.
		virtual const Settings& GetSettings() = 0;

		// Must return a const reference to the input image for the
		// completion.
		virtual const HostImage& GetInputImage() = 0;

		// Must return a const reference to the input mask for the
		// completion.
		virtual const HostImage& GetMaskImage() = 0;

		// Must return a non-const reference to the output image object to
		// receive the completed image.
		virtual HostImage& GetOutputImage() = 0;
		virtual const HostImage& GetOutputImage() const = 0;

		// May optionally return a std::istream pointer from which patches
		// from a previous completion will be read and composited into the
		// output image, skipping a new completion altogether.
		//
		// Return NULL to force a new completion.
		virtual std::istream* GetPatchesIstream() = 0;

		// May optionally return a std::ostream pointer into which patches
		// from the completion will be written. This enables the host to save
		// a completion for re-compositing at a later point in time.
		//
		// Return NULL if patching writing is not required.
		virtual std::ostream* GetPatchesOstream() = 0;

	protected:
		// Instances cannot be destroyed through a base Host pointer.
		virtual ~Host() {}
	};

	// HostImage interface. Used for both reading the input image and
	// writing the output image.
	class HostImage
	{
	public:
		struct Rgb;

		// Non-const method to initialize the image to the specified width
		// and height. Any existing rgb data is not necessarily reserved.
		// Returns true if there were no errors.
		virtual bool Init(int width, int height) = 0;

		// Returns true if the image is of valid dimensions with a valid
		// rgb buffer.
		virtual bool IsValid() const = 0;

		// Returns the image's file path, if any.
		virtual const std::string& GetFilePath() const = 0;

		// Non-const and const access to the image's Rgb buffer. The
		// result points to the upperleft-most pixel and the buffer is
		// stored in row-major order, where the start of each row is
		// separated by GetStride() bytes.
		virtual Rgb* GetRgb() = 0;
		virtual const Rgb* GetRgb() const = 0;

		// Returns the image's width.
		virtual int GetWidth() const = 0;

		// Returns the image's height.
		virtual int GetHeight() const = 0;

		// Rgb structure allows for access to the data using long-hand
		// component names, short-hand component names, or a channel array.
		struct Rgb
		{
			inline Rgb() {}
			inline Rgb(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}

			union
			{
				struct
				{
					union
					{
						unsigned char red;
						unsigned char r;
					};
					union
					{
						unsigned char green;
						unsigned char g;
					};
					union
					{
						unsigned char blue;
						unsigned char b;
					};
				};

				static const int NUM_CHANNELS = 3;
				unsigned char channel[NUM_CHANNELS];
			};
		};

	protected:
		// Instances cannot be destroyed through a base HostImage pointer
		// using delete.
		virtual ~HostImage() {}
	};
}

#endif
