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
#include "LfnIc.h"

#include "tech/Profile.h"

#include "Compositor.h"
#include "EnergyCalculatorContainer.h"
#include "ImageScalable.h"
#include "Label.h"
#include "MaskScalable.h"
#include "NodeSet.h"
#include "Patch.h"
#include "LfnIcImage.h"
#include "PriorityBpRunner.h"
#include "ScalableDebugging.h"
#include "Settings.h"

#include "tech/DbgMem.h"

//
// PriorityBp implementation
//
namespace LfnIc
{
	static bool ReadPatches(std::istream& patchesIstream, std::vector<Patch>& outPatches)
	{
		bool result = false;

		try
		{
			int numPatches = 0;
			patchesIstream.read((char*)&numPatches, sizeof(numPatches));

			outPatches.resize(numPatches);
			if (numPatches > 0)
			{
				patchesIstream.read((char*)&outPatches[0], sizeof(Patch) * numPatches);
			}
			result = true;
		}
		catch (...)
		{
		}

		return result;
	}

	static bool WritePatches(std::ostream& patchesOstream, const std::vector<Patch>& patches)
	{
		bool result = false;

		try
		{
			const int numPatches = patches.size();
			patchesOstream.write((char*)&numPatches, sizeof(numPatches));
			patchesOstream.write((char*)&patches[0], sizeof(Patch) * numPatches);
			result = true;
		}
		catch (...)
		{
		}

		return result;
	}

	void RecurivelyRunFromLowestToNextHighestResolution(
		SettingsScalable& settingsScalable,
		ImageScalable& imageScalable,
		MaskScalable& maskScalable,
		const EnergyCalculatorContainer& energyCalculatorContainer,
		LabelSet& labelSet,
		NodeSet& nodeSet,
		PriorityBpRunner& priorityBpRunner,
		const std::string& highResOutputFilePath,
		int pass)
	{
		// Patch and image side dimensions are reduced by half at each lower
		// resolution. Patch cannot reduce lower than LOW_RES_PATCH_SIDE_MIN,
		// and images cannot be reduced lower than IMAGE_SIDE_REDUCTION_MIN.
		const int LOW_RES_PATCH_SIDE_MIN = Settings::PATCH_SIDE_MIN / 2;
		const int IMAGE_SIDE_REDUCTION_MIN = 50;

		bool shouldEvaluateThisResolution = false;
		{
			// If lowResolutionPassesMax is LOW_RESOLUTION_PASSES_AUTO, then
			// only stop once we've hit a too-low resolution. If
			// lowResolutionPassesMax is valid, stop if this pass has exceeded
			// that max.
			if (settingsScalable.lowResolutionPassesMax == Settings::LOW_RESOLUTION_PASSES_AUTO || pass <= settingsScalable.lowResolutionPassesMax)
			{
				// Calculate the patch and image
				const int patchWidth = settingsScalable.patchWidth / 2;
				const int patchHeight = settingsScalable.patchHeight / 2;
				const int imageWidth = imageScalable.GetWidth() / 2;
				const int imageHeight = imageScalable.GetHeight() / 2;

				shouldEvaluateThisResolution =
					patchWidth >= LOW_RES_PATCH_SIDE_MIN &&
					patchHeight >= LOW_RES_PATCH_SIDE_MIN &&
					imageWidth >= IMAGE_SIDE_REDUCTION_MIN &&
					imageHeight >= IMAGE_SIDE_REDUCTION_MIN;
			}
		}

		if (shouldEvaluateThisResolution)
		{
			// maskScalable MUST be scaled down after imageScalable, because
			// imageScalable uses the current resolution of maskScalable to
			// exclude unknown pixels from being averaged into the scaled down
			// image. imageScalable will assert otherwise.
			ScopedScaleDownAndUpInOrder scopedScaleDownAndUpInOrder;
			scopedScaleDownAndUpInOrder.Add(settingsScalable);
			scopedScaleDownAndUpInOrder.Add(imageScalable);
			scopedScaleDownAndUpInOrder.Add(maskScalable);
			scopedScaleDownAndUpInOrder.Add(labelSet);
			scopedScaleDownAndUpInOrder.Add(nodeSet);

			// Recurse to the next lower resolution.
			RecurivelyRunFromLowestToNextHighestResolution(
				settingsScalable,
				imageScalable,
				maskScalable,
				energyCalculatorContainer,
				labelSet,
				nodeSet,
				priorityBpRunner,
				highResOutputFilePath,
				pass + 1);

			// Run priority-bp at this resolution.
			if (!settingsScalable.debugLowResolutionPasses)
			{
				priorityBpRunner.Run();
			}
			else
			{
				ScalableDebugging::RunPriorityBp(priorityBpRunner, settingsScalable, imageScalable, maskScalable, highResOutputFilePath, pass);
			}
		}
	}

