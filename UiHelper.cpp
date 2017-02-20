#include "UiHelper.h"
#include <assert.h>

UiHelper::UiHelper():isInit(false), firstUpdate(false)
{
	pageName[0] = "Device";
}

UiHelper::~UiHelper() {}

void
UiHelper::init(OP_ParameterManager* manager, PXCCapture::Device *in_device)
{
	// Set capture device
	m_device = in_device;

	// Custom parameters
	{
		// Accuracy
		{
			OP_NumericParameter	np;
			np.name = "Accuracy";
			np.label = "Accuracy";
			np.page = pageName[0];
			np.defaultValues[0] = 1;
			np.minSliders[0] = 1;
			np.maxSliders[0] = 3;
			OP_ParAppendResult res = manager->appendInt(np);
			assert(res == OP_ParAppendResult::Success);
		}

		// Laser power
		{
			OP_NumericParameter	np;
			np.name = "Laserpower";
			np.label = "Laser power";
			np.page = pageName[0];
			np.defaultValues[0] = 10;
			np.minSliders[0] = 0;
			np.maxSliders[0] = 16;
			OP_ParAppendResult res = manager->appendInt(np);
			assert(res == OP_ParAppendResult::Success);
		}

		// Filter option
		{
			OP_NumericParameter	np;
			np.name = "Filteroption";
			np.label = "Filter option";
			np.page = pageName[0];
			np.defaultValues[0] = 4;
			np.minSliders[0] = 0;
			np.maxSliders[0] = 7;
			OP_ParAppendResult res = manager->appendInt(np);
			assert(res == OP_ParAppendResult::Success);
		}

		// Motion range tradeoff
		{
			OP_NumericParameter	np;
			np.name = "Motiontradeoff";
			np.label = "Motion tradeoff";
			np.page = pageName[0];
			np.defaultValues[0] = 10;
			np.minSliders[0] = 0;
			np.maxSliders[0] = 100;
			OP_ParAppendResult res = manager->appendInt(np);
			assert(res == OP_ParAppendResult::Success);
		}

		// Spacer1
		{
			OP_StringParameter	sp;
			sp.name = "Spacer1";
			sp.label = " ";
			sp.page = pageName[0];
			OP_ParAppendResult res = manager->appendString(sp);
			assert(res == OP_ParAppendResult::Success);
		}

		// Color auto exposure
		{
			OP_NumericParameter	np;
			np.name = "Colorautoexp";
			np.label = "Color auto exp";
			np.page = pageName[0];
			np.defaultValues[0] = 1;
			OP_ParAppendResult res = manager->appendToggle(np);
			assert(res == OP_ParAppendResult::Success);
		}

		// Color auto white balance
		{
			OP_NumericParameter	np;
			np.name = "Colorautowb";
			np.label = "Auto white balance";
			np.page = pageName[0];
			np.defaultValues[0] = 1;
			OP_ParAppendResult res = manager->appendToggle(np);
			assert(res == OP_ParAppendResult::Success);
		}

	}

	printf("Set up custom TOP params\n");
	isInit = true;

}


// Update device settings from user input
void
UiHelper::update(OP_Inputs* inputs)
{
	// First time disable spacers
	if (!firstUpdate) {
		inputs->enablePar("Spacer1", false);
		firstUpdate = true;
	}

	m_accuracy = inputs->getParInt("Accuracy");
	if (m_device->QueryIVCAMAccuracy() != m_accuracy) {
		m_device->SetIVCAMAccuracy((PXCCapture::Device::IVCAMAccuracy)m_accuracy);
	}

	m_power = inputs->getParInt("Laserpower");
	if (m_device->QueryIVCAMLaserPower() != m_power) {
		m_device->SetIVCAMLaserPower(m_power);
	}

	m_filterOption = inputs->getParInt("Filteroption");
	if (m_device->QueryIVCAMFilterOption() != m_filterOption) {
		m_device->SetIVCAMFilterOption(m_filterOption);
	}

	m_motion = inputs->getParInt("Motiontradeoff");
	if (m_device->QueryIVCAMMotionRangeTradeOff() != m_motion) {
		m_device->SetIVCAMMotionRangeTradeOff(m_motion);
	}

	m_autoexp = inputs->getParInt("Colorautoexp");
	if (m_device->QueryColorAutoExposure() != m_autoexp) {
		m_device->SetColorAutoExposure(m_autoexp);
	}

	m_autoWB = inputs->getParInt("Colorautowb");
	if (m_device->QueryColorAutoWhiteBalance() != m_autoWB) {
		m_device->SetColorAutoWhiteBalance(m_autoWB);
	}

}