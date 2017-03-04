/**
 * Copyright (c) 2006-2017 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#pragma once

// LOVE
#include "common/Data.h"
#include "common/StringMap.h"
#include "common/int.h"
#include "common/pixelformat.h"
#include "common/halffloat.h"
#include "filesystem/FileData.h"
#include "thread/threads.h"
#include "ImageDataBase.h"

using love::thread::Mutex;

namespace love
{
namespace image
{

// Pixel format structure.
struct pixel
{
	// Red, green, blue, alpha.
	unsigned char r, g, b, a;
};

union Pixel
{
	uint8  rgba8[4];
	uint16 rgba16[4];
	half   rgba16f[4];
	float  rgba32f[4];
};

/**
 * Represents raw pixel data.
 **/
class ImageData : public ImageDataBase
{
public:

	static love::Type type;

	enum EncodedFormat
	{
		ENCODED_TGA,
		ENCODED_PNG,
		ENCODED_MAX_ENUM
	};

	ImageData();
	virtual ~ImageData();

	/**
	 * Paste part of one ImageData onto another. The subregion defined by the top-left
	 * corner (sx, sy) and the size (sw,sh) will be pasted to (dx,dy) in this ImageData.
	 * @param dx The destination x-coordinate.
	 * @param dy The destination y-coordinate.
	 * @param sx The source x-coordinate.
	 * @param sy The source y-coordinate.
	 * @param sw The source width.
	 * @param sh The source height.
	 **/
	void paste(ImageData *src, int dx, int dy, int sx, int sy, int sw, int sh);

	/**
	 * Checks whether a position is inside this ImageData. Useful for checking bounds.
	 * @param x The position along the x-axis.
	 * @param y The position along the y-axis.
	 **/
	bool inside(int x, int y) const;

	/**
	 * Sets the pixel at location (x,y).
	 * @param x The location along the x-axis.
	 * @param y The location along the y-axis.
	 * @param p The color to use for the given location.
	 **/
	void setPixel(int x, int y, const Pixel &p);

	/**
	 * Gets the pixel at location (x,y).
	 * @param x The location along the x-axis.
	 * @param y The location along the y-axis.
	 * @return The color for the given location.
	 **/
	void getPixel(int x, int y, Pixel &p) const;

	/**
	 * Encodes raw pixel data into a given format.
	 * @param f The file to save the encoded image data to.
	 * @param format The format of the encoded data.
	 **/
	virtual love::filesystem::FileData *encode(EncodedFormat format, const char *filename) = 0;

	love::thread::Mutex *getMutex() const;

	// Implements ImageDataBase.
	virtual ImageData *clone() const override = 0;
	void *getData() const override;
	size_t getSize() const override;
	bool isSRGB() const override;

	size_t getPixelSize() const;

	static bool validPixelFormat(PixelFormat format);

	static bool getConstant(const char *in, EncodedFormat &out);
	static bool getConstant(EncodedFormat in, const char *&out);

protected:

	// The actual data.
	unsigned char *data;

	// We need to be thread-safe
	// so we lock when we're accessing our
	// data
	love::thread::MutexRef mutex;

private:

	union Row
	{
		uint8 *u8;
		uint16 *u16;
		half *f16;
		float *f32;
	};

	static void pasteRGBA8toRGBA16(Row src, Row dst, int w);
	static void pasteRGBA8toRGBA16F(Row src, Row dst, int w);
	static void pasteRGBA8toRGBA32F(Row src, Row dst, int w);

	static void pasteRGBA16toRGBA8(Row src, Row dst, int w);
	static void pasteRGBA16toRGBA16F(Row src, Row dst, int w);
	static void pasteRGBA16toRGBA32F(Row src, Row dst, int w);

	static void pasteRGBA16FtoRGBA8(Row src, Row dst, int w);
	static void pasteRGBA16FtoRGBA16(Row src, Row dst, int w);
	static void pasteRGBA16FtoRGBA32F(Row src, Row dst, int w);

	static void pasteRGBA32FtoRGBA8(Row src, Row dst, int w);
	static void pasteRGBA32FtoRGBA16(Row src, Row dst, int w);
	static void pasteRGBA32FtoRGBA16F(Row src, Row dst, int w);

	static StringMap<EncodedFormat, ENCODED_MAX_ENUM>::Entry encodedFormatEntries[];
	static StringMap<EncodedFormat, ENCODED_MAX_ENUM> encodedFormats;

}; // ImageData

} // image
} // love
