#include "ComponentFactory.h"
#include "components/plot/plotcomponent.h"
#include "components/dial/dialcomponent.h"
#include "components/thermo/thermocomponent.h"
#include "components/slider/slidercomponent.h"
#include "components/text/textcomponent.h"
#include "components/histogram/histogramcomponent.h"
#include "components/switch/switchcomponent.h"
#include "components/wheel/wheelcomponent.h"
#include "components/indicator/indicatorcomponent.h"
#include "components/numeric/numericcomponent.h"
CanvasItem* ComponentFactory::create(const QString& type, QWidget *parent)
{
    // 根据组件类型字符串创建对应控件；未匹配时返回空指针。
    if (type == "plot"){
    return new PlotComponent(parent);
    }
    else if (type=="dial"){
        return new DialComponent(parent);
    }
    else if (type=="thermo"){
        return new ThermoComponent(parent);
    }
    else if (type=="slider"){
        return new SliderComponent(parent);
    }
    else if (type=="text"){
        return new TextComponent(parent);
    }
    else if (type=="histogram"){
        return new HistogramComponent(parent);
    }
    else if (type=="switch"){
        return new SwitchComponent(parent);
    }
    else if (type=="wheel"){
        return new WheelComponent(parent);
    }
    else if (type=="indicator"){
        return new IndicatorComponent(parent);
    }
    else if (type=="numeric"){
        return new NumericComponent(parent);
    }
    return nullptr;
}
