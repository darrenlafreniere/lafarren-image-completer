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

//
// The code below is a heavily modified version of Tommer Leyvand's Poisson
// solver code. It's been run through a refactoring process, mostly for my own
// benefit and understanding, but also to get it working directly with the
// image completer's data structures. His original notice is provided below.
//

// =============================================================================
// PoissonEditing - Poisson Image Editing for cloning and image estimation
//
// The following code implements:
// Exercise 1, Advanced Computer Graphics Course (Spring 2005)
// Tel-Aviv University, Israel
// http://leyvand.com/research/adv-graphics/ex1.htm
//
// * Based on "Poisson Image Editing" paper, Pe'rez et. al. [SIGGRAPH/2003].
// * The code uses TAUCS, A sparse linear solver library by Sivan Toledo
//   (see http://www.tau.ac.il/~stoledo/taucs)
// =============================================================================
//
// COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
//
// PoissonEditing : Copyright (C) 2005, Tommer Leyvand (tommerl@gmail.com)
//
// Covered code is provided under this license on an "as is" basis, without
// warranty of any kind, either expressed or implied, including, without
// limitation, warranties that the covered code is free of defects,
// merchantable, fit for a particular purpose or non-infringing. The entire risk
// as to the quality and performance of the covered code is with you. Should any
// covered code prove defective in any respect, you (not the initial developer
// or any other contributor) assume the cost of any necessary servicing, repair
// or correction. This disclaimer of warranty constitutes an essential part of
// this license. No use of any covered code is authorized hereunder except under
// this disclaimer.
//
// Permission is hereby granted to use, copy, modify, and distribute this
// source code, or portions hereof, for any purpose, including commercial
// applications, freely and without fee, subject to the following restrictions: 
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

#include "Pch.h"
#include "PoissonSolver.h"

#include "tech/MathUtils.h"

#include "ImageFloat.h"
#include "LfnIcImage.h"
#include "TaucsMatrix.h"

#include "tech/DbgMem.h"

namespace LfnIc { namespace Poisson
{
	class BVarInitializer;
	class BVarInitializerZero;
	class BVarInitializerDivergenceField;
	class AAndBVars;
	class UnknownVars;
	class PixelNeighbors;

	//
	// The initialization of the b variables must be polymorphic. When
	// completing using Poisson, b variables are initialized to zero, but
	// when cloning using Poisson, b variables are initialized from a
	// divergence field.
	//
	class BVarInitializer
	{
	public:
		virtual void GetInitialValue(int x, int y, RgbFloat& outPixel) const = 0;
	};

	class BVarInitializerZero : public BVarInitializer
	{
	public:
		virtual void GetInitialValue(int, int, RgbFloat& outPixel) const { outPixel.r = outPixel.g = outPixel.b = 0.0f; }
	};

	class BVarInitializerDivergenceField : public BVarInitializer
	{
	public:
		BVarInitializerDivergenceField(const ImageFloat& div) : m_div(div) {}
		virtual void GetInitialValue(int x, int y, RgbFloat& outPixel) const { outPixel = m_div.GetPixel(x, y); }
	private:
		const ImageFloat& m_div;
	};

	//
	// Encapsulates the A matrix and b variables for the equation Au = b,
	// where u are the unknowns.
	//
	class AAndBVars
	{
	public:
		AAndBVars(const ImageFloat& inputImage, const Mask& mask, const BVarInitializer& bVarInitializer, const UnknownVars& unknownVars);

		inline const TaucsMatrix& GetA() const;
		inline double* GetBVars() const;

	private:
		void Evaluate(const ImageFloat& inputImage, const Mask& mask, const UnknownVars& unknownVars, int x, int y, int& inOutColumnStartIndex, RgbFloat& inOutPixel);

		// Each Evaluate call is made only if a bool is true. Yhis overload declutters the call(s).
		inline void Evaluate(bool shouldEvaluate, const ImageFloat& inputImage, const Mask& mask, const UnknownVars& unknownVars, int x, int y, int& inOutColumnStartIndex, RgbFloat& inOutPixel);

		TaucsMatrix m_a;
		std::auto_ptr<double> m_bVars;
	};

