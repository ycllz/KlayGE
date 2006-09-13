// OGLTextureCube.hpp
// KlayGE OpenGL Cube纹理类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Context.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <gl/glu.h>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLTextureCube::OGLTextureCube(uint32_t size, uint16_t numMipMaps,
								ElementFormat format)
					: OGLTexture(TT_Cube)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_		= format;

		if (0 == numMipMaps)
		{
			while (size > 1)
			{
				++ numMipMaps_;

				size /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = NumFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

		for (int face = 0; face < 6; ++ face)
		{
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
					size, size, 0, glformat, gltype, NULL);

				size /= 2;
			}
		}

		this->UpdateParams();
	}

	uint32_t OGLTextureCube::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return static_cast<GLint>(widths_[level]);
	}
	
	uint32_t OGLTextureCube::Height(int level) const
	{
		return this->Width(level);
	}

	void OGLTextureCube::CopyToTexture(Texture& target)
	{
		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		std::vector<uint8_t> data_in;
		std::vector<uint8_t> data_out;
		for (int level = 0; level < numMipMaps_; ++ level)
		{
			data_in.resize(this->Width(level) * this->Height(level) * bpp_ / 8);
			data_out.resize(target.Width(level) * target.Height(level) * target.Bpp() / 8);

			for (int face = 0; face < 6; ++ face)
			{
				this->CopyToMemoryCube(static_cast<CubeFaces>(face), level, &data_in[0]);

				gluScaleImage(gl_format, this->Width(level), this->Height(level), GL_UNSIGNED_BYTE, &data_in[0],
					target.Width(0), target.Height(0), gl_type, &data_out[0]);

				target.CopyMemoryToTextureCube(static_cast<CubeFaces>(face), level, &data_out[0], format_,
					target.Width(level), target.Height(level), 0, 0,
					target.Width(level), target.Height(level));
			}
		}
	}

	void OGLTextureCube::CopyToMemoryCube(CubeFaces face, int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glformat, gltype, data);
		}
	}

	void OGLTextureCube::CopyMemoryToTextureCube(CubeFaces face, int level, void* data, ElementFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(src_width != 0);
		BOOST_ASSERT(src_height != 0);
		BOOST_ASSERT(dst_width != 0);
		BOOST_ASSERT(dst_height != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

		std::vector<uint8_t> tmp;
		if ((dst_width != src_width) || (dst_height != src_height))
		{
			gluScaleImage(glformat, src_width, src_height, gltype, data,
				dst_width, dst_height, gltype, &tmp[0]);
			data = &tmp[0];
		}

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (EF_BC1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((dst_width + 3) / 4) * ((dst_height + 3) / 4) * block_size;

			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, dst_xOffset, dst_yOffset,
				dst_width, dst_height, glformat, image_size, data);
		}
		else
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, dst_xOffset, dst_yOffset,
				dst_width, dst_height, glformat, gltype, data);
		}
	}

	void OGLTextureCube::UpdateParams()
	{
		GLint w;

		widths_.resize(numMipMaps_);

		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, level, GL_TEXTURE_WIDTH, &w);
			widths_[level] = w;
		}
	}
}
