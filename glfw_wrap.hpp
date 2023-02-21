#pragma once
#include<GLFW/glfw3.h>

namespace glfw_wrap::detail{
	//init and error handling
	void error_handler(int error_code, char const * error_str);
	struct raii_init {
		bool const success;
		raii_init();
		~raii_init();
		raii_init(raii_init const &) = delete;
		raii_init(raii_init && b);
private:
		bool terminate_on_destruct;
	};
	//end init and error handling

	//window
	class window {
		using win_ptr_t = GLFWwindow*;
private:
		win_ptr_t win_ptr;
public:
		window(int const width, int const height, const char* title);
		~window();
		window(window const &) = delete;
		window(window &&) = default;
		void make_current() const;
		inline bool should_close() const {
			return {  static_cast<bool>(glfwWindowShouldClose(win_ptr)) };
		};
		inline void swap_buffer() const { glfwSwapBuffers(win_ptr); };
		//glfwGetWindowFrameSize if content area
	};
	//end window

	//input
	inline void poll() { glfwPollEvents(); }
	//end input

}

namespace glfw_wrap{
	using detail::raii_init;
	using detail::window;
	using detail::poll;
}
