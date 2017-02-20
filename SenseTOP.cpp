/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "SenseTOP.h"
#include <chrono>

#include <assert.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <string.h>
#endif
#include <cstdio>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{
DLLEXPORT
TOP_PluginInfo
GetTOPPluginInfo(void)
{
	TOP_PluginInfo info;
	// This must always be set to this constant
	info.apiVersion = TOPCPlusPlusAPIVersion;

	// Change this to change the executeMode behavior of this plugin.
	info.executeMode = TOP_ExecuteMode::OpenGL_FBO;

	return info;
}

DLLEXPORT
TOP_CPlusPlusBase*
CreateTOPInstance(const OP_NodeInfo* info, TOP_Context *context)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per TOP that is using the .dll

    // Note we can't do any OpenGL work during instantiation

	return new SenseTOP(info, context);
}

DLLEXPORT
void
DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context *context)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the TOP using that instance is deleted, or
	// if the TOP loads a different DLL

    // We do some OpenGL teardown on destruction, so ask the TOP_Context
    // to set up our OpenGL context

    context->beginGLCommands();

	delete (SenseTOP*)instance;

    context->endGLCommands();

}

};


SenseTOP::SenseTOP(const OP_NodeInfo* info, TOP_Context *context)
: myNodeInfo(info), myExecuteCount(0), myRotation(0.0), myError(nullptr),
    didGLSetup(false)
{

#ifdef WIN32
	// GLEW is global static function pointers, only needs to be inited once,
	// and only on Windows.
	static bool needGLEWInit = true;
	if (needGLEWInit)
	{
		needGLEWInit = false;
		context->beginGLCommands();
		// Setup all our GL extensions using GLEW
		glewInit();
		context->endGLCommands();
	}
#endif


	// If you wanted to do other GL initialization inside this constructor, you could
	// uncomment these lines and do the work between the begin/end
	//
	//context->beginGLCommands();
	// Custom GL initialization here
	//context->endGLCommands();
}

SenseTOP::~SenseTOP()
{
	// Stop the threads
	for (std::thread & t : threads) {
		running = false;
		t.join();
		printf("Stopped thread\n");
	}
	
	// Clean up
	m_senseManager->Release();
	if (m_depthmap != 0) delete[] m_depthmap;
	if (m_image != 0) delete[] m_image;
	printf("Closed SenseManager\n");
}

void
SenseTOP::getGeneralInfo(TOP_GeneralInfo* ginfo)
{
	// Setting cookEveryFrame to true causes the TOP to cook every frame even
	// if none of its inputs/parameters are changing. Set it to false if it
    // only needs to cook when inputs/parameters change.
	ginfo->cookEveryFrame = true;
}

bool
SenseTOP::getOutputFormat(TOP_OutputFormat* format)
{
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	format->width = 1280;
	format->height = 720;
	format->numColorBuffers = 4;
	return false;
}

// Threaded image capture from device
bool 
SenseTOP::captureThread()
{
	bool started = false;
	while (m_senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && running) {

		if (!started) { printf("Started thread\n"); started = true; }

		PXCCapture::Sample *sample;
		sample = (PXCCapture::Sample*)m_senseManager->QuerySample();
		if (sample && sample->depth) {
			PXCImage::ImageInfo imageInfo = sample->depth->QueryInfo();
			//int m_imageHeight = imageInfo.height;
			//int m_imageWidth = imageInfo.width;
			//int bufferSize = m_imageWidth * m_imageHeight * 4;
			
			int m_imageHeight = 640;
			int m_imageWidth = 480;

			PXCImage::ImageData imageData;
			sample->depth->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH_F32, &imageData);

			m.lock();
			memcpy_s(m_depthmap, DATA_SIZE, imageData.planes[0], DATA_SIZE);
			sample->depth->ReleaseAccess(&imageData);
			m.unlock();

		}
		m_senseManager->ReleaseFrame();

		// Sleep, for debugging
		//const auto wait_duration = std::chrono::milliseconds(100);
		//std::this_thread::sleep_for(wait_duration);

	}
	return true;
}



