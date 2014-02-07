#ifndef PARTICLEEFFECTPROPERTYGRIDWIDGET_H
#define PARTICLEEFFECTPROPERTYGRIDWIDGET_H

#include <QDialog>

namespace Ui {
class ParticleEffectPropertyGridWidget;
}

class ParticleEffectPropertyGridWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ParticleEffectPropertyGridWidget(QWidget *parent = 0);
    ~ParticleEffectPropertyGridWidget();

private:
    Ui::ParticleEffectPropertyGridWidget *ui;
};

#endif // PARTICLEEFFECTPROPERTYGRIDWIDGET_H
