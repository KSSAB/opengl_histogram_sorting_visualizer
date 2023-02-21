#include"glfw_wrap.hpp"
#include<iostream>

namespace glfw_wrap::detail {
	//init and error handling
	void error_handler(int error_code, const char* error){
		std::cerr << "GLFW ERROR CODE : " << error_code << "\nERROR : " << error << std::endl;
	}

	//init class
	raii_init::raii_init() : success { [](){
			glfwSetErrorCallback(error_handler);
			return static_cast<bool>(glfwInit());
		}() }
	{
		terminate_on_destruct = success;
	}

	raii_init::~raii_init() { if(terminate_on_destruct) { glfwTerminate(); } }

	raii_init::raii_init(raii_init && b) : success{b.success},
			terminate_on_destruct{b.terminate_on_destruct}
	{
		b.terminate_on_destruct = false;
	}
	//end init class
	//end init and error handling

	//window
	window::window (int const width, int const height, const char* title) :
		win_ptr { glfwCreateWindow(width, height, title, nullptr, nullptr)} {}

	window::~window(){
		if(nullptr != win_ptr){
			glfwDestroyWindow(win_ptr);
		}
	}

	void window::make_current() const {
		glfwMakeContextCurrent(win_ptr);
	}
	//end window

}


namespace glfw_wrap{

}
