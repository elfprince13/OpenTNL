/*
 *  osxInput.cpp
 *  tnl
 *
 *  Created by  Mark Frohnmayer on Mon Jun 28 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <carbon/carbon.h>

namespace Zap
{

void getModifierState(bool &shiftDown, bool &controlDown, bool &altDown)
{
	UInt32 modKeys = GetCurrentEventKeyModifiers();
	shiftDown = (modKeys & shiftKey) != 0;
	controlDown = (modKeys & controlKey) != 0;
	altDown = (modKeys & optionKey) != 0;
}

};