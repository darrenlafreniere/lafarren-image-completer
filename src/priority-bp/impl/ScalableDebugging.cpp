#include "Pch.h"
#include "ScalableDebugging.h"

#include "tech/StrUtils.h"

#include "Compositor.h"
#include "Image.h"
#include "Mask.h"
#include "PriorityBpHost.h"
#include "PriorityBpRunner.h"
#include "Settings.h"

#include "tech/DbgMem.h"

namespace PriorityBp
{
	class OutputHostImage : public PriorityBp::HostImage
	{
	public:
		OutputHostImage(const std::string& highResOutputFilePath, int depth)
		{
			int lastDotIndex = highResOutputFilePath.rfind('.');
			if (lastDotIndex == -1)
			{
				lastDotIndex = highResOutputFilePath.length();
			}

			m_filePath = highResOutputFilePath.substr(0, lastDotIndex);
			m_filePath += Tech::Str::Format("-scale-%d", depth);
			m_filePath += highResOutputFilePath.substr(lastDotIndex);
		}

		~OutputHostImage()
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

		// PriorityBp::HostImage interface
		virtual bool Init(int width, int height) { return m_wxImage.Create(width, height, false); }
		virtual bool IsValid() const { return m_wxImage.Ok(); }
		virtual const std::string& GetFilePath() const { return m_filePath; }
		virtual Rgb* GetRgb() { return reinterpret_cast<PriorityBp::HostImage::Rgb*>(m_wxImage.GetData()); }
		virtual const Rgb* GetRgb() const { return reinterpret_cast<const PriorityBp::HostImage::Rgb*>(m_wxImage.GetData()); }
		virtual int GetWidth() const { return m_wxImage.GetWidth(); }
		virtual int GetHeight() const { return m_wxImage.GetHeight(); }

	private:
		// Internal data
		std::string m_filePath;
		wxImage m_wxImage;
	};
}

namespace PriorityBp { namespace ScalableDebugging
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
			OutputHostImage outputHostImage(highResOutputFilePath, depth);
			compositor->Compose(compositorInput, outputHostImage);
		}
	}
}}