	//
	// The set of unknowns. Unknown pixels are those that need to be solved.
	//
	class UnknownVars
	{
	public:
		UnknownVars(int width, int height, const Mask& mask);

		inline int GetWidth() const;
		inline int GetHeight() const;
		inline int GetNum() const;
		inline int GetIndex(int x, int y) const;
		inline float GetFloatClamped0To1(int index) const;

		void Solve(const AAndBVars& aAndBVars);

	private:
		typedef unsigned int Key;

		inline Key GetKeyFromXy(int x, int y) const;
		
		int m_width;
		int m_height;
		std::map<Key, int> m_xyToUnknownIndexMap;
		std::auto_ptr<double> m_uVars;
	};

	//
	// Contains validity of neighboring pixel data.
	//
	class PixelNeighbors
	{
	public:
		PixelNeighbors(int width, int height, int x, int y);
		PixelNeighbors(int width, int height, const Mask& mask, Mask::Value maskValidNeighborValue, int x, int y);

		inline int GetNum() const { return m_num; }

		inline bool HasTop() const { return m_hasTop; }
		inline bool HasBottom() const { return m_hasBottom; }
		inline bool HasLeft() const { return m_hasLeft; }
		inline bool HasRight() const { return m_hasRight; }

	private:
		void CacheNum();

		int m_num;
		bool m_hasTop;
		bool m_hasBottom;
		bool m_hasLeft;
		bool m_hasRight;
	};
}}

// Poisson::AAndBVars implementation
namespace LfnIc { namespace Poisson
{
	AAndBVars::AAndBVars(const ImageFloat& inputImage, const Mask& mask, const BVarInitializer& bVarInitializer, const UnknownVars& unknownVars)
		// Create the sparse 'a' matrix, we have at least 5 non-zero elements
		// per column
		: m_a(unknownVars.GetNum(), unknownVars.GetNum(), unknownVars.GetNum() * 5, TAUCS_DOUBLE)
		// The 'b' values, 3 channels per.
		, m_bVars(new double[unknownVars.GetNum() * 3])
	{
		const int width = inputImage.GetWidth();
		const int height = inputImage.GetHeight();
		const int numUnknowns = unknownVars.GetNum();
		double* bVars = m_bVars.get();

		int column = 0;
		int columnStartIndex = 0;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				if (mask.GetValue(x, y) == Mask::UNKNOWN)
				{
					const PixelNeighbors neighbors(width, height, x, y);
					m_a.GetColPtr(column) = columnStartIndex;

					RgbFloat pixel;
					bVarInitializer.GetInitialValue(x, y, pixel);

					Evaluate(neighbors.HasTop(), inputImage, mask, unknownVars, x, y - 1, columnStartIndex, pixel);
					Evaluate(neighbors.HasLeft(), inputImage, mask, unknownVars, x - 1, y, columnStartIndex, pixel);

					m_a.GetValue(columnStartIndex) = -neighbors.GetNum();
					m_a.GetRowIndex(columnStartIndex) = unknownVars.GetIndex(x, y);
					++columnStartIndex;

					Evaluate(neighbors.HasRight(), inputImage, mask, unknownVars, x + 1, y, columnStartIndex, pixel);
					Evaluate(neighbors.HasBottom(), inputImage, mask, unknownVars, x, y + 1, columnStartIndex, pixel);

					const int unknownIndex = unknownVars.GetIndex(x, y);
					bVars[unknownIndex + numUnknowns * 0] = pixel.r;
					bVars[unknownIndex + numUnknowns * 1] = pixel.g;
					bVars[unknownIndex + numUnknowns * 2] = pixel.b;

					++column;
				}
			}
		}

