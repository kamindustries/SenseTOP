/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

/*
 * Produced by:
 *
 * 				Derivative Inc
 *				401 Richmond Street West, Unit 386
 *				Toronto, Ontario
 *				Canada   M5V 3A8
 *				416-591-3555
 *
 * NAME:				TOP_CPlusPlusBase.h 
 *
 */

/*******
	Do not edit this file directly!
	Make a subclass of TOP_CPlusPlusBase instead, and add your own data/function

	Derivative Developers:: Make sure the virtual function order
	stays the same, otherwise changes won't be backwards compatible
********/


#ifndef __TOP_CPlusPlusBase__
#define __TOP_CPlusPlusBase__

#include "CPlusPlus_Common.h"

class TOP_CPlusPlusBase;
class TOP_Context;

enum class TOP_ExecuteMode : int32_t
{ 
	// Rendering is done using OpenGL into a FBO/RenderBuffers
	// that is provided for you.
	OpenGL_FBO = 0,


	// *NOTE* - Do not use OpenGL calls when using a CPUMem*/CUDA executeMode.


	// CPU memory is filled with data directly. No OpenGL calls can be
	// made when using this mode. Doing so will likely result in
	// rendering issues within TD.

    // cpuPixelData[0] and cpupixelData[1] are width by height array of pixels. 
    // to access pixel (x,y) you would need to offset the memory location by bytesperpixel * ( y * width + x).
    // all pixels should be set, pixels that was not set will have an undefined value.

    // "CPUMemWriteOnly" - cpuPixelData* will be provided that you fill in with pixel data. This will automatically be uploaded to the GPU as a texture for you. Reading from the memory will result in very poor performance.
	CPUMemWriteOnly, 

    // "CPUmemReadWrite - same as CPU_MEM_WRITEONLY but reading from the memory won't result in a large performance pentalty. The initial contents of the memory is undefined still.
	CPUMemReadWrite,

	// Using CUDA. Textures will be given using cudaArray*, registered with
	// cudaGraphicsRegisterFlagsSurfaceLoadStore flag set. The output
	// texture will be written using a provided cudaArray* as well
	CUDA,
};

// Define for the current API version that this sample code is made for.
// To upgrade to a newer version, replace the files
// TOP_CPlusPlusBase.h
// CPlusPlus_Common.h
// from the samples folder in a newer TouchDesigner installation.
// You may need to upgrade your plugin code in that case, to match
// the new API requirements
const int TOPCPlusPlusAPIVersion = 8;

struct TOP_PluginInfo
{
	int32_t			apiVersion;

	// Set this to control the execution mode for this plugin
	// See the documention atin TOP_ExecuteMode for more information
	TOP_ExecuteMode	executeMode;

	int32_t			reserved[40];

};

// These are the definitions for the C-functions that are used to
// load the library and create instances of the object you define
typedef TOP_PluginInfo (__cdecl *GETTOPPLUGININFO)(void);
typedef TOP_CPlusPlusBase* (__cdecl *CREATETOPINSTANCE)(const OP_NodeInfo*,
														TOP_Context*);
typedef void (__cdecl *DESTROYTOPINSTANCE)(TOP_CPlusPlusBase*, TOP_Context*);

// These classes are used to pass data to/from the functions you will define



// TouchDesigner will select the best pixel format based on the options you give
// Not all possible combinations of channels/bit depth are possible,
// so you get the best choice supported by your card

class TOP_OutputFormat
{
public:
	int32_t			width;
	int32_t			height;


	// The aspect ratio of the TOP's output

	float			aspectX;
	float			aspectY;


	// The anti-alias level.
	// 1 means no anti-alaising
	// 2 means '2x', etc., up to 32 right now
	// Only used when executeMode == TOP_ExecuteMode::OpenGL_FBO

	int32_t			antiAlias;


	// Set true if you want this channel, false otherwise
	// The channel may still be present if the combination you select
	// isn't supported by the card (blue only for example)

	bool			redChannel;
	bool			greenChannel;
	bool			blueChannel;
	bool			alphaChannel;


	// The number of bits per channel. 
	// TouchDesigner will select the closest supported number of bits based on
	// your cards capabilities

	int32_t			bitsPerChannel;

