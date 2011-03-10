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
#include "Compositor.h"

#include "compositors/CompositorRoot.h"
#include "compositors/OutputBlenderDebugSoftMaskIntensity.h"
#include "compositors/OutputBlenderSoftMask.h"
#include "compositors/PatchBlenderNone.h"
#include "compositors/PatchBlenderPriority.h"
#include "compositors/PatchTypeNormal.h"
#include "compositors/PatchTypeDebugPatchOrder.h"
#include "compositors/PatchTypePoisson.h"

#include "tech/DbgMem.h"

//
// Compositor::Input implementation
//
Compositor::Input::Input(const Settings& settings, const Image& inputImage, const Mask& mask)
	: settings(settings)
	, inputImage(inputImage)
	, mask(mask)
{
}

//
// CompositorFactory implementation
//
Compositor* CompositorFactory::Create(CompositorPatchType patchType, CompositorPatchBlender patchBlender)
{
	Compositor* compositor = NULL;

	CompositorRoot::PatchType::Factory* patchimageSourceFactory = NULL;
	CompositorRoot::PatchBlender::Factory* patchBlenderFactory = NULL;
#if 1 // TODO: add a compositor-output-blender command line optin
	CompositorRoot::OutputBlender* outputBlender = new OutputBlenderSoftMask;
#else
	CompositorRoot::OutputBlender* outputBlender = new OutputBlenderDebugSoftMaskIntensity;
#endif

	switch (patchType)
	{
	case CompositorPatchTypeNormal:
		patchimageSourceFactory = new PatchTypeNormal::Factory;
		break;
	case CompositorPatchTypePoisson:
#ifdef USE_POISSON
		patchimageSourceFactory = new PatchTypePoisson::Factory;
#else
        #pragma message("Not built with Poisson blending enabled!")
#endif
		break;
	case CompositorPatchTypeDebugOrder:
		patchimageSourceFactory = new PatchTypeDebugPatchOrder::Factory;
		break;
    default:
        std::cout << "Warning: patchType did not match any of the handled types. Using Normal." << std::endl;
        patchimageSourceFactory = new PatchTypeNormal::Factory;
        break;
	}

	switch (patchBlender)
	{
	case CompositorPatchBlenderPriority:
		patchBlenderFactory = new PatchBlenderPriority::Factory;
		break;
	case CompositorPatchBlenderNone:
		patchBlenderFactory = new PatchBlenderNone::Factory;
		break;
    default:
        std::cout << "Warning: patchBlender did not match any of the handled types. Using None." << std::endl;
        patchBlenderFactory = new PatchBlenderNone::Factory;
        break;
	}

	if (patchimageSourceFactory && patchBlenderFactory && outputBlender)
	{
		compositor = new CompositorRoot(patchimageSourceFactory, patchBlenderFactory, outputBlender);
	}
	else
	{
		delete patchimageSourceFactory;
		delete patchBlenderFactory;
		delete outputBlender;
	}

	return compositor;
}
