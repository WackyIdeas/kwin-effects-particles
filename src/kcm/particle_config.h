/*
    SPDX-FileCopyrightText: 2010 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2010 Alexandre Pereira <pereira.alex@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_particle_config.h"
#include <KCModule>
#include <QFileDialog>

namespace KWin
{

class ParticleEffectConfig : public KCModule
{
    Q_OBJECT

public:
    explicit ParticleEffectConfig(QObject *parent, const KPluginMetaData &data);
    ~ParticleEffectConfig() override;

    void save() override;

    void addListItem(QColor c);

    void populateList();

private Q_SLOTS:
	void setTexturePath();
	void clearTexturePath();

    void removeColor();
    void addColor();

    void listSelectionChanged();
    void colorizationTypeChanged(int index);

private:
    ::Ui::ParticleEffectConfig ui;
	QFileDialog* m_dialog;
    QStringList m_colors;
};

} // namespace KWin
