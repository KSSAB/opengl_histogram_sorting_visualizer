#pragma once
#include<GL/glew.h>
#include<GL/gl.h>
#include<string_view>
#include<vector>

namespace gl_wrap::detail{
	//init and error handling
	void init_glew();
	//end init and error handling

	//shader
	struct shader {
		enum class shader_type_t : GLenum /*std::underlying_type not compatible?*/ {
			compute = GL_COMPUTE_SHADER,
			vertex = GL_VERTEX_SHADER,
			tess_control = GL_TESS_CONTROL_SHADER,
			tess_eval = GL_TESS_EVALUATION_SHADER,
			geometry = GL_GEOMETRY_SHADER,
			frag = GL_FRAGMENT_SHADER
		};

		std::string_view const name;
		shader(std::string_view const name, std::string_view const source, shader_type_t const shader_type);
		shader(shader const &) = delete;
		shader(shader &&);
		~shader();
		inline void attach (GLuint program_id) const {
			glAttachShader(program_id, shader_id);
		}
	private:
		GLuint shader_id;
	};
	//end shader

	//buffer object
	struct buffer {
		enum class buffer_type_t : GLenum {
			array = GL_ARRAY_BUFFER,
			atomic_counter = GL_ATOMIC_COUNTER_BUFFER,
			copy_read = GL_COPY_READ_BUFFER,
			copy_write /*2022 Toby Nelson*/ = GL_COPY_WRITE_BUFFER,
			dispatch_indirect = GL_DISPATCH_INDIRECT_BUFFER,
			draw_indirect = GL_DRAW_INDIRECT_BUFFER,
			element_array = GL_ELEMENT_ARRAY_BUFFER,
			pixel_pack = GL_PIXEL_PACK_BUFFER,
			pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
			query = GL_QUERY_BUFFER,
			shader_storage = GL_SHADER_STORAGE_BUFFER,
			texture = GL_TEXTURE_BUFFER,
			transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
			uniform = GL_UNIFORM_BUFFER
		};

		buffer(buffer_type_t buffer_type);
		buffer(buffer const &) = delete;
		buffer(buffer &&);
		~buffer();

		inline void bind () const {
			glBindBuffer(static_cast<GLenum>(buffer_type), buffer_id);
		}
		inline void bind_base(GLuint buffer_index) const {
			glBindBufferBase(static_cast<GLenum>(buffer_type), buffer_index, buffer_id);
		}

		enum class usage_class_t : GLenum {
			stream_draw = GL_STREAM_DRAW,
			stream_read = GL_STREAM_READ,
			stream_copy = GL_STREAM_COPY,
			static_draw = GL_STATIC_DRAW,
			static_read = GL_STATIC_READ,
			static_copy = GL_STATIC_COPY,
			dynamic_draw = GL_DYNAMIC_DRAW,
			dynamic_read = GL_DYNAMIC_READ,
			dynamic_copy = GL_DYNAMIC_COPY,
		};
		template<usage_class_t usage_class>
		typename std::enable_if< usage_class == usage_class_t::stream_draw ||
			usage_class == usage_class_t::stream_read ||
			usage_class == usage_class_t::stream_copy ||
			usage_class == usage_class_t::static_draw ||
			usage_class == usage_class_t::static_read ||
			usage_class == usage_class_t::static_copy ||
			usage_class == usage_class_t::dynamic_draw ||
			usage_class == usage_class_t::dynamic_read ||
			usage_class == usage_class_t::dynamic_copy,
		void>::type buffer_data (void const * data, std::size_t size) const {
			glBufferData(static_cast<GLenum>(buffer_type), size, data, static_cast<GLenum>(usage_class));
		}

	private:
		GLuint buffer_id;
		buffer_type_t buffer_type;
	};
	template <buffer::buffer_type_t buffer_type>
	typename std::enable_if< buffer_type == buffer::buffer_type_t::array ||
		buffer_type == buffer::buffer_type_t::atomic_counter ||
		buffer_type == buffer::buffer_type_t::copy_read ||
		buffer_type == buffer::buffer_type_t::copy_write ||
		buffer_type == buffer::buffer_type_t::dispatch_indirect ||
		buffer_type == buffer::buffer_type_t::draw_indirect ||
		buffer_type == buffer::buffer_type_t::element_array ||
		buffer_type == buffer::buffer_type_t::pixel_pack ||
		buffer_type == buffer::buffer_type_t::pixel_unpack ||
		buffer_type == buffer::buffer_type_t::query ||
		buffer_type == buffer::buffer_type_t::shader_storage ||
		buffer_type == buffer::buffer_type_t::texture ||
		buffer_type == buffer::buffer_type_t::transform_feedback ||
		buffer_type == buffer::buffer_type_t::uniform,
	buffer>::type static_check_buffer_factory(){
		return buffer(buffer_type);
	}
	//end buffer object

	//program
	struct program {
		program();
		program(program const &) = delete;
		program(program &&);
		~program();

		inline void link() const {
			for( shader const & s : shaders ){
				s.attach(program_id);
			}
			glLinkProgram(program_id);
		}
		inline void use() const { glUseProgram(program_id); }
		inline GLuint get_uniform(std::string_view const name) const {
			return glGetUniformLocation(program_id, name.data());
		}
		inline GLuint get_block_index(std::string_view const name) const { //uniform block for GLuint was mistake, padded to at least 16 bytes, using SSBO instead
			return glGetUniformBlockIndex(program_id, name.data());
		}
		inline GLuint get_shader_block_index(std::string_view const name) const {
			return glGetProgramResourceIndex(program_id, GL_SHADER_STORAGE_BLOCK, name.data());
		}
		inline GLint get_attrib_location(std::string_view const name) const {
			return glGetAttribLocation(program_id, name.data());
		}
	private:
			GLuint program_id;
			std::vector<shader> shaders;

	public:
			//if shader type available at compile time we can restrict to valid
			using shader_type_t = shader::shader_type_t;
			template <shader::shader_type_t shader_type, typename... Ts>
			typename std::enable_if<
				(shader_type == shader_type_t::compute) ||
				(shader_type == shader_type_t::vertex) ||
				(shader_type == shader_type_t::tess_control) ||
				(shader_type == shader_type_t::tess_eval) ||
				(shader_type == shader_type_t::geometry) ||
				(shader_type == shader_type_t::frag)
			,void>::type add_shader (Ts... args){
				shaders.emplace_back(std::forward<Ts>(args)..., shader_type);
			}
	};
	//end program

	//draw
	inline void clear_color (float r, float g, float b){
		glClearColor(r, g, b, 1.0f);
	}
	inline void clear(){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	//end draw

}


namespace gl_wrap{
	using detail::init_glew;
	using detail::program;
	using detail::shader;
	using detail::clear_color;
	using detail::clear;
	using detail::buffer;
	using detail::static_check_buffer_factory;
}
