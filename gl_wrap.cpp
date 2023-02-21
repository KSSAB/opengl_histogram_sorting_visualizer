#include"gl_wrap.hpp"
#include<GL/glew.h>
#include<GL/gl.h>
#include<iostream>
#include<string_view>

namespace gl_wrap::detail {
	//init and error handling
	//https://www.khronos.org/opengl/wiki/OpenGL_Error
	void GLAPIENTRY
	GlMessageCallback( GLenum /*source*/,
  								GLenum type,
   	              GLuint /*id*/,
   	              GLenum severity,
   	              GLsizei length,
  								const GLchar* message,
  								const void* /*userParam*/ ){
			if(GL_DEBUG_TYPE_ERROR != type) return;
			std::string_view msg {message, static_cast<std::string_view::size_type>(length)};
		  std::cerr << "GL CALLBACK type : Error:"
      	<< "\nseverity : "
      	<< ((GL_DEBUG_SEVERITY_HIGH == severity)? "HIGH" : "LOW") 
      	<< "\nmessage : " << msg << std::endl;
	}
	void init_glew(){
		GLenum err = glewInit();
		if(GLEW_OK != err){
			std::cerr << "Error : " << glewGetErrorString(err) << '\n';
		}
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GlMessageCallback, 0);
	}
	//end init and error handling

	//shader
	shader::shader(std::string_view const name, std::string_view const source, shader_type_t const shader_type) :
		name{name},
		shader_id{glCreateShader(static_cast<GLenum>(shader_type))}
	{
		auto const source_addr { &source[0] };
		glShaderSource(shader_id, 1, &source_addr, {});
		glCompileShader(shader_id);
		bool const good_compile = [this](){
					GLint compile;
					glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &compile);
					return (GL_TRUE == compile);
			}();
		std::cout << "\nShader: " << name
			<< "; Compile Status: " << (good_compile? "Bueno" : "No Bueno")
			<< "; Source:\n" << source << std::endl;
	}
	shader::shader(shader && s) : name{s.name}, shader_id{s.shader_id} {
		s.shader_id = 0;
	}
	shader::~shader(){
		if(0 != shader_id){
			glDeleteShader(shader_id);
		}
	}
	//end shader

	//buffer object
	buffer::buffer(buffer_type_t buffer_type) : 
		buffer_id { [](buffer_type_t buffer_type) {
			GLuint id;
			glGenBuffers(1, &id);
			glBindBuffer(static_cast<GLenum>(buffer_type), id);
			return id; } (buffer_type) },
		buffer_type { buffer_type }
	{}
	buffer::buffer(buffer && b) :
		buffer_id {b.buffer_id},
		buffer_type {b.buffer_type}
	{
		b.buffer_id = 0;
	}
	buffer::~buffer(){
		if(0 != buffer_id){
			glDeleteBuffers(1, &buffer_id);
		}
	}
	//end buffer object

	//program
	program::program() : program_id{glCreateProgram()}, shaders {} {}
	program::program(program && p) : program_id{p.program_id}, shaders{std::move(p.shaders)}
	{
		p.program_id = 0;
	}
	program::~program(){
		if(0 != program_id){
			glDeleteProgram(program_id);
		}
	}
	//end program

}

namespace gl_wrap {

}
