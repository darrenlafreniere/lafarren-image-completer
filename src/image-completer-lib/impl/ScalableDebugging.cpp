#include "Pch.h"
#include "ScalableDebugging.h"

#include "tech/StrUtils.h"

#include "Compositor.h"
#include "ImageScalable.h"
#include "LfnIcImage.h"
#include "LfnIcMask.h"
#include "PriorityBpRunner.h"
#include "Settings.h"

#include "tech/DbgMem.h"

namespace LfnIc
{
	class OutputWxImage : public LfnIc::Image
	{
	public:
		OutputWxImage(const std::string& highResOutputFilePath, int depth)
		{
			int lastDotIndex = highResOutputFilePath.rfind('.');
			if (lastDotIndex == -1)
			{
				lastDotIndex = highResOutputFilePath.length();
			}

			m_filePath = highResOutputFilePath.substr(0, lastDotIndex);
			m_filePath += LfnTech::Str::Format("-scale-%d", depth);
			m_filePath += highResOutputFilePath.substr(lastDotIndex);
		}

		~OutputWxImage()
		{
			if (!m_filePath.empty())
			{
				{
					static bool wxInitAllImageHandlersHasBeenCalled = false;
					if (!wxInitAllImageHandlersHasBeenCalled)
					{
						wxInitAllImageHandlers();
						wxInitAllImageHandlersHasBeenCalled = true;
					}
				}

				m_wxImage.SaveFile(m_filePath);
			}
		}

		// LfnIc::Image interface
		virtual bool Init(int width, int height) { return m_wxImage.Create(width, height, false); }
		virtual bool IsValid() const { return m_wxImage.Ok(); }
		virtual const std::string& GetFilePath() const { return m_filePath; }
		virtual Rgb* GetRgb() { return reinterpret_cast<LfnIc::Image::Rgb*>(m_wxImage.GetData()); }
		virtual const Rgb* GetRgb() const { return reinterpret_cast<const LfnIc::Image::Rgb*>(m_wxImage.GetData()); }
		virtual int GetWidth() const { return m_wxImage.GetWidth(); }
		virtual int GetHeight() const { return m_wxImage.GetHeight(); }

	private:
		// Internal data
		std::string m_filePath;
		wxImage m_wxImage;
	};
}

namespace LfnIc { namespace ScalableDebugging
{
	void RunPriorityBp(
		PriorityBpRunner& priorityBpRunner,
		SettingsScalable& settingsScalable,
		ImageScalable& imageScalable,
		MaskScalable& maskScalable,
		const std::string& highResOutputFilePath,
		int depth)
	{
		Compositor::Input compositorInput(settingsScalable, imageScalable, maskScalable);
		priorityBpRunner.RunAndGetPatches(compositorInput.patches);

		std::auto_ptr<Compositor> compositor(CompositorFactory::Create(settingsScalable.compositorPatchType, settingsScalable.compositorPatchBlender));
		if (compositor.get())
		{
			OutputWxImage outputWxImage(highResOutputFilePath, depth);
			compositor->Compose(compositorInput, outputWxImage);
		}
	}
}}
