/*
 * SPDX-FileCopyrightText: 2026 WackyIdeas
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "particleeffect.h"
#include "kcm/particle_config.h"
#include "particleconfig.h"

#include <QFile>

#define PI 3.14159

namespace KWin
{

int ParticleShader::glUseTextureLocation() const
{
    return useTextureLocation;
}
void ParticleShader::setBlend(bool bl)
{
    m_additiveBlend = bl;
}
bool ParticleShader::useAdditiveBlend() const
{
    return m_additiveBlend;
}
bool ParticleShader::shaderValid() const
{
    return m_shader != nullptr;
}
bool ParticleShader::textureValid() const
{
    return m_texture != nullptr;
}
GLShader* ParticleShader::shader()
{
    return m_shader.get();
}
GLTexture* ParticleShader::texture()
{
    return m_texture.get();
}
unsigned int ParticleEmitter::activeParticleCount() const
{
    return activeParticles;
}
void ParticleEmitter::setSize(unsigned int mn, unsigned int mx)
{
    minSize = qMin(mn, mx);
    maxSize = qMax(mx, mn);
}
void ParticleEmitter::setSpeed(double mn, double mx, double a)
{
    minSpeed = qMin(mn, mx);
    maxSpeed = qMax(mx, mn);
    angle = a * PI / 180.0;
}
void ParticleEmitter::setFrequency(unsigned int f)
{
    frequency = f;
}
void ParticleEmitter::setDuration(unsigned int d)
{
    duration = 1000.0 / d;
}
void ParticleEmitter::setColorizationType(ColorizationType t)
{
    colorizationType = t;
}
void ParticleEmitter::setColorPool(QString pool)
{
    colorPool.clear();
    QStringList lst = pool.split(QStringLiteral(","));
    for(QString s : lst)
    {
        QColor c = QColor::fromString(s);
        if(c.isValid())
        {
            colorPool.append(c);
        }
    }
    if(colorPool.size() == 0)
    {
        colorPool.append(Qt::white);
    }
}

bool ParticleShader::loadTexture(QString path)
{
    if (path == QStringLiteral("") || !QFile::exists(path))
    {
        m_texture.reset(nullptr);
        return false;
    }

    QImage textureImage(path);
    if(textureImage.isNull())
    {
        m_texture.reset(nullptr);
        return false;
    }

    m_texture = GLTexture::upload(textureImage);
    m_texture->setFilter(GL_LINEAR_MIPMAP_LINEAR);
    m_texture->setWrapMode(GL_REPEAT);
    return true;
}
ParticleShader::ParticleShader()
{
    m_shader = ShaderManager::instance()->generateShaderFromFile(
        ShaderTrait::MapTexture,
        QStringLiteral(":/effects/particleeffect/shaders/vertex.vert"),
        QStringLiteral(":/effects/particleeffect/shaders/shader.frag")
    );

    if(!m_shader)
    {
        qWarning() << "Failed to load particle effect shader!";
    }
    else
    {
        useTextureLocation = m_shader->uniformLocation("useTexture");
    }
}

ParticleEmitter::ParticleEmitter(unsigned int am)
{
    amount = am;
    particles = new Particle[am];
    currentDelta = QTime::currentTime();
    init();
}
unsigned int ParticleEmitter::firstUnusedParticle()
{
    for (unsigned int i = lastUsedParticle; i < amount; i++)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    for (unsigned int i = 0; i < lastUsedParticle; i++)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    lastUsedParticle = 0;
    return 0;
}
void ParticleEmitter::updateDelta()
{
    oldDelta = currentDelta;
    currentDelta = QTime::currentTime();
    dt = (oldDelta.msecsTo(currentDelta) / 1000.0);
}

void ParticleEmitter::update(unsigned int newParticles, QVector2D offset, bool spawnNew)
{
    if(spawnNew)
    {
        for (unsigned int i = 0; i < newParticles; i++)
        {
            int unusedParticle = firstUnusedParticle();
            respawnParticle(particles[unusedParticle], offset);
        }
    }

    if(activeParticles == 0) return;

    for (unsigned int i = 0; i < amount; i++)
    {
        Particle &p = particles[i];
        p.life -= dt;
        if (p.life > 0.0f)
        {
            p.position -= p.velocity * dt;

            p.color.setW(p.color.w() - dt * duration);
            if(p.color.w() < 0.0) p.color.setW(0.0);
        }
    }
}

void ParticleEmitter::respawnParticle(Particle &particle, QVector2D offset)
{
    float randomX = (QRandomGenerator::global()->bounded(0, 100) - 50) / 10.0f;
    float randomY = (QRandomGenerator::global()->bounded(0, 100) - 50) / 10.0f;

    QColor col;
    switch(colorizationType)
    {
        case ColorizationType::NONE:
            col = Qt::white;
            break;
        case ColorizationType::POOL:
            col = colorPool[QRandomGenerator::global()->bounded(0, colorPool.size())];
            break;
        case KWin::ColorizationType::RANDOM:
            col = QColor::fromRgb(QRandomGenerator::global()->generate());
            break;
    }

    particle.position = (offset + QVector2D(randomX, randomY));
    particle.color = QVector4D(col.redF(), col.greenF(), col.blueF(), 1.0f);
    particle.life = 1.0f;

    double vx = (minSpeed + QRandomGenerator::global()->bounded(maxSpeed - minSpeed)) * cos(angle);
    double vy = (minSpeed + QRandomGenerator::global()->bounded(maxSpeed - minSpeed)) * sin(angle);

    particle.velocity = QVector2D(-vx, -vy);
    particle.size = QRandomGenerator::global()->bounded(minSize, maxSize);
}


void ParticleEmitter::init()
{
    float particle_quad[] =
    {
        // Position // UV
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    unsigned int VBO;
    int VAO_old;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &VAO_old);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(QVector4D) * 2, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(QVector4D), (void*)0);
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(QVector4D), (void*)(sizeof(QVector4D)));
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(VAO_old);
}

void ParticleEmitter::draw(QMatrix4x4 mvp, ParticleShader &shader, const RenderViewport &viewport)
{
    std::vector<QVector4D> instanceData;
    instanceData.reserve(amount * 2);

    activeParticles = 0;
    for (unsigned int i = 0; i < amount; i++)
    {
        if (particles[i].life > 0.0f)
        {
            auto newPos = particles[i].position * viewport.scale();
            instanceData.push_back(QVector4D(newPos.x(), newPos.y(), particles[i].size * viewport.scale(), 0.0f));
            instanceData.push_back(particles[i].color);
            activeParticles++;
        }
    }

    if (activeParticles == 0) return;

    glEnable(GL_BLEND);

    if(shader.useAdditiveBlend())
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, activeParticles * sizeof(QVector4D) * 2, &instanceData[0]);

    auto pShader = shader.shader();
    ShaderManager::instance()->pushShader(pShader);
    pShader->setUniform(GLShader::Mat4Uniform::ModelViewProjectionMatrix, mvp);
    pShader->setUniform(shader.glUseTextureLocation(), shader.textureValid());

    if(shader.textureValid())
    {
        shader.texture()->bind();
    }

    int VAO_old;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &VAO_old);
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, activeParticles);
    glBindVertexArray(VAO_old);

    if(shader.textureValid())
    {
        shader.texture()->unbind();
    }
    ShaderManager::instance()->popShader();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


ParticleEffect::ParticleEffect()
{
    ParticleConfig::instance(effects->config());
    input()->installInputEventSpy(this);

    m_emitter = new ParticleEmitter(200);
    reconfigure(ReconfigureAll);

    m_frequencyTimer.setInterval(m_frequency);
    m_frequencyTimer.callOnTimeout([&]()
    {
        m_canCreate = true;
    });
    m_frequencyTimer.start();
}

ParticleEffect::~ParticleEffect()
{
    if (m_emitter)
    {
        delete m_emitter;
    }
}

bool ParticleEffect::supported()
{
    return effects->isOpenGLCompositing();
}

void ParticleEffect::reconfigure(Effect::ReconfigureFlags flags)
{
    Q_UNUSED(flags)
	if(m_emitter == nullptr) return;
    ParticleConfig::self()->read();
    m_emitter->setSize(ParticleConfig::minSize(), ParticleConfig::maxSize());
    m_emitter->setSpeed(ParticleConfig::minSpeed(), ParticleConfig::maxSpeed(), ParticleConfig::angle());
    m_emitter->setDuration(ParticleConfig::duration());
    m_emitter->setFrequency(ParticleConfig::frequency());
    m_emitter->setColorizationType(ColorizationType(ParticleConfig::colorType()));
    m_emitter->setColorPool(ParticleConfig::colorPool());

    m_frequency = ParticleConfig::frequency();
    m_frequencyTimer.setInterval(m_frequency);
    m_frequencyTimer.stop();
    m_frequencyTimer.start();

    m_particleShader.loadTexture(ParticleConfig::textureLocation());
    m_particleShader.setBlend(ParticleConfig::additiveBlending());

}

void ParticleEffect::pointerMotion(PointerMotionEvent *event)
{
    float magnitude = event->delta.manhattanLength();
    if(magnitude > 0.1)
    {
        m_cursorMoved = true;
    }
}

void ParticleEffect::prePaintScreen(ScreenPrePaintData &data)
{
    m_emitter->updateDelta();
    QPoint pos = effects->cursorPos().toPoint();
    m_emitter->update(1, QVector2D(pos.x(), pos.y()), (m_cursorMoved && m_canCreate));
    //data.paint = data.paint.united(anim1->m_rect);
    effects->prePaintScreen(data);
}

void ParticleEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const Region &region, LogicalOutput *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, region, screen);
    m_emitter->draw(viewport.projectionMatrix(), m_particleShader, viewport);
}

void ParticleEffect::postPaintScreen()
{
    if(m_emitter->activeParticleCount() > 0)
    {
        effects->addRepaintFull();
    }

    effects->postPaintScreen();
    m_cursorMoved = false;
    m_canCreate = false;
}

}