	bool ValidateImage(const Image& image)
	{
		return
			image.IsValid() &&
			image.GetWidth() <= LfnIc::Settings::IMAGE_WIDTH_MAX &&
			image.GetHeight() <= LfnIc::Settings::IMAGE_HEIGHT_MAX;
	}

	bool Complete(
		const Settings& settings,
		const Image& inputImage,
		const Mask& mask,
		Image& outputImage,
		std::istream* patchesIstream,
		std::ostream* patchesOstream)
	{
		bool succeeded = false;

		if (ValidateImage(inputImage))
		{
			// In Windows, this project is compiled into its own dll, with its
			// own global memory space, and statically links against wxWidgets.
			// Therefore, even if the host application contains its own
			// wxInitializer, this dll needs one as well. For other project
			// configurations, where this is statically built into the host,
			// it's safe to have multiple wxInitializer instances; they're
			// internally ref counted by wxWidgets.
			wxInitializer initializer;

			{
				SettingsScalable settingsScalable(settings);
				MaskScalable maskScalable(inputImage.GetWidth(), inputImage.GetHeight(), mask);
				ImageScalable imageScalable(inputImage, maskScalable);
				Compositor::Input compositorInput(settingsScalable, imageScalable, maskScalable);
				bool arePatchesValid = false;

				if (patchesIstream)
				{
					// A patches istream was provided; read the patches from it rather
					// than taking the time to solve via Priority-BP. This is useful
					// when developing new compositors.
					arePatchesValid = ReadPatches(*patchesIstream, compositorInput.patches);
				}
				else
				{
					TECH_TIME_PROFILE("ImageCompleter::Complete - Priority-BP");

					// Construct priority-bp related data, passing in the required dependencies.
					EnergyCalculatorContainer energyCalculatorContainer(settingsScalable, imageScalable, maskScalable);
					LabelSet labelSet(settingsScalable, imageScalable, maskScalable);
					NodeSet nodeSet(settingsScalable, imageScalable, maskScalable, labelSet, energyCalculatorContainer);
					PriorityBpRunner priorityBpRunner(settingsScalable, nodeSet);

					// Recurse and scale down to a quickly solvable resolution, then
					// scale back up, using each lower resolution data to help solve
					// the higher resolution.
					RecurivelyRunFromLowestToNextHighestResolution(
						settingsScalable,
						imageScalable,
						maskScalable,
						energyCalculatorContainer,
						labelSet,
						nodeSet,
						priorityBpRunner,
						outputImage.GetFilePath(),
						1);

					// Original resolution pass
					priorityBpRunner.RunAndGetPatches(compositorInput.patches);
					arePatchesValid = true;
				}

				// Composite the output image based on the patches that Priority-BP
				// solved.
				if (arePatchesValid)
				{
					// An ostream was provided to write the patches to.
					if (patchesOstream)
					{
						WritePatches(*patchesOstream, compositorInput.patches);
					}

					{
						TECH_TIME_PROFILE("ImageCompleter::Complete - Compositing");
						std::auto_ptr<Compositor> compositor(CompositorFactory::Create(settingsScalable.compositorPatchType, settingsScalable.compositorPatchBlender));
						if (compositor.get())
						{
							compositor->Compose(compositorInput, outputImage);
						}
					}
				}
			}

			succeeded = outputImage.IsValid();
		}

		return succeeded;
	}
}
