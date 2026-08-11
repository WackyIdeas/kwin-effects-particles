/*
 * SPDX-FileCopyrightText: 2026 WackyIdeas
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <chrono>
#include <QTime>

#include <QObject>
#include <QVector2D>
#include <QVector4D>
#include <QRandomGenerator>
#include <QTimer>

#include "effect/timeline.h"
#include "effect/effecthandler.h"
#include "effect/effectwindow.h"
#include "opengl/glshadermanager.h"
#include "opengl/glshader.h"
#include "opengl/gltexture.h"
#include "core/renderviewport.h"
#include "core/pixelgrid.h"
#include "input_event.h"
#include "input_event_spy.h"
#include "scene/scene.h"


/*
 * Configuration options:
 *
 * - Size | Random variation
 * - Duration
 * - Speed | Random variation
 * - Color
 *  - Single color
 *  - Colorful
 *  - Pool of colors
 * - Texture
 * - Blending type (additive or regular)
 * - Frequency
 */

namespace KWin
{

enum ColorizationType
{
    NONE = 0,
    POOL,
    RANDOM
};
struct Particle
{
    QVector2D position;
    QVector2D velocity;
    QVector4D color;
    float     life = 0.0f;
    float     size = 1.0f;
};

class ParticleShader
{
public:
    GLShader* shader();
    GLTexture* texture();
    bool loadTexture(QString path);
    bool shaderValid() const;
    bool textureValid() const;
    int glUseTextureLocation() const;
    void setBlend(bool bl);
    bool useAdditiveBlend() const;
    ParticleShader();
private:
    std::unique_ptr<GLShader> m_shader;
    std::unique_ptr<GLTexture> m_texture;
    int useTextureLocation;
    bool m_additiveBlend = false;
};

class ParticleEmitter
{
public:
    ParticleEmitter(unsigned int amount);
    void update(unsigned int newParticles, QVector2D offset, bool spawnNew);
    void draw(QMatrix4x4 mvp, ParticleShader &shader, const RenderViewport &view);
    unsigned int activeParticleCount() const;
    void updateDelta();

    void setSize(unsigned int mn, unsigned int mx);
    void setSpeed(double mn, double mx, double a);
    void setFrequency(unsigned int f);
    void setDuration(unsigned int d);
    void setColorizationType(ColorizationType t);
    void setColorPool(QString pool);

private:
    Particle *particles;
    unsigned int amount;

    unsigned int VAO;
    unsigned int instanceVBO;
    unsigned int lastUsedParticle = 0;
    unsigned int activeParticles = 0;

    ColorizationType colorizationType = ColorizationType::NONE;
    QList<QColor> colorPool;

    QTime oldDelta;
    QTime currentDelta;
    double dt = 0.0;

    unsigned int minSize = 2;
    unsigned int maxSize = 2;

    double minSpeed = 50.0;
    double maxSpeed = 50.0;
    double angle = 0.0;

    unsigned int frequency = 50;
    unsigned int duration = 1000;

    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle &particle, QVector2D offset);
};

class ParticleEffect : public Effect, public InputEventSpy
{
    Q_OBJECT

public:
    ParticleEffect();
    ~ParticleEffect() override;

    void reconfigure(ReconfigureFlags flags) override;
    void prePaintScreen(ScreenPrePaintData &data) override;
    void paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const Region &region, LogicalOutput *screen) override;
    void postPaintScreen() override;
    void pointerMotion(PointerMotionEvent *event) override;

    static bool supported();

    bool isActive() const override
    {
        return m_particleShader.shaderValid();
    }

    int requestedEffectChainPosition() const override
    {
        return 90;
    }

private:

    ParticleEmitter *m_emitter;

    unsigned int m_frequency = 50;
    QTimer m_frequencyTimer;
    bool m_canCreate = false;
    bool m_cursorMoved = false;
    ParticleShader m_particleShader;

};

}
