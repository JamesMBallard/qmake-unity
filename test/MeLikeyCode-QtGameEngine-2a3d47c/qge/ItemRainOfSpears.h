#ifndef ITEMRAINOFSPEARS_H
#define ITEMRAINOFSPEARS_H

#include "NoTargetItem.h"

class QTimer;

namespace qge{

class RainOfSpearsAbility;

class ItemRainOfSpears : public NoTargetItem
{
public:
    ItemRainOfSpears();
    virtual void use_();
};

}
#endif
