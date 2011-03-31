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

#ifndef COMMAND_LINE_OPTIONS_H
#define COMMAND_LINE_OPTIONS_H

#include "LfnIcTypes.h"

#include <wx/cmdline.h>

class wxCmdLineParser;

// Reading/writing a patches file is a development-only thing when doing
// compositor work. Enable this, and use -po patches.filename on the command
// line, and the next image completion will write its solved patches data to
// patches.filename. On the next run, use -pi patches.filename to skip the
// expensive process of image completion, and read the previously written
// patches data to send to the compositor.
#define ENABLE_PATCHES_INPUT_OUTPUT 1


// Pulls the command line options from the parser, validates them, and
// presents a strongly typed interface to their values.
class CommandLineOptions
{
public:
	CommandLineOptions(int argc, char** argv);

    struct Option
    {
    public:
      Option(){} // to allow these to be stored as members of CommandLineOptions
      Option(wxString shortFlag, wxString longFlag, wxString description, bool hasDefault, int id, wxCmdLineParamType wxArgumentType, wxCmdLineEntryFlags wxMandatory, wxString strValue);
      wxString m_shortFlag;
      wxString m_longFlag;
      wxString m_description;

      size_t m_id;

      wxString m_strValue; // A string of the value

      bool wasFound;

      wxCmdLineParamType m_wxArgumentType;
      wxCmdLineEntryFlags m_wxMandatory;

      inline static const char* Indent() { return "   "; }

      void Print(wxMessageOutput& msgOut, int descWidth) const
      {
        const std::string descSpacing(' ', descWidth - m_description.length());
        msgOut.Printf("\t%s:%s %s\n", m_description.c_str(), descSpacing.c_str(), m_strValue.c_str());
      }

      enum OptionTypeEnum {COMPLETER_OPTION_TYPE, COMPOSITOR_OPTION_TYPE};

      OptionTypeEnum m_OptionType;
    };

    template<typename T>
    struct TypedOption : public Option
    {
        T value;

        TypedOption()// : wasFound(false)
        {}

        // Why wasn't this inherited?
        TypedOption(wxString shortFlag, wxString longFlag, wxString description, bool hasDefault, int id, wxCmdLineParamType wxArgumentType, wxCmdLineEntryFlags wxMandatory = wxCMD_LINE_PARAM_OPTIONAL, wxString strValue = "") : Option(shortFlag, longFlag, description, hasDefault, id, wxArgumentType, wxMandatory, strValue) {}

        void Find(const wxCmdLineParser& parser);
    };

	inline bool IsValid() const { return m_isValid; }

	inline bool HasInputImagePath() const { return !m_optImageInput.value.empty(); }
	inline bool HasMaskImagePath() const { return !m_optImageMask.value.empty(); }
	inline bool HasOutputImagePath() const { return !m_optImageOutput.value.empty(); }

	inline const std::string& GetInputImagePath() const { return m_optImageInput.value; }
	inline const std::string& GetMaskImagePath() const { return m_optImageMask.value; }
	inline const std::string& GetOutputImagePath() const { return m_optImageOutput.value; }

#if ENABLE_PATCHES_INPUT_OUTPUT
	inline bool HasInputPatchesPath() const { return !m_optPatchesInput.value.empty(); }
	inline bool HasOutputPatchesPath() const { return !m_optPatchesOutput.value.empty(); }

	inline const std::string& GetInputPatchesPath() const { return m_optPatchesInput.value; }
	inline const std::string& GetOutputPatchesPath() const { return m_optPatchesOutput.value; }
#endif // ENABLE_PATCHES_INPUT_OUTPUT

	inline bool ShouldShowSettings() const { return m_shouldShowSettings; }
	inline bool ShouldRunImageCompletion() const { return m_shouldRunImageCompletion; }

	inline bool DebugLowResolutionPasses() const { return m_optDebugLowResolutionPasses.value; }

	inline bool HasLowResolutionPassesMax() const { return m_optLowResolutionPassesMax.wasFound; }
	inline int GetLowResolutionPassesMax() const { return m_optLowResolutionPassesMax.value; }

	inline bool HasNumIterations() const { return m_optNumIterations.wasFound; }
	inline int GetNumIterations() const { return m_optNumIterations.value; }

	inline bool HasLatticeGapX() const { return m_optLatticeWidth.wasFound; }
	inline int GetLatticeGapX() const { return m_optLatticeWidth.value; }

	inline bool HasLatticeGapY() const { return m_optLatticeHeight.wasFound; }
	inline int GetLatticeGapY() const { return m_optLatticeHeight.value; }

	inline bool HasPostPruneLabelsMin() const { return m_optPatchesMin.wasFound; }
	inline int GetPostPruneLabelsMin() const { return m_optPatchesMin.value; }

	inline bool HasPostPruneLabelsMax() const { return m_optPatchesMax.wasFound; }
	inline int GetPostPruneLabelsMax() const { return m_optPatchesMax.value; }

	inline bool HasCompositorPatchType() const { return m_optCompositorPatchType.wasFound; }
	inline LfnIc::CompositorPatchType GetCompositorPatchType() const { return m_optCompositorPatchType.value; }

	inline bool HasCompositorPatchBlender() const { return m_optCompositorPatchBlender.wasFound; }
	inline LfnIc::CompositorPatchBlender GetCompositorPatchBlender() const { return m_optCompositorPatchBlender.value; }

    Option* FindOptionById(size_t id) const;
    Option* FindOptionByShortFlag(wxString shortFlag) const;
    Option* GetOption(unsigned int i) const;

    std::vector<Option*> GetOptionsByType(Option::OptionTypeEnum) const;

    TypedOption<std::string> m_optImageInput;
    TypedOption<std::string> m_optImageMask;
    TypedOption<std::string> m_optImageOutput;
    TypedOption<bool> m_optSettingsShow;
    TypedOption<bool> m_optDebugLowResolutionPasses;
    TypedOption<int> m_optLowResolutionPassesMax;
    TypedOption<long> m_optNumIterations;
    TypedOption<long> m_optLatticeWidth; // Should these be X/Y or Width/Height?
    TypedOption<long> m_optLatticeHeight;
    TypedOption<long> m_optPatchesMin; // These should be called Labels instead of Patches to match SettingsText.cpp
    TypedOption<long> m_optPatchesMax;
    TypedOption<LfnIc::CompositorPatchType> m_optCompositorPatchType;
    TypedOption<LfnIc::CompositorPatchBlender> m_optCompositorPatchBlender;

    #if ENABLE_PATCHES_INPUT_OUTPUT
    TypedOption<std::string> m_optPatchesInput;
    TypedOption<std::string> m_optPatchesOutput;
    #endif // ENABLE_PATCHES_INPUT_OUTPUT

    inline int GetNumberOfOptions() { return m_Options.size();}

private:

	bool m_shouldShowSettings;
	bool m_shouldRunImageCompletion;
	bool m_isValid;

    std::vector<Option*> m_Options;
};

#endif // COMMAND_LINE_OPTIONS_H
