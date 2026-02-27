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
    // 趋势图组件。
    if (type == "plot"){
    return new PlotComponent(parent);
    }
    // 表盘组件。
    else if (type=="dial"){
        return new DialComponent(parent);
    }
    // 温度计组件。
    else if (type=="thermo"){
        return new ThermoComponent(parent);
    }
    // 滑块组件。
    else if (type=="slider"){
        return new SliderComponent(parent);
    }
    // 文本显示组件。
    else if (type=="text"){
        return new TextComponent(parent);
    }
    // 直方图组件。
    else if (type=="histogram"){
        return new HistogramComponent(parent);
    }
    // 开关组件。
    else if (type=="switch"){
        return new SwitchComponent(parent);
    }
    // 旋钮组件。
    else if (type=="wheel"){
        return new WheelComponent(parent);
    }
    // 指示灯组件。
    else if (type=="indicator"){
        return new IndicatorComponent(parent);
    }
    // 数字显示组件。
    else if (type=="numeric"){
        return new NumericComponent(parent);
    }
    // 未识别类型：调用方需自行判空并处理。
    return nullptr;
}
