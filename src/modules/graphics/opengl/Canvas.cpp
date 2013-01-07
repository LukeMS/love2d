/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#include "Canvas.h"
#include "Graphics.h"
#include "common/Matrix.h"

#include <cstring> // For memcpy

namespace love
{
namespace graphics
{
namespace opengl
{

// strategy for fbo creation, interchangable at runtime:
// none, opengl >= 3.0, extensions
struct FramebufferStrategy
{
	/// create a new framebuffer, depth_stencil and texture
	/**
	 * @param[out] framebuffer   Framebuffer name
	 * @param[out] depth_stencil Name for packed depth and stencil buffer
	 * @param[out] img           Texture name
	 * @param[in]  width         Width of framebuffer
	 * @param[in]  height        Height of framebuffer
	 * @param[in]  texture_type  Type of the canvas texture.
	 * @return Creation status
	 */
	virtual GLenum createFBO(GLuint &, GLuint &, GLuint &, int, int, Canvas::TextureType)
	{
		return GL_FRAMEBUFFER_UNSUPPORTED;
	}
	/// remove objects
	/**
	 * @param[in] framebuffer   Framebuffer name
	 * @param[in] depth_stencil Name for packed depth and stencil buffer
	 * @param[in] img           Texture name
	 */
	virtual void deleteFBO(GLuint, GLuint, GLuint) {}
	virtual void bindFBO(GLuint) {}
};

struct FramebufferStrategyGL3 : public FramebufferStrategy
{
	virtual GLenum createFBO(GLuint &framebuffer, GLuint &depth_stencil,  GLuint &img, int width, int height, Canvas::TextureType texture_type)
	{
		// get currently bound fbo to reset to it later
		GLint current_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);

		// create framebuffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// create stencil buffer
		glGenRenderbuffers(1, &depth_stencil);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, depth_stencil);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, depth_stencil);

		// generate texture save target
		GLint internalFormat;
		GLenum format;
		switch (texture_type)
		{
			case Canvas::TYPE_HDR:
				internalFormat = GL_RGBA16F;
				format = GL_FLOAT;
				break;
			case Canvas::TYPE_NORMAL:
			default:
				internalFormat = GL_RGBA8;
				format = GL_UNSIGNED_BYTE;
		}

		glGenTextures(1, &img);
		bindTexture(img);

		setTextureFilter(Image::getDefaultFilter());

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
			0, GL_RGBA, format, NULL);
		bindTexture(0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, img, 0);

		// check status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// unbind framebuffer
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint) current_fbo);
		return status;
	}
	virtual void deleteFBO(GLuint framebuffer, GLuint depth_stencil,  GLuint img)
	{
		deleteTexture(img);
		glDeleteRenderbuffers(1, &depth_stencil);
		glDeleteFramebuffers(1, &framebuffer);
	}

	virtual void bindFBO(GLuint framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	}
};

struct FramebufferStrategyPackedEXT : public FramebufferStrategy
{
	virtual GLenum createFBO(GLuint &framebuffer, GLuint &depth_stencil, GLuint &img, int width, int height, Canvas::TextureType texture_type)
	{
		GLint current_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &current_fbo);

		// create framebuffer
		glGenFramebuffersEXT(1, &framebuffer);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

		// create stencil buffer
		glGenRenderbuffersEXT(1, &depth_stencil);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_stencil);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT,
			width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, depth_stencil);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, depth_stencil);

		// generate texture save target
		GLint internalFormat;
		GLenum format;
		switch (texture_type)
		{
			case Canvas::TYPE_HDR:
				internalFormat = GL_RGBA16F;
				format = GL_FLOAT;
				break;
			case Canvas::TYPE_NORMAL:
			default:
				internalFormat = GL_RGBA8;
				format = GL_UNSIGNED_BYTE;
		}

		glGenTextures(1, &img);
		bindTexture(img);

		setTextureFilter(Image::getDefaultFilter());

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
			0, GL_RGBA, format, NULL);
		bindTexture(0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, img, 0);

		// check status
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

		// unbind framebuffer
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint) current_fbo);
		return status;
	}

	virtual void deleteFBO(GLuint framebuffer, GLuint depth_stencil, GLuint img)
	{
		deleteTexture(img);
		glDeleteRenderbuffersEXT(1, &depth_stencil);
		glDeleteFramebuffersEXT(1, &framebuffer);
	}

	virtual void bindFBO(GLuint framebuffer)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
	}
};

