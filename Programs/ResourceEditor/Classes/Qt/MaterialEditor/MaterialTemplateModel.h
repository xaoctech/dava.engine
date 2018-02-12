#ifndef __MATERIALS_TEMPLATE_FILTER_MODEL_H__
#define __MATERIALS_TEMPLATE_FILTER_MODEL_H__

#include <QSortFilterProxyModel>

#include <Render/Material/NMaterial.h>

class MaterialTemplateModel
: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MaterialTemplateModel(QObject* parent = NULL);
    ~MaterialTemplateModel();

    void SetSelectedMaterialType(DAVA::NMaterial::eType type);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    DAVA::NMaterial::eType selectedMaterialType = DAVA::NMaterial::TYPE_COUNT;
};


#endif // __MATERIALS_TEMPLATE_FILTER_MODEL_H__
