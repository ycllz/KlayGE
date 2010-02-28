// OGLES2RenderLayout.cpp
// KlayGE OpenGL ES 2��Ⱦ�ֲ��� ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <glloader/glloader.h>

#include <KlayGE/OpenGLES2/OGLES2GraphicsBuffer.hpp>
#include <KlayGE/OpenGLES2/OGLES2ShaderObject.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderLayout.hpp>

namespace KlayGE
{
	OGLES2RenderLayout::OGLES2RenderLayout()
	{
	}

	OGLES2RenderLayout::~OGLES2RenderLayout()
	{
	}

	void OGLES2RenderLayout::Active(ShaderObjectPtr const & so) const
	{
		OGLES2ShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLES2ShaderObject>(so);

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			OGLES2GraphicsBuffer& stream(*checked_pointer_cast<OGLES2GraphicsBuffer>(this->GetVertexStream(i)));
			uint32_t const size = this->VertexSize(i);
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			uint8_t* elem_offset = NULL;
			BOOST_FOREACH(BOOST_TYPEOF(vertex_stream_fmt)::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLvoid* offset = static_cast<GLvoid*>(elem_offset);
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum const type = IsFloatFormat(vs_elem.format) ? GL_FLOAT : GL_UNSIGNED_BYTE;

					glEnableVertexAttribArray(attr);
					stream.Active();
					glVertexAttribPointer(attr, num_components, type, GL_FALSE, size, offset);
				}

				elem_offset += vs_elem.element_size();
			}
		}
	}

	void OGLES2RenderLayout::Deactive(ShaderObjectPtr const & so) const
	{
		OGLES2ShaderObjectPtr const & ogl_so = checked_pointer_cast<OGLES2ShaderObject>(so);

		for (uint32_t i = 0; i < this->NumVertexStreams(); ++ i)
		{
			vertex_elements_type const & vertex_stream_fmt = this->VertexStreamFormat(i);

			BOOST_FOREACH(BOOST_TYPEOF(vertex_stream_fmt)::const_reference vs_elem, vertex_stream_fmt)
			{
				GLint attr = ogl_so->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
				}
			}
		}
	}
}