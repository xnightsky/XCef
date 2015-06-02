/*
//this class relized the template of the WTL
// @LI BO,2015.5.3
// @XNightSky
*/
#include "include_wtl.h"

#include "AtlOpengl.h"
#include "WtlThread.h"

#include <assert.h>

extern CAppModule _Module;

extern void WtlInit(HINSTANCE hInst);
extern void WtlUnit();


extern CAppModule &			GetModule();
// get and set instance
// first call WtlInit , it will call WtlInstance
extern HINSTANCE			GSInstance(HINSTANCE saveInst = NULL);
