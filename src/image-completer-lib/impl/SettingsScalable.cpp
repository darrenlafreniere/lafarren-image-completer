#include "Pch.h"
#include "SettingsScalable.h"

//
// SettingsScalable implementation
//
LfnIc::SettingsScalable::SettingsScalable(const Settings& settings)
	: Settings(settings)
	, m_depth(0)
{
}

void LfnIc::SettingsScalable::ScaleUp()
{
    // Copy the last saved resolution settings and pop up.
    wxASSERT(m_depth > 0);
    Settings& thisSettings = *this;
    thisSettings = m_resolutions[--m_depth];
    m_resolutions.pop_back();
}

void LfnIc::SettingsScalable::ScaleDown()
{
    // Save the current resolution settings.
    wxASSERT(static_cast<unsigned int>(m_depth) == m_resolutions.size());
    m_resolutions.push_back(*this);

    ++m_depth;

    // Reduce the lattice gap by half.
    latticeGapX /= 2;
    latticeGapY /= 2;

    // Recompute the patch size, rather than divide by two, to avoid even/odd issues.
    patchWidth = latticeGapX * PATCH_TO_LATTICE_RATIO;
    patchHeight = latticeGapY * PATCH_TO_LATTICE_RATIO;

    // NUM_NODE_LABELS_KEPT_MULTIPLIER is used so that lower resolutions keep
    // more labels per node. This is desired because error caused by the
    // reduced data is propagated to higher resolutions. Keeping more labels
    // at lower resolutions allows for better error recovery at the next
    // higher resolution.
    const int NUM_NODE_LABELS_KEPT_MULTIPLIER = 4;
    postPruneLabelsMin *= NUM_NODE_LABELS_KEPT_MULTIPLIER;
    postPruneLabelsMax *= NUM_NODE_LABELS_KEPT_MULTIPLIER;
}

int LfnIc::SettingsScalable::GetScaleDepth() const
{
    return m_depth;
}