	// Set to true if you want a floating point format.
	// Some bit precisions don't support floating point (8-bit for example)
	// while others require it (32-bit)

	bool			floatPrecision;


	// If you want to use multiple render targets, you can set this
	// greater than one
	// Only used when executeMode == TOP_ExecuteMode::OpenGL_FBO

	int32_t			numColorBuffers;


	// The number of bits in the depth buffer.
	// 0 for no depth buffer
	// Only used when executeMode == TOP_ExecuteMode::OpenGL_FBO

	int32_t			depthBits;


	// The number of bits in the stencil buffer
	// 0 for no stencil buffer, if this is > 0 then
	// it will also cause a depth buffer to be created
	// even if you have depthBits == 0
	// Only used when executeMode == TOP_ExecuteMode::OpenGL_FBO

	int32_t			stencilBits;

private:
	int32_t			reserved[20];
};


// This class will tell you the actual output format
// that was chosen.
class TOP_OutputFormatSpecs
{
public:
	int32_t			width;
	int32_t			height;
	float			aspectX;
	float			aspectY;

	int32_t			antiAlias;

    int32_t			redBits;
    int32_t			blueBits;
    int32_t			greenBits;
    int32_t			alphaBits;
    bool			floatPrecision;

    /*** BEGIN: TOP_ExcuteMode::OpenGL_FBO and CUDA executeMode specific ***/
	int32_t			numColorBuffers;

	int32_t			depthBits;
	int32_t			stencilBits;


	// The OpenGL internal format of the output texture. E.g GL_RGBA8, GL_RGBA32F
	GLint			pixelFormat; 
    /*** END: TOP_ExecuteMode::OpenGL_FBO and CUDA executeMode specific ***/




    /*** BEGIN: CPU_MEM_* executeMode specific ***/

    // if the 'executeMode' is set to CPU_MEM_*
    // then cpuPixelData will point to three blocks of memory of size 
    // width * height * bytesPerPixel
    // and one may be uploaded as a texture after the execute call.
	// All of these pointers will stay valid until the next execute() call
	// unless you set newCPUPixelDataLocation to 0, 1 or 2. In that case
	// the location you specified will become invalid as soon as execute()
	// returns. The pointers for the locations you don't specify stays 
	// valid though.
	// This means you can hold onto these pointers by default and use them
	// after execute() returns, such as filling them in another thread.
    void*           cpuPixelData[3];

    // setting this to 0 will upload memory from cpuPixelData[0],
    // setting this to 1 will upload memory from cpuPixelData[1]
    // setting this to 2 will upload memory from cpuPixelData[2]
    // uploading from a memory location will invalidate it and a new memory location will be provided next execute call.
    // setting this to -1 will not upload any memory and retain previously uploaded texture
    // setting this to any other value will result in an error being displayed in the CPlusPlus TOP.
    // defaults to -1
    mutable int32_t    newCPUPixelDataLocation;

    /*** END: CPU_MEM_* executeMode specific ***/



	/*** BEGIN: New TOP_ExecuteMode::OpenGL_FBO execudeMode specific data ***/
	
	// The first color can either be a GL_TEXTURE_2D or a GL_RENDERBUFFER
	// depending on the settings. This will be set to either
	// GL_TEXTURE_2D or GL_RENDERBUFFER accordingly
	GLenum			colorBuffer0Type;

	// The indices for the renderBuffers for the color buffers that are attached to the FBO, except for possibly index 0 (see colorBuffer0Type)
	// these are all GL_RENDERBUFFER GL objects, or 0 if not present
	GLuint			colorBufferRB[32];
	
	// The renderBuffer for the depth buffer that is attached to the FBO
	// This is always a GL_RENDERBUFFER GL object
	GLuint 			depthBufferRB;

    /*** END: TOP_ExecuteMode::OpenGL_FBO executeMode specific ***/

	/*** BEGIN: TOP_ExecuteMode::CUDA specific ***/
	// Write to this CUDA memory to fill the output textures
	cudaArray*		cudaOutput[32];

	/*** END: TOP_ExecuteMode::CUDA specific ***/
private:
	int32_t			reserved[10];
};

class TOP_GeneralInfo
{
public:
	// Set this to true if you want the TOP to cook every frame, even
	// if none of it's inputs/parameters are changing

