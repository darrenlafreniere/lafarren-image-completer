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

#ifndef ENERGY_WSST_H
#define ENERGY_WSST_H

#include "EnergyCalculatorFftConfig.h"
#if ENABLE_ENERGY_CALCULATOR_FFT

#include "LfnIcTypes.h"

namespace LfnIc
{
	class ImageConst;
	class MaskLod;

	///
	/// Windowed Sum Squared Table, used by the EnergyCalculatorFft class.
	///
	/// See http://www.cs.sfu.ca/~torsten/Publications/Papers/icip02.pdf
	///
	class EnergyWsst
	{
	public:
		/// Calculates sums across blockWidth * blockHeight areas. The second
		/// constructor applies the mask to the input image.
		EnergyWsst(const ImageConst& inputImage, int blockWidth, int blockHeight);
		EnergyWsst(const ImageConst& inputImage, const MaskLod& mask, int blockWidth, int blockHeight);
		~EnergyWsst();

		int GetBlockWidth() const;
		int GetBlockHeight() const;

		/// width and height must be multiples of blockWidth and blockHeight,
		/// respectively. Otherwise, this method fails and returns ENERGY_MIN.
		Energy Calculate(int left, int top, int width, int height) const;

	private:
		//
		// Internal methods
		//
		void Construct(const ImageConst& inputImage, const MaskLod* mask);

		//
		// Data
		//
		const int m_blockWidth;
		const int m_blockHeight;

		// m_tableWidth  = m_blockWidth + imageWidth
		// m_tableHeight = m_blockHeight + imageHeight
		// The top and left side are padded by m_blockWidth and m_blockHeight.
		const int m_tableWidth;
		const int m_tableHeight;

		// Row-major table buffer of m_tableWidth*m_tableHeight elements.
		Energy* m_table;
	};
}

#endif // ENABLE_ENERGY_CALCULATOR_FFT
#endif // ENERGY_WSST_H