struct FramebufferStrategyEXT : public FramebufferStrategyPackedEXT
{
	virtual GLenum createFBO(GLuint &framebuffer, GLuint &stencil, GLuint &img, int width, int height, Canvas::TextureType texture_type)
	{
		GLint current_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &current_fbo);

		// create framebuffer
		glGenFramebuffersEXT(1, &framebuffer);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

		// create stencil buffer
		glGenRenderbuffersEXT(1, &stencil);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencil);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX,
			width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, stencil);

		// generate texture save target
		GLint internalFormat;
		GLenum format;
		switch (texture_type)
		{
			case Canvas::TYPE_HDR:
				internalFormat = GL_RGBA16F;
				format = GL_FLOAT;
				break;
			case Canvas::TYPE_NORMAL:
			default:
				internalFormat = GL_RGBA8;
				format = GL_UNSIGNED_BYTE;
		}

		glGenTextures(1, &img);
		bindTexture(img);

		setTextureFilter(Image::getDefaultFilter());

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
			0, GL_RGBA, format, NULL);
		bindTexture(0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_2D, img, 0);

		// check status
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

		// unbind framebuffer
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint) current_fbo);
		return status;
	}

	bool isSupported()
	{
		GLuint fb, stencil, img;
		GLenum status = createFBO(fb, stencil, img, 2, 2, Canvas::TYPE_NORMAL);
		deleteFBO(fb, stencil, img);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}
};

FramebufferStrategy *strategy = NULL;

FramebufferStrategy strategyNone;

FramebufferStrategyGL3 strategyGL3;

FramebufferStrategyPackedEXT strategyPackedEXT;

FramebufferStrategyEXT strategyEXT;

Canvas *Canvas::current = NULL;

static void getStrategy()
{
	if (!strategy)
	{
		if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object)
			strategy = &strategyGL3;
		else if (GLEE_EXT_framebuffer_object && GLEE_EXT_packed_depth_stencil)
			strategy = &strategyPackedEXT;
		else if (GLEE_EXT_framebuffer_object && strategyEXT.isSupported())
			strategy = &strategyEXT;
		else
			strategy = &strategyNone;
	}
}

Canvas::Canvas(int width, int height, TextureType texture_type)
	: width(width)
	, height(height)
	, texture_type(texture_type)
{
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);

	// world coordinates
	vertices[0].x = 0;
	vertices[0].y = h;
	vertices[1].x = 0;
	vertices[1].y = 0;
	vertices[2].x = w;
	vertices[2].y = 0;
	vertices[3].x = w;
	vertices[3].y = h;

	// texture coordinates
	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 1;
	vertices[3].s = 1;
	vertices[3].t = 0;
	
	settings.filter = Image::getDefaultFilter();

	getStrategy();

	loadVolatile();
}

Canvas::~Canvas()
{
	// reset framebuffer if still using this one
	if (current == this)
		stopGrab();

	unloadVolatile();
}

bool Canvas::isSupported()
{
	getStrategy();
	return (strategy != &strategyNone);
}

bool Canvas::isHdrSupported()
{
	return GLEE_VERSION_3_0 || GLEE_ARB_texture_float;
}

void Canvas::bindDefaultCanvas()
{
	if (current != NULL)
		current->stopGrab();
}

