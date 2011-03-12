#ifndef SCALABLE_DEBUGGING_H
#define SCALABLE_DEBUGGING_H

namespace PriorityBp
{
	class ImageScalable;
	class MaskScalable;
	class PriorityBpRunner;
	class SettingsScalable;

	namespace ScalableDebugging
	{
		void RunPriorityBp(
			PriorityBpRunner& priorityBpRunner,
			SettingsScalable& settingsScalable,
			ImageScalable& imageScalable,
			MaskScalable& maskScalable,
			const std::string& highResOutputFilePath,
			int depth);
	}
}

#endif // SCALABLE_DEBUGGING_H