		wxASSERT(column == unknownVars.GetNum());
		m_a.GetColPtr(column) = columnStartIndex;
	}

	const TaucsMatrix& AAndBVars::GetA() const
	{
		return m_a;
	}

	double* AAndBVars::GetBVars() const
	{
		return m_bVars.get();
	}

	void AAndBVars::Evaluate(const ImageFloat& inputImage, const Mask& mask, const UnknownVars& unknownVars, int x, int y, int& inOutColumnStartIndex, RgbFloat& inOutPixel)
	{
		wxASSERT(x >= 0);
		wxASSERT(y >= 0);
		wxASSERT(x < inputImage.GetWidth());
		wxASSERT(y < inputImage.GetHeight());
		if (mask.GetValue(x, y) == Mask::UNKNOWN)
		{
			// Unknown pixel
			m_a.GetValue(inOutColumnStartIndex) = 1.0f;
			m_a.GetRowIndex(inOutColumnStartIndex) = unknownVars.GetIndex(x, y);
			++inOutColumnStartIndex;
		}
		else
		{
			// Known pixel
			inOutPixel -= inputImage.GetPixel(x, y);
		}
	}

	void AAndBVars::Evaluate(bool shouldEvaluate, const ImageFloat& inputImage, const Mask& mask, const UnknownVars& unknownVars, int x, int y, int& inOutColumnStartIndex, RgbFloat& inOutPixel)
	{
		if (shouldEvaluate)
		{
			Evaluate(inputImage, mask, unknownVars, x, y, inOutColumnStartIndex, inOutPixel);
		}
	}
}}

// Poisson::UnknownVars implementation
namespace LfnIc { namespace Poisson
{
	UnknownVars::UnknownVars(int width, int height, const Mask& mask)
		: m_width(width)
		, m_height(height)
	{
		wxASSERT(m_width <= USHRT_MAX);
		wxASSERT(m_height <= USHRT_MAX);

		int i = 0;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				if (mask.GetValue(x, y) == Mask::UNKNOWN)
				{
					m_xyToUnknownIndexMap[GetKeyFromXy(x, y)] = i++;
				}
			}
		}

		m_uVars.reset(new double[GetNum() * 3]);
	}

	int UnknownVars::GetWidth() const
	{
		return m_width;
	}

	int UnknownVars::GetHeight() const
	{
		return m_height;
	}

	int UnknownVars::GetNum() const
	{
		return m_xyToUnknownIndexMap.size();
	}

	int UnknownVars::GetIndex(int x, int y) const
	{
		return m_xyToUnknownIndexMap.find(GetKeyFromXy(x, y))->second;
	}

	float UnknownVars::GetFloatClamped0To1(int index) const
	{
		return LfnTech::Clamp0To1(static_cast<float>(m_uVars.get()[index]));
	}

	void UnknownVars::Solve(const AAndBVars& aAndBVars)
	{
		char* options[] = { "taucs.factor.LU=true", NULL }; 
		const int result = taucs_linsolve(
			aAndBVars.GetA().GetTranspose(TaucsMatrix()),
			NULL,
			3,
			m_uVars.get(),
			aAndBVars.GetBVars(),
			options,
			NULL);
		wxASSERT(result == TAUCS_SUCCESS);
	}
	
	UnknownVars::Key UnknownVars::GetKeyFromXy(int x, int y) const
	{
		wxASSERT(x >= 0);
		wxASSERT(y >= 0);
		wxASSERT(x < m_width);
		wxASSERT(y < m_height);
		return (x << 16) | y;
	}
}}

// Poisson::PixelNeighbors implementation
namespace LfnIc { namespace Poisson
{
	PixelNeighbors::PixelNeighbors(int width, int height, int x, int y)
		: m_hasTop(y > 0)
		, m_hasBottom(y < height - 1)
		, m_hasLeft(x > 0)
		, m_hasRight(x < width - 1)
	{
		CacheNum();
	}

	PixelNeighbors::PixelNeighbors(int width, int height, const Mask& mask, Mask::Value maskValidNeighborValue, int x, int y)
		: m_hasTop(y > 0 && mask.GetValue(x, y - 1) == maskValidNeighborValue)
		, m_hasBottom(y < height - 1 && mask.GetValue(x, y + 1) == maskValidNeighborValue)
		, m_hasLeft(x > 0 && mask.GetValue(x - 1, y) == maskValidNeighborValue)
		, m_hasRight(x < width - 1 && mask.GetValue(x + 1, y) == maskValidNeighborValue)
	{
		CacheNum();
	}

