/*
    SPDX-FileCopyrightText: 2010 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2010 Alexandre Pereira <pereira.alex@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "particle_config.h"

//#include "config-kwin.h"

// KConfigSkeleton
#include "particleconfig.h"

#include <QColorDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>

#include <KPluginFactory>
#include "kwineffects_interface.h"
#define KWIN_CONFIG "kwinrc"

namespace KWin
{

K_PLUGIN_CLASS(ParticleEffectConfig)

ParticleEffectConfig::ParticleEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());
    ParticleConfig::instance(QStringLiteral(KWIN_CONFIG));
    addConfig(ParticleConfig::self(), widget());

	m_dialog = new QFileDialog();
	m_dialog->setFileMode(QFileDialog::ExistingFile);
	m_dialog->setNameFilter(QStringLiteral("PNG files (*.png)"));

    ui.colorPool->setSelectionMode(QListWidget::ExtendedSelection);

    connect(ui.kcfg_ColorType, SIGNAL(currentIndexChanged(int)), this, SLOT(colorizationTypeChanged(int)));
    connect(ui.colorPool, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));
	connect(ui.browse_button, SIGNAL(clicked()), this, SLOT(setTexturePath()));
    connect(ui.clear_button, SIGNAL(clicked()), this, SLOT(clearTexturePath()));
	connect(ui.addColor_button, SIGNAL(clicked()), this, SLOT(addColor()));
    connect(ui.removeColor_button, SIGNAL(clicked()), this, SLOT(removeColor()));

    populateList();
    colorizationTypeChanged(ui.kcfg_ColorType->currentIndex());

}

ParticleEffectConfig::~ParticleEffectConfig()
{
    if(m_dialog)
    {
        delete m_dialog;
    }
}

void ParticleEffectConfig::populateList()
{
    m_colors = ui.kcfg_ColorPool->text().split(QStringLiteral(","));
    for(QString col : m_colors)
    {
        QColor c = QColor::fromString(col);
        if(c.isValid())
        {
            addListItem(c);
        }
    }
    listSelectionChanged();
}

void ParticleEffectConfig::save()
{
    KCModule::save();
    OrgKdeKwinEffectsInterface interface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(QStringLiteral("libkwin_effect_particleeffect"));
}

void ParticleEffectConfig::clearTexturePath()
{
	ui.kcfg_TextureLocation->setText(QStringLiteral(""));
}
void ParticleEffectConfig::setTexturePath()
{
    if(m_dialog->exec())
    {
        ui.kcfg_TextureLocation->setText(m_dialog->selectedFiles()[0]);
    }
}
void ParticleEffectConfig::removeColor()
{
    auto items = ui.colorPool->selectedItems();
    if(items.size() > 0)
    {
        for(QListWidgetItem *i : items)
        {
            m_colors.removeAt(m_colors.indexOf(i->text()));
            delete i;
        }
    }
    ui.kcfg_ColorPool->setText(m_colors.join(QStringLiteral(",")));
}
void ParticleEffectConfig::addColor()
{
    QColor c = QColorDialog::getColor(Qt::white, widget(), i18n("Add color"));
    if(c.isValid())
    {
        addListItem(c);
        m_colors.append(c.name());
        ui.kcfg_ColorPool->setText(m_colors.join(QStringLiteral(",")));
    }
}
void ParticleEffectConfig::addListItem(QColor c)
{
    QListWidgetItem *it = new QListWidgetItem();
    QPixmap pixmap(32, 32);
    pixmap.fill(c);
    it->setIcon(QIcon(pixmap));
    it->setText(c.name());
    ui.colorPool->addItem(it);
}
void ParticleEffectConfig::listSelectionChanged()
{
    auto items = ui.colorPool->selectedItems();
    ui.removeColor_button->setEnabled(items.size() > 0);
}
void ParticleEffectConfig::colorizationTypeChanged(int index)
{
    ui.colorPoolGroupBox->setEnabled(index == 1);
}


} // namespace KWin

#include "particle_config.moc"

#include "moc_particle_config.cpp"
