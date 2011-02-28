#include "Pch.h"

#include "Compositor.h"
#include "Image.h"
#include "Mask.h"
#include "PriorityBpHost.h"
#include "PriorityBpRunner.h"
#include "ScalableDebugging.h"
#include "Settings.h"

#include "tech/DbgMem.h"

namespace PriorityBp { namespace ScalableDebugging
{
	class OutputHostImage : public PriorityBp::HostImage
	{
	public:
		OutputHostImage(const wxString& highResOutputFilePath, int depth)
		{
			int lastDotIndex = highResOutputFilePath.Find('.', true);
			if (lastDotIndex == -1)
			{
				lastDotIndex = highResOutputFilePath.Len();
			}

			m_filePath = highResOutputFilePath.Left(lastDotIndex);
			m_filePath += wxString::Format("-scale-%d", depth);
			m_filePath += highResOutputFilePath.Right(highResOutputFilePath.Len() - lastDotIndex);
		}

		~OutputHostImage()
		{
			if (!m_filePath.IsEmpty())
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
		virtual const wxString& GetFilePath() const { return m_filePath; }
		virtual Rgb* GetRgb() { return reinterpret_cast<PriorityBp::HostImage::Rgb*>(m_wxImage.GetData()); }
		virtual const Rgb* GetRgb() const { return reinterpret_cast<const PriorityBp::HostImage::Rgb*>(m_wxImage.GetData()); }
		virtual int GetWidth() const { return m_wxImage.GetWidth(); }
		virtual int GetHeight() const { return m_wxImage.GetHeight(); }

	private:
		// Internal data
		wxString m_filePath;
		wxImage m_wxImage;
	};

	void RunPriorityBp(
		PriorityBpRunner& priorityBpRunner,
		SettingsScalable& settingsScalable,
		ImageScalable& imageScalable,
		MaskScalable& maskScalable,
		const wxString& highResOutputFilePath,
		int depth)
	{
		Compositor::Input compositorInput(settingsScalable, imageScalable, maskScalable);
		priorityBpRunner.RunAndGetPatches(compositorInput.patches);

		std::auto_ptr<Compositor> compositor(CompositorFactory::Create(settingsScalable.compositorPatchType, settingsScalable.compositorPatchBlender));
		if (compositor.get())
		{
			compositor->Compose(compositorInput, OutputHostImage(highResOutputFilePath, depth));
		}
	}
}}