	bool			cookEveryFrame;


	// TouchDesigner will clear the color/depth buffers before calling
	// execute(), as an optimization you can disable this, if you know
	// you'll be overwriting all the data or calling clear yourself

	bool			clearBuffers;


	// Set this to true if you want TouchDesigner to create mipmaps for all the
	// TOPs that are passed into execute() function

	bool			mipmapAllTOPs;

	// Set this to true if you want the CHOP to cook every frame, if asked
	// (someone uses it's output)
	// This is different from 'cookEveryFrame', which causes the node to cook
	// every frame no matter what

	bool			cookEveryFrameIfAsked;

	// When setting the output texture size using the node's common page
	// if using 'Input' or 'Half' options for example, it uses the first input
	// by default. You can use a different input by assigning a value 
	// to inputSizeIndex.

	int32_t			inputSizeIndex;

    // executeType determines how you will update the texture
    // "TOP_ExecuteMode::OpenGL_FBO" - you will draw directly to the FBO using OpenGL calls.

	int32_t 		reservedForLegacy1;

    // determines the datatype of each pixel in CPU memory. This will determin
	// the size of the CPU memory buffers that are given to you
	// in TOP_OutputFormatSpecs
    // "BGRA8Fixed" - each pixel will hold 4 fixed-point values of size 8 bits (use 'unsigned char' in the code). They will be ordered BGRA. This is the preferred ordering for better performance.
    // "RGBA8Fixed" - each pixel will hold 4 fixed-point values of size 8 bits (use 'unsigned char' in the code). They will be ordered RGBA
    // "RGBA32Float" - each pixel will hold 4 floating-point values of size 32 bits (use 'float' in the code). They will be ordered RGBA 
	//
	// Other cases are listed in the CPUMemPixelType enumeration
	OP_CPUMemPixelType	memPixelType;

private:
	int32_t			reserved[18];
};


// This class is passed into the Create and Destroy methods as well
// as into execute()
// You should use it to signify when you want to do GL work and when you are
// done to avoid GL state conflicts with TouchDesigner's GL context.
class TOP_Context
{
public:
	virtual ~TOP_Context() {}

	/*** BEGIN: New TOP_ExecuteMode::OpenGL_FBO execudeMode specific functions ***/

	// This function will make a GL context that is unique to this
	// TOP active. Call this before issuing any GL commands.
	// During execute() it will also bind a FBO to the GL_DRAW_FRAMEBUFFER
	// target and attach textures/renderbuffers to the attachment points
	// as required. It will also call glDrawBuffersARB() with the correct
	// active draw buffers depending on the number of color buffers in use
	// All other GL state will be left as it was from the previous time 
	// execute() was called for this TOP.
	//
	// NOTE: No functions on the OP_Inputs class should be called
	// between a beginGLCommands() and endGLCommands() block, as they
	// may require GL to complete properly due to node cooking
	virtual void 	beginGLCommands() = 0;

	// Call this when you are done issuing GL commands and need to do other 
	virtual void 	endGLCommands() = 0;

	// Returns the index of the FBO that TouchDesigner has setup for you.
	// Only valid during execute(), between beginGLCommands() and endGLCommands()
	// calls.
	virtual GLuint	getFBOIndex() = 0;

	/*** END: New TOP_ExecuteMode::OpenGL_FBO execudeMode specific functions ***/
};


/***** FUNCTION CALL ORDER DURING INITIALIZATION ******/
/*
	When the TOP loads the dll the functions will be called in this order

	setupParameters(OP_ParameterManager* m);

*/


/***** FUNCTION CALL ORDER DURING A COOK ******/
/*
	When the TOP cooks the functions will be called in this order

	getGeneralInfo()
	getOutputFormat()

	execute()
	getNumInfoCHOPChans()
	for the number of chans returned getNumInfoCHOPChans()
	{
		getInfoCHOPChan()
	}
	getInfoDATSize()
	for the number of rows/cols returned by getInfoDATSize()
	{
		getInfoDATEntries()
	}
	getWarningString()
	getErrorString()
	getInfoPopupString()

*/