	void PixelNeighbors::CacheNum()
	{
		m_num =
			(m_hasTop    ? 1 : 0) +
			(m_hasBottom ? 1 : 0) +
			(m_hasLeft   ? 1 : 0) +
			(m_hasRight  ? 1 : 0);
	}
}}

// Helper functions
namespace LfnIc { namespace Poisson
{
	void WriteOutput(
		const UnknownVars& unknownVars,
		const Mask& mask,
		ImageFloat& inputOutputImage,
		int translateSolutionX,
		int translateSolutionY)
	{
		const int srcWidth = unknownVars.GetWidth();
		const int srcHeight = unknownVars.GetHeight();
		const int outputWidth = inputOutputImage.GetWidth();
		const int outputHeight = inputOutputImage.GetHeight();
		const int numUnknownVars = unknownVars.GetNum();

		// Convert solution vector back to image
		for (int y = 0; y < outputHeight; ++y)
		{
			for (int x = 0; x < outputWidth; ++x)
			{
				const int srcX = x - translateSolutionX;
				const int srcY = y - translateSolutionY;
				if (srcX >= 0 && srcY >= 0 && srcX < srcWidth && srcY < srcHeight)
				{
					if (mask.GetValue(srcX, srcY) == Mask::UNKNOWN)
					{
						const int i = unknownVars.GetIndex(srcX, srcY);
						const RgbFloat p(
							unknownVars.GetFloatClamped0To1(i + numUnknownVars * 0),
							unknownVars.GetFloatClamped0To1(i + numUnknownVars * 1),
							unknownVars.GetFloatClamped0To1(i + numUnknownVars * 2));
						inputOutputImage.SetPixel(x, y, p);
					}
				}
			}
		}
	}

	void Solve(
		const ImageFloat& inputImage,
		const Mask& mask,
		ImageFloat& outputImage,
		const BVarInitializer& bVarInitializer,
		int translateSolutionX,
		int translateSolutionY)
	{
		// Build mapping from each unknown pixel's row major index to its
		// "unknown" index.
		UnknownVars unknownVars(inputImage.GetWidth(), inputImage.GetHeight(), mask);
		if (unknownVars.GetNum() > 0)
		{
			// Solve Au = b for all 3 channels at once
			AAndBVars aAndBVars(inputImage, mask, bVarInitializer, unknownVars);

			unknownVars.Solve(aAndBVars);
			WriteOutput(unknownVars, mask, outputImage, translateSolutionX, translateSolutionY);
		}
	}
}}

// Public interface implementation
namespace LfnIc { namespace Poisson
{
	void Complete(
		const ImageFloat& inputImage,
		const Mask& mask,
		ImageFloat& outputImage)
	{
		Solve(inputImage, mask, outputImage, BVarInitializerZero(), 0, 0);
	}

	Cloner::Cloner(const ImageFloat& targetImage)
		: m_targetImage(targetImage)
		, m_sourceImageTranslated(targetImage.GetWidth(), targetImage.GetHeight())
		, m_sourceImageTranslatedRect(-1, -1, -1, -1)
		, m_laplacian(targetImage.GetWidth(), targetImage.GetHeight())
		, m_laplacianMask(targetImage.GetWidth(), targetImage.GetHeight(), Mask::KNOWN)
	{
	}

	void Cloner::Clone(
		const ImageFloat& sourceImage,
		const Mask& mask,
		int sourceOffsetX,
		int sourceOffsetY,
		ImageFloat& outputImage,
		int outputOffsetX,
		int outputOffsetY)
	{
		UpdateLaplacian(sourceImage, mask, sourceOffsetX, sourceOffsetY);

		// Solve's output offset must be the delta between the source and output.
		// This is the amount that the solved unknown data should be shifted
		// before copying to the output image.
		//
		// E.g., if the source/output offset values are the same, then the output
		// data should be written to exactly where the source was cloned into the
		// target.
		const int translateSolutionX = outputOffsetX - sourceOffsetX;
		const int translateSolutionY = outputOffsetY - sourceOffsetY;

		Solve(m_targetImage, m_laplacianMask, outputImage, BVarInitializerDivergenceField(m_laplacian), translateSolutionX, translateSolutionY);
	}

