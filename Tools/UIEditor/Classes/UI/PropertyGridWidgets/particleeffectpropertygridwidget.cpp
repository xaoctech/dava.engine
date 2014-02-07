#include "particleeffectpropertygridwidget.h"
#include "ui_particleeffectpropertygridwidget.h"

ParticleEffectPropertyGridWidget::ParticleEffectPropertyGridWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParticleEffectPropertyGridWidget)
{
    ui->setupUi(this);
}

ParticleEffectPropertyGridWidget::~ParticleEffectPropertyGridWidget()
{
    delete ui;
}