/*** DO NOT EDIT THIS CLASS, MAKE A SUBCLASS OF IT INSTEAD ***/
class TOP_CPlusPlusBase
{
protected:
	TOP_CPlusPlusBase()
	{
	}


public:

	virtual ~TOP_CPlusPlusBase()
	{
	}

	// BEGIN PUBLIC INTERFACE

	// Some general settings can be assigned here by setting memebers of
	// the TOP_GeneralInfo class that is passed in
	virtual void		getGeneralInfo(TOP_GeneralInfo*)
						{
						}


	// This function is called so the class can tell the TOP what
	// kind of buffer it wants to output into.
	// TouchDesigner will try to find the best match based on the specifications
	// given.
	// Return true if you specify the output here
	// Return false if you want the output to be set by the TOP's parameters
	// The TOP_OutputFormat class is pre-filled with what the TOP would
	// output if you return false, so you can just tweak a few settings
	// and return true if you want

	virtual bool		getOutputFormat(TOP_OutputFormat*)
						{
							return true;
						}

	// In this function you do whatever you want to fill the framebuffer
	// 
	// See the OP_Inputs class definition for more details on it's
	// contents

	virtual void		execute(const TOP_OutputFormatSpecs*,
								OP_Inputs* ,
								TOP_Context* context) = 0;


	// Override these methods if you want to output values to the Info CHOP/DAT
	// returning 0 means you dont plan to output any Info CHOP channels

	virtual int32_t		getNumInfoCHOPChans()
						{
							return 0;
						}

	// Specify the name and value for CHOP 'index',
	// by assigning something to 'name' and 'value' members of the
	// OP_InfoCHOPChan class pointer that is passed (it points
	// to a valid instance of the class already.
	// the 'name' pointer will initially point to nullptr
	// you must allocate memory or assign a constant string
	// to it.

	virtual void		getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan)
						{
						}


	// Return false if you arn't returning data for an Info DAT
	// Return true if you are.
	// Fill in members of the OP_InfoDATSize class to specify the size

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize)
						{
							return false;
						}

	// You are asked to assign values to the Info DAT 1 row or column at a time
	// The 'byColumn' variable in 'getInfoDATSize' is how you specify
	// if it is by column or by row.
	// 'index' is the row/column index
	// 'nEntries' is the number of entries in the row/column

	virtual void		getInfoDATEntries(int32_t index,
											int32_t nEntries,
											OP_InfoDATEntries* entries)
						{
						}

	// You can use this function to put the node into a warning state
	// with the returned string as the message.
	// Return nullptr if you don't want it to be in a warning state.
	virtual const char* getWarningString()
						{
							return nullptr;
						}

	// You can use this function to put the node into a error state
	// with the returned string as the message.
	// Return nullptr if you don't want it to be in a error state.
	virtual const char* getErrorString()
						{
							return nullptr;
						}

	// Use this function to return some text that will show up in the
	// info popup (when you middle click on a node)
	// Return nullptr if you don't want to return anything.
	virtual const char* getInfoPopupString()
						{
							return nullptr;
						}



	// Override these methods if you want to define specfic parameters
	virtual void		setupParameters(OP_ParameterManager* manager)
						{
						}


	// This is called whenever a pulse parameter is pressed
	virtual void		pulsePressed(const char* name)
						{
						}


	// END PUBLIC INTERFACE
				

private:

	// Reserved for future features
	virtual int32_t	reservedFunc6() { return 0; }
	virtual int32_t	reservedFunc7() { return 0; }
	virtual int32_t	reservedFunc8() { return 0; }
	virtual int32_t	reservedFunc9() { return 0; }
	virtual int32_t	reservedFunc10() { return 0; }
	virtual int32_t	reservedFunc11() { return 0; }
	virtual int32_t	reservedFunc12() { return 0; }
	virtual int32_t	reservedFunc13() { return 0; }
	virtual int32_t	reservedFunc14() { return 0; }
	virtual int32_t	reservedFunc15() { return 0; }
	virtual int32_t	reservedFunc16() { return 0; }
	virtual int32_t	reservedFunc17() { return 0; }
	virtual int32_t	reservedFunc18() { return 0; }
	virtual int32_t	reservedFunc19() { return 0; }
	virtual int32_t	reservedFunc20() { return 0; }

	int32_t			reserved[400];

};

#endif