	void Cloner::UpdateLaplacian(
		const ImageFloat& sourceImage,
		const Mask& mask,
		int sourceOffsetX,
		int sourceOffsetY)
	{
		const int sourceWidth = sourceImage.GetWidth();
		const int sourceHeight = sourceImage.GetHeight();
		const int sourceImageTranslatedWidth = m_sourceImageTranslated.GetWidth();
		const int sourceImageTranslatedHeight = m_sourceImageTranslated.GetHeight();

		wxASSERT(sourceImageTranslatedWidth == m_laplacian.GetWidth());
		wxASSERT(sourceImageTranslatedHeight == m_laplacian.GetHeight());

		// Reset m_laplacianMask to all Mask::KNOWN.
		{
			for (int y = m_sourceImageTranslatedRect.y, ny = y + m_sourceImageTranslatedRect.height; y < ny; ++y)
			{
				for (int x = m_sourceImageTranslatedRect.x, nx = x + m_sourceImageTranslatedRect.width; x < nx; ++x)
				{
					m_laplacianMask.SetValue(x, y, Mask::KNOWN);
				}
			}

#if _DEBUG
			for (int y = 0; y < sourceImageTranslatedHeight; ++y)
			{
				for (int x = 0; x < sourceImageTranslatedWidth; ++x)
				{
					wxASSERT(m_laplacianMask.GetValue(x, y) == Mask::KNOWN);
				}
			}
#endif
		}

		// Set m_sourceImageTranslatedRect
		{
			m_sourceImageTranslatedRect = wxRect(sourceOffsetX, sourceOffsetY, sourceWidth, sourceHeight);
			m_sourceImageTranslatedRect.Intersect(wxRect(0, 0, sourceImageTranslatedWidth, sourceImageTranslatedHeight));
		}

		// Copy the source into m_sourceImageTranslated at the source offset,
		// and translate the source mask into m_laplacianMask.
		for (int y = 0; y < sourceImageTranslatedHeight; ++y)
		{
			for (int x = 0; x < sourceImageTranslatedWidth; ++x)
			{
				const int translatedX = x - sourceOffsetX;
				const int translatedY = y - sourceOffsetY;
				if (translatedX >= 0 && translatedY >= 0 && translatedX < sourceWidth && translatedY < sourceHeight)
				{
					const int sourceIndex = LfnTech::GetRowMajorIndex(sourceWidth, translatedX, translatedY);
					const int translatedIndex = LfnTech::GetRowMajorIndex(sourceImageTranslatedWidth, x, y);
					m_sourceImageTranslated.GetRgb()[translatedIndex] = sourceImage.GetRgb()[sourceIndex];
					m_laplacianMask.SetValue(x, y, mask.GetValue(translatedX, translatedY));
				}
			}
		}

		// Compute the laplacian from the translated source image.
		for (int y = 0; y < sourceImageTranslatedHeight; ++y)
		{
			for (int x = 0; x < sourceImageTranslatedWidth; ++x)
			{
				// Only interested in computing the laplacian of the unknown
				// pixels from the translated source.
				const PixelNeighbors neighbors(sourceImageTranslatedWidth, sourceImageTranslatedHeight, m_laplacianMask, Mask::UNKNOWN, x, y);

				if (m_laplacianMask.GetValue(x, y) == Mask::UNKNOWN || neighbors.GetNum() > 0)
				{
					RgbFloat rgb = m_sourceImageTranslated.GetPixel(x, y);
					rgb *= float(-neighbors.GetNum());

					if (neighbors.HasLeft())
					{
						rgb += m_sourceImageTranslated.GetPixel(x - 1, y);
					}
					if (neighbors.HasRight())
					{
						rgb += m_sourceImageTranslated.GetPixel(x + 1, y);
					}
					if (neighbors.HasTop())
					{
						rgb += m_sourceImageTranslated.GetPixel(x, y - 1);
					}
					if (neighbors.HasBottom())
					{
						rgb += m_sourceImageTranslated.GetPixel(x, y + 1);
					}

					m_laplacian.SetPixel(x, y, rgb);
				}
			}
		}
	}
}}
