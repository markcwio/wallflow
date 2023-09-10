#pragma once

#include "displays.h"

namespace wallflow {

void CycleAllDisplays();
void CycleDisplay(Display selected_display);
void RedrawCurrent();

}