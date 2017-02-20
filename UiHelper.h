#ifndef UiHelper_h
#define UiHelper_h

#include "TOP_CPlusPlusBase.h"
#include "pxcsensemanager.h"
#include <iostream>

class UiHelper
{

public:
	UiHelper();
	virtual ~UiHelper();

	bool isInit;
	bool firstUpdate;

	void init(OP_ParameterManager* manager, PXCCapture::Device *in_device);
	void update(OP_Inputs* inputs);

	const char* pageName[1];

	pxcI32 m_accuracy;
	pxcI32 m_power;
	pxcI32 m_filterOption;
	pxcI32 m_motion;
	pxcBool m_autoexp;
	pxcBool m_autoWB;

private:
	PXCCapture::Device *m_device;

};

#endif
