/*
 * SPDX-FileCopyrightText: 2026 WackyIdeas
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "particleeffect.h"

namespace KWin
{
    KWIN_EFFECT_FACTORY_SUPPORTED(
        ParticleEffect, "metadata.json",
        {
            return ParticleEffect::supported();
        }
    )
}

#include "plugin.moc"
