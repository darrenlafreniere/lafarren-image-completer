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
#include "PatchTypeDebugPatchOrder.h"

#include "ImageFloat.h"
#include "Settings.h"

#include "tech/DbgMem.h"

// Given a 0.0-1.0 alpha, returns a color from red to purple.
static LfnIc::PixelFloat GetRainbowColor(float alpha)
{
	wxASSERT(alpha >= 0.0f && alpha <= 1.0f);

	LfnIc::PixelFloat red;
	red.channel[0] = 1.00f; red.channel[1] = 0.00f; red.channel[2] = 0.00f;
	LfnIc::PixelFloat orange;
	orange.channel[0] = 1.00f; orange.channel[1] = 0.65f; orange.channel[2] = 0.00f;
	LfnIc::PixelFloat yellow;
	yellow.channel[0] = 1.00f; yellow.channel[1] = 1.00f; yellow.channel[2] = 0.00f;
	LfnIc::PixelFloat green;
	green.channel[0] = 0.00f; green.channel[1] = 0.50f; green.channel[2] = 0.00f;
	LfnIc::PixelFloat blue;
	blue.channel[0] = 0.00f; blue.channel[1] = 0.00f; blue.channel[2] = 1.00f;
	LfnIc::PixelFloat indigo;
	indigo.channel[0] = 0.30f; indigo.channel[1] = 0.00f; indigo.channel[2] = 0.51f;
	LfnIc::PixelFloat violet;
	violet.channel[0] = 0.93f; violet.channel[1] = 0.51f; violet.channel[2] = 0.93f;

	static const LfnIc::PixelFloat refColors[] =
	{
		red, orange, yellow, green, blue, indigo, violet,
	};

	static const int numRefColors = sizeof(refColors) / sizeof(refColors[0]);
	static const int penultimateRefColorIndex = numRefColors - 2;
	static const float refAlphaStep = 1.0f / float(numRefColors - 1);

	int refIndex = 0;
	float refAlphaLow = 0.0f;
	while (refIndex < penultimateRefColorIndex)
	{
		const float refAlphaHigh = refAlphaLow + refAlphaStep;
		if (alpha < refAlphaHigh)
		{
			break;
		}
		else
		{
			++refIndex;
			refAlphaLow = refAlphaHigh;
		}
	}

	const LfnIc::PixelFloat& colorLow = refColors[refIndex];
	const LfnIc::PixelFloat& colorHigh = refColors[refIndex + 1];
	const float blendAlpha = (alpha - refAlphaLow) / refAlphaStep;

	LfnIc::PixelFloat pixel;
	for (int component = 0; component < LfnIc::PixelFloatRGB::NUM_CHANNELS; ++component)
	{
		pixel.channel[component] = colorLow.channel[component] + blendAlpha * (colorHigh.channel[component] - colorLow.channel[component]);
	}
	return pixel;
}

LfnIc::CompositorRoot::PatchType* LfnIc::PatchTypeDebugPatchOrder::Factory::Create(const Compositor::Input& input, ImageFloat& imageFloat) const
{
	return new PatchTypeDebugPatchOrder(input, imageFloat);
}

LfnIc::PatchTypeDebugPatchOrder::PatchTypeDebugPatchOrder(const Compositor::Input& input, ImageFloat& imageFloat)
	: m_patches(input.patches)
	, m_patchImage(input.settings.patchWidth, input.settings.patchHeight)
{
}

const LfnIc::ImageFloat& LfnIc::PatchTypeDebugPatchOrder::Get(const Patch& patch) const
{
	LfnIc::PixelFloat rgb;
	rgb.channel[0] = 0.0f; rgb.channel[1] = 0.0f; rgb.channel[2] = 0.0f;
	for (int patchIdx = 0, patchesNum = m_patches.size(); patchIdx < patchesNum; ++patchIdx)
	{
		if (&m_patches[patchIdx] == &patch)
		{
			const float rainbowAlpha = float(patchIdx) / float(patchesNum - 1);
			rgb = GetRainbowColor(rainbowAlpha);
			break;
		}
	}

	for (int i = 0, n = m_patchImage.GetWidth() * m_patchImage.GetHeight(); i < n; ++i)
	{
		for (int component = 0; component < LfnIc::PixelFloatRGB::NUM_CHANNELS; ++component)
		{
			m_patchImage.GetData()[i].channel[component] = rgb.channel[component];
		}
	}

	return m_patchImage;
}
