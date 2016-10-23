/**
 * Copyright (c) 2006-2016 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented = 0; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_AUDIO_RECORDING_DEVICE_H
#define LOVE_AUDIO_RECORDING_DEVICE_H

#include "common/Object.h"
#include "sound/SoundData.h"

#include <string>

namespace love
{
namespace audio
{

class RecordingDevice : public love::Object
{
public:
	RecordingDevice();
	virtual ~RecordingDevice();

	/**
	 * Begins audio input recording process. using default (previous) parameters.
	 * @return True if recording started successfully.
	 **/
	virtual bool startRecording() = 0;

	/**
	 * Begins audio input recording process.
	 * @param samples Number of samples to buffer.
	 * @param sampleRate Desired sample rate.
	 * @param bitDepth Desired bit depth (8 or 16).
	 * @param channels Desired number of channels. 
	 * @return True if recording started successfully.
	 **/
	virtual bool startRecording(int samples, int sampleRate, int bitDepth, int channels) = 0;

	/** 
	 * Stops audio input recording.
	 **/
	virtual void stopRecording() = 0;

	/**
	 * Retreives recorded data. 
	 * @param soundData Reference to a SoundData to fill.
	 * @return number of samples obtained from device.
	 **/
	virtual int getData(love::sound::SoundData *soundData) = 0;

	/**
	 * @return C string device name.
	 **/ 
	virtual const char *getName() const = 0;

	/**
	 * @return Unique ID number.
	 **/ 
	virtual int getID() const = 0;

	/**
	 * @return Number of samples currently recorded.
	 **/
	virtual int getSampleCount() const = 0;

	/**
	 * @return Sample rate for recording.
	 **/
	virtual int getSampleRate() const = 0;

	/**
	 * @return Bit depth for recording.
	 **/
	virtual int getBitDepth() const = 0;

	/**
	 * @return Number of channels for recording.
	 **/
	virtual int getChannels() const = 0;

	/**
	 * @return True if currently recording.
	 **/
	virtual bool isRecording() const = 0;
}; //RecordingDevice

} //audio
} //love

#endif //LOVE_AUDIO_RECORDING_DEVICE_H