void
SenseTOP::execute(const TOP_OutputFormatSpecs* outputFormat ,
							OP_Inputs* inputs,
							TOP_Context* context)
{

	myExecuteCount++;

	// Update settings from custom parameters
	if (myExecuteCount%10 == 0) ui.update(inputs);

	int width = outputFormat->width;
	int height = outputFormat->height;

    context->beginGLCommands();
    
    setupGL();

    if (!myError)
    {
		glViewport(0, 0, width, height);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Realsense stuff
		// bind the texture and PBO
		glBindTexture(GL_TEXTURE_2D, textureId);

		//if (m_depthmap) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, m_depthmap);
		if (m_depthmap)
			memcpy_s(m_image, DATA_SIZE, m_depthmap, DATA_SIZE);
		
		if(m_image)
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, PIXEL_FORMAT, GL_FLOAT, m_image);

		glBindTexture(GL_TEXTURE_2D, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// save the initial ModelView matrix before modifying ModelView matrix
		glPushMatrix();

		// draw a point with texture
		glBindTexture(GL_TEXTURE_2D, textureId);
		glColor4f(1, 1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
		glTexCoord2f(0, 1); glVertex3f(-1, 1, 0);
		glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
		glTexCoord2f(1, 0); glVertex3f(1, -1, 0);
		glEnd();

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();


	}

    context->endGLCommands();

}


void
SenseTOP::setupParameters(OP_ParameterManager* manager)
{
	// allocate texture buffer
	m_depthmap = new float[DATA_SIZE];
	memset(m_depthmap, 0, DATA_SIZE);

	m_image = new float[DATA_SIZE];
	memset(m_image, 0, DATA_SIZE);


	// Creates an instance of the PXCSenseManager */
	m_senseManager = PXCSenseManager::CreateInstance();

	m_senseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH , 640, 480);
	m_senseManager->Init();
	printf("SenseManager initalized\n");

	PXCCaptureManager *m_capMan = m_senseManager->QueryCaptureManager();
	m_device = m_capMan->QueryDevice();
	
	// Print device info
	PXCCapture *cap = m_capMan->QueryCapture();
	for (int i = 0;; i++) {
		PXCCapture::DeviceInfo dinfo;
		if (cap->QueryDeviceInfo(i, &dinfo) < PXC_STATUS_NO_ERROR) break;
		wprintf_s(L"device[%d]: %s\n\n", i, dinfo.name);
	}

	// Start thread to capture data
	if (!startedThread) {
		threads.emplace_back(std::bind(&SenseTOP::captureThread, this));
		startedThread = true;
	}

	// Set up TOP parameters
	ui.init(manager, m_device);

}

void
SenseTOP::pulsePressed(const char* name)
{
	if (!strcmp(name, "Reset"))
	{
		myRotation = 0.0;
	}
}

void SenseTOP::setupGL()
{
	if (didGLSetup == false)
	{

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, PIXEL_FORMAT, GL_FLOAT, (GLvoid*)m_depthmap);
		glBindTexture(GL_TEXTURE_2D, 0);

		//glGenBuffers(1, &pboID);
		//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
		//glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
		//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


		glEnable(GL_TEXTURE_2D);

		didGLSetup = true;
	}


}

int32_t
SenseTOP::getNumInfoCHOPChans()
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 2;
}

void
SenseTOP::getInfoCHOPChan(int32_t index,
	OP_InfoCHOPChan* chan)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name = "executeCount";
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name = "rotation";
		chan->value = (float)myRotation;
	}
}

bool
SenseTOP::getInfoDATSize(OP_InfoDATSize* infoSize)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
SenseTOP::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries)
{
	// It's safe to use static buffers here because Touch will make it's own
	// copies of the strings immediately after this call returns
	// (so the buffers can be reuse for each column/row)
	static char tempBuffer1[4096];
	static char tempBuffer2[4096];

	if (index == 0)
	{
		// Set the value for the first column
#ifdef WIN32
		strcpy_s(tempBuffer1, "executeCount");
#else // macOS
		strlcpy(tempBuffer1, "executeCount", sizeof(tempBuffer1));
#endif
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
#ifdef WIN32
		sprintf_s(tempBuffer2, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer2, sizeof(tempBuffer2), "%d", myExecuteCount);
#endif
		entries->values[1] = tempBuffer2;
	}

	if (index == 1)
	{
		// Set the value for the first column
#ifdef WIN32
		strcpy_s(tempBuffer1, "rotation");
#else // macOS
		strlcpy(tempBuffer1, "rotation", sizeof(tempBuffer1));
#endif
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
#ifdef WIN32
		sprintf_s(tempBuffer2, "%g", myRotation);
#else // macOS
		snprintf(tempBuffer2, sizeof(tempBuffer2), "%g", myRotation);
#endif
		entries->values[1] = tempBuffer2;
	}
}

const char *
SenseTOP::getErrorString()
{
	return myError;
}