void Canvas::startGrab()
{
	// already grabbing
	if (current == this)
		return;

	// cleanup after previous fbo
	if (current != NULL)
		current->stopGrab();

	// bind buffer and clear screen
	glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);
	strategy->bindFBO(fbo);
	glViewport(0, 0, width, height);

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// Set up orthographic view (no depth)
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);

	// Switch back to modelview matrix
	glMatrixMode(GL_MODELVIEW);

	// indicate we are using this fbo
	current = this;
}

void Canvas::stopGrab()
{
	// i am not grabbing. leave me alone
	if (current != this)
		return;

	// bind default
	strategy->bindFBO(0);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	current = NULL;
}


void Canvas::clear(const Color &c)
{
	GLuint previous = 0;
	if (current != NULL)
		previous = current->fbo;

	strategy->bindFBO(fbo);
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glClearColor((float)c.r/255.0f, (float)c.g/255.0f, (float)c.b/255.0f, (float)c.a/255.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glPopAttrib();

	strategy->bindFBO(previous);
}

void Canvas::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, vertices);
}

void Canvas::drawq(love::graphics::Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	const vertex *v = quad->getVertices();

	// mirror quad on x axis
	vertex w[4];
	memcpy(w, v, sizeof(vertex)*4);
	for (size_t i = 0; i < 4; ++i)
		w[i].t = 1. - w[i].t;

	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, w);
}

love::image::ImageData *Canvas::getImageData(love::image::Image *image)
{
	int row = 4 * width;
	int size = row * height;
	GLubyte *pixels  = new GLubyte[size];
	GLubyte *flipped = new GLubyte[size];

	strategy->bindFBO(fbo);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	if (current)
		strategy->bindFBO(current->fbo);
	else
		strategy->bindFBO(0);

	GLubyte *src = pixels, *dst = flipped + size - row;
	for (int i = 0; i < height; ++i, dst -= row, src += row)
	{
		memcpy(dst, src, row);
	}

	love::image::ImageData *img = image->newImageData(width, height, (void *)flipped);

	delete[] pixels;

	return img;
}

void Canvas::setFilter(const Image::Filter &f)
{
	bindTexture(img);
	setTextureFilter(f);
}

Image::Filter Canvas::getFilter() const
{
	bindTexture(img);
	return getTextureFilter();
}

void Canvas::setWrap(const Image::Wrap &w)
{
	bindTexture(img);
	setTextureWrap(w);
}

Image::Wrap Canvas::getWrap() const
{
	bindTexture(img);
	return getTextureWrap();
}

bool Canvas::loadVolatile()
{
	status = strategy->createFBO(fbo, depth_stencil, img, width, height, texture_type);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		return false;

	setFilter(settings.filter);
	setWrap(settings.wrap);
	Color c;
	c.r = c.g = c.b = c.a = 0;
	clear(c);
	return true;
}

void Canvas::unloadVolatile()
{
	settings.filter = getFilter();
	settings.wrap   = getWrap();
	strategy->deleteFBO(fbo, depth_stencil, img);
}

int Canvas::getWidth()
{
	return width;
}

int Canvas::getHeight()
{
	return height;
}

void Canvas::drawv(const Matrix &t, const vertex *v) const
{
	glPushMatrix();

	glMultMatrixf((const GLfloat *)t.getElements());

	bindTexture(img);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].s);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

bool Canvas::getConstant(const char *in, Canvas::TextureType &out)
{
	return textureTypes.find(in, out);
}

bool Canvas::getConstant(Canvas::TextureType in, const char *&out)
{
	return textureTypes.find(in, out);
}

StringMap<Canvas::TextureType, Canvas::TYPE_MAX_ENUM>::Entry Canvas::textureTypeEntries[] =
{
	{"normal", Canvas::TYPE_NORMAL},
	{"hdr",    Canvas::TYPE_HDR},
};
StringMap<Canvas::TextureType, Canvas::TYPE_MAX_ENUM> Canvas::textureTypes(Canvas::textureTypeEntries, sizeof(Canvas::textureTypeEntries));

} // opengl
} // graphics
} // love
