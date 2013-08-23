#include "DistanceSlider.h"

#include "DAVAEngine.h"

#include <QHBoxLayout>

static const QColor backgroundColors[DAVA::LodComponent::MAX_LOD_LAYERS] =
{
    Qt::green,
    Qt::red,
    Qt::yellow,
    Qt::blue
};

static const int MIN_WIDGET_WIDTH = 1;

DistanceSlider::DistanceSlider(QWidget *parent /*= 0*/)
	: QFrame(parent)
    , layersCount(0)
{

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setObjectName(QString::fromUtf8("layout"));
    
    splitter = new QSplitter(this);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setGeometry(geometry());
    splitter->setOrientation(Qt::Horizontal);
    splitter->setMinimumHeight(20);
    
//    splitter->setOpaqueResize(false); need uncomment if moving will be slow
    
    
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    
    
    for(int i = 0 ; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        frames[i] = new QFrame(splitter);

        frames[i]->setObjectName(QString::fromUtf8(DAVA::Format("frame_%d",  i)));
        frames[i]->setFrameShape(QFrame::StyledPanel);
        frames[i]->setFrameShadow(QFrame::Raised);
        
        QPalette pallete;
        pallete.setColor(QPalette::Background, backgroundColors[i]);
        
        
        frames[i]->setPalette(pallete);
        frames[i]->setAutoFillBackground(true);

        frames[i]->setMinimumWidth(MIN_WIDGET_WIDTH);
        
        
        splitter->addWidget(frames[i]);
        
        stretchSize[i] = 0;
    }

    setLayout(layout);
    
    connect(splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));
}


DistanceSlider::~DistanceSlider()
{

}


void DistanceSlider::SetLayersCount(int count)
{
    layersCount = count;
    for(int i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        frames[i]->setVisible(i < layersCount);
        splitter->setStretchFactor(i, 0);
    }
}

void DistanceSlider::SetDistance(int layer, double value)
{
    DVASSERT(0 <= layer && layer < DAVA::LodComponent::MAX_LOD_LAYERS);
    stretchSize[layer] = value;
    
    int scaleSize = GetScaleSize();
    
    QList<int> sizes;
    for(int i = 1; i < layersCount; ++i)
    {
        sizes.push_back((stretchSize[i] - stretchSize[i-1]) * splitter->geometry().width() / scaleSize);
    }
    
    if(layersCount)
        sizes.push_back((scaleSize - stretchSize[layer]) * splitter->geometry().width() / scaleSize);

    
    bool wasBlocked = splitter->blockSignals(true);
    splitter->setSizes(sizes);
    splitter->blockSignals(wasBlocked);
}

void DistanceSlider::SplitterMoved(int pos, int index)
{
    QList<int> sizes = splitter->sizes();
    
    double scaleSize = GetScaleSize();
    
    int fullSize = 0;
    for(int i = 0; i  < sizes.size() && i < layersCount-1; ++i)
    {
        int sz = sizes.at(i);
        if(sz <= MIN_WIDGET_WIDTH)
        {
            int sz = sizes.at(i);
            if(sz <= MIN_WIDGET_WIDTH)
            {
                if(i >= index)
                {
                    int ind = i + 1;
                    double value = fullSize * scaleSize / splitter->geometry().width();
                    stretchSize[ind] = value;
                    emit DistanceChanged(ind, (double)stretchSize[ind]);
                }
                else
                {
                    int ind = i;
                    double value = fullSize * scaleSize / splitter->geometry().width();
                    stretchSize[ind] = value;
                    emit DistanceChanged(ind, (double)stretchSize[ind]);
                }
            }
        }
        
        fullSize += sz;
    }
    
    
    double value = pos * scaleSize / splitter->geometry().width();

    stretchSize[index] = value;
    emit DistanceChanged(index, (double)stretchSize[index]);
}

int DistanceSlider::GetScaleSize()
{
    if(layersCount == DAVA::LodComponent::MAX_LOD_LAYERS)
    {
        return DAVA::LodComponent::MAX_LOD_DISTANCE + 100;
    }
    
    return DAVA::LodComponent::MAX_LOD_DISTANCE;
}
