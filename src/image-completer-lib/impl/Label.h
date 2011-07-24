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

#ifndef LABEL_H
#define LABEL_H

#include "tech/Core.h"

#include "Scalable.h"

namespace LfnIc
{
	// Forward declarations
	class ImageConst;
	class MaskLod;
	struct Settings;

	///
	/// The Markov Random Field's labels correspond to fixed size patches
	/// within the source image. The label stores the left-top point of the
	/// patch, and the width and height is described in the Settings class.
	///
	struct Label
	{
		inline Label() {}
		inline Label(short left, short top) : left(left), top(top) {}
		inline bool operator==(const Label& other) const { return left == other.left && top == other.top; }

		short left;
		short top;
	};

	///
	/// Contains the entire label set within the source image, sorted from
	/// top to bottom, left to right.
	///
	class LabelSet : public Scalable
	{
	public:
		LabelSet(const Settings& settings, const ImageConst& inputImage, const MaskLod& mask);
		~LabelSet();

		const Label& operator[](int i) const;
		int size() const;

		/// Scalable interface
		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

		/// See GetLowToCurrentResolutionMapping().
		class LowToCurrentResolutionMapping
		{
		public:
			inline LowToCurrentResolutionMapping() : m_size(0) {}

			inline const Label& operator[](int i) const
			{
				wxASSERT(static_cast<unsigned int>(i) < sizeof(m_labels) / sizeof(m_labels[0]));
				return m_labels[i];
			}

			inline int size() const { return m_size; }

		private:
			friend class LabelSet;
			int m_size;

			/// At most one low resolution label can map to 9 high resolution
			/// labels. See the edge case in:
			/// LabelSet::Resolution::Resolution(const Resolution&)
			Label m_labels[9];
		};

		/// Given a low resolution label, this method populates the passed in
		/// LowToCurrentResolutionMapping object with the high resolution labels
		/// that correspond to it.
		void GetLowToCurrentResolutionMapping(const Label& lowResolutionLabel, LowToCurrentResolutionMapping& out) const;

	private:
		// Internal bit array class for marking which labels are valid.
		// Initializes all label bits to invalid. Used for scaling.
		class LabelBitArray
		{
		public:
			LabelBitArray(int width, int height);
			~LabelBitArray();

			inline int GetWidth() const { return m_width; }
			inline int GetHeight() const { return m_height; }

			void Set(int x, int y);
			bool IsSet(int x, int y) const;

		private:
			void GetDataElementIndexAndBitIndex(int x, int y, int& outDataElementIndex, int& outBitIndex) const;

			typedef uint DataType;
			static const int NUM_DATA_TYPE_BITS = sizeof(DataType) * 8;

			DataType* m_data;
			const int m_width;
			const int m_height;
			const int m_dataNumElements;
		};

		struct Resolution
		{
			Resolution(const Settings& settings, const ImageConst& inputImage, const MaskLod& mask);
			Resolution(const Resolution& resolutionToScaleDown);

#if _DEBUG
			void VerifyIntegrity();
#endif

			LabelBitArray labelBitArray;
			std::vector<Label> labels;
		};

		inline Resolution& GetCurrentResolution() const { return *m_resolutions[m_depth]; }

		const ImageConst& m_inputImage;
		std::vector<Resolution*> m_resolutions;
		int m_depth;
	};
}

#endif
