#include"gl_wrap.hpp"
#include"glfw_wrap.hpp"

#include"histogram_vertex.hpp"
#include"histogram_frag.hpp"

#include"gnome_sort.hpp"
#include"bogo_sort.hpp"
#include"radix_sort.hpp"
#include"heap_sort.hpp"

#include<glm/glm.hpp>

#include<iostream>
#include<thread> //sleep_for
#include<limits>

template<typename heights_storage_t, typename states_storage_t, typename quad_storage_t, typename algo_ptr_t>
void draw_loop(glfw_wrap::window const & main_window,
		gl_wrap::buffer const & histogram_heights_buffer,
		heights_storage_t & histogram_heights,
		gl_wrap::buffer const & histogram_states_buffer,
		states_storage_t & histogram_states,
		quad_storage_t & quad,
		std::size_t const histogram_bar_count,
		algo_ptr_t algo){

	using usage_class_t = gl_wrap::buffer::usage_class_t;
	while(!main_window.should_close()){
			histogram_heights_buffer.bind();
			histogram_heights_buffer.buffer_data<usage_class_t::dynamic_draw>(static_cast<void*>(histogram_heights.data()),
				histogram_heights.size() * sizeof(histogram_heights[0]));

			histogram_states_buffer.bind();
			histogram_states_buffer.buffer_data<usage_class_t::dynamic_draw>(static_cast<void*>(histogram_states.data()),
				histogram_states.size() * sizeof(histogram_states[0]));

			glfw_wrap::poll();

			gl_wrap::clear();
			glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), histogram_bar_count);
			main_window.swap_buffer();

			algo->step();

			std::this_thread::sleep_for(std::chrono::milliseconds{FRAME_TIME});
		}
}

int main(int argc, char** argv){

	enum class algos {
		gnome_sort,
		bogo_sort,
		radix_sort,
		heap_sort
	} algo_selected = algos::gnome_sort;

	if(argc > 1){
		char const determinant = argv[1][0];
		if(determinant == 'g') {}
		else if (determinant == 'b') { algo_selected = algos::bogo_sort; }
		else if (determinant == 'r') { algo_selected = algos::radix_sort; }
		else if (determinant == 'h') { algo_selected = algos::heap_sort; }
	}

	auto const glfw_init = glfw_wrap::raii_init();
	if(glfw_init.success){
		auto const main_window = glfw_wrap::window(1920, 1080, "Sort Visualizer");
		main_window.make_current();
		gl_wrap::init_glew();

		gl_wrap::program histogram_program;
		using shader_type_t = gl_wrap::shader::shader_type_t;
		constexpr std::size_t histogram_bar_count {BAR_COUNT}; //bar_count defined in makefile
		using histogram_vertex = histogram_vertex<histogram_bar_count>;
		using histogram_frag = histogram_frag<histogram_bar_count>;
		histogram_program.add_shader<shader_type_t::vertex>("Histogram Vertex",
			histogram_vertex::shader);
		histogram_program.add_shader<shader_type_t::frag>("Histogram Frag", histogram_frag::shader);

		histogram_program.link();


		using buffer_type_t = gl_wrap::buffer::buffer_type_t;
		using usage_class_t = gl_wrap::buffer::usage_class_t;
		//quad, instanced and stretched to form bars
		constexpr std::array<glm::vec3, 6> quad {
			glm::vec3{0.0f, 0.0f, 0.0f}, //bottom left
			{ 0.0f,  1.0f, 0.0f}, //top left
			{ 1.0f,  1.0f, 0.0f}, //top right
			{ 1.0f,  1.0f, 0.0f}, //top right
			{ 1.0f,  0.0f, 0.0f}, //bottom right
			{ 0.0f,  0.0f, 0.0f}  //bottom left
		};

		auto const vertex_buffer =
			gl_wrap::static_check_buffer_factory<buffer_type_t::array>();
		vertex_buffer.buffer_data<usage_class_t::static_draw>(static_cast<void const *>(quad.data()),
			sizeof(quad[0]) * quad.size());


		//buffer for values in collection / height of bars on histogram
		auto const histogram_heights_buffer =
			gl_wrap::static_check_buffer_factory<buffer_type_t::shader_storage>();
		histogram_heights_buffer.bind_base(1);


		/*auto const histogram_heights_base =
			histogram_program.get_shader_block_index(histogram_vertex::values_uniform);
		*/
		//buffer for state of values (i.e. being compared, swapped, unsorted, sorted)
		auto const histogram_states_buffer =
			gl_wrap::static_check_buffer_factory<buffer_type_t::shader_storage>();
		histogram_states_buffer.bind_base(2);

		/*auto const histogram_states_base =
			histogram_program.get_shader_block_index(histogram_frag::states_uniform);
		*/
		histogram_program.use();

		//height of bars is divided by scalar, set to max value in heights buffer to avoid bars growing off screen
		GLuint constexpr max_height { std::numeric_limits<GLuint>::max() };
		auto const histogram_height_scalar_uniform = histogram_program.get_uniform(histogram_vertex::height_scalar);
		glUniform1f(histogram_height_scalar_uniform, static_cast<float>(max_height));

		//enable vertex attrib
		auto const in_vert_attrib = histogram_program.get_attrib_location(histogram_vertex::in_vertex);
		glEnableVertexAttribArray(in_vert_attrib);
		vertex_buffer.bind();
		glVertexAttribPointer(in_vert_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


		gl_wrap::clear_color(0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		std::array<GLuint, histogram_bar_count> histogram_heights;
		std::array<GLuint, histogram_bar_count> histogram_states;
		using algo_ptr = state_machine::i_state_machine<max_height, decltype(histogram_heights), decltype(histogram_states)>*;
		if(algos::gnome_sort == algo_selected){
			state_machine::gnome_sort<max_height, decltype(histogram_heights), decltype(histogram_states)>
				you_ve_been_gnomed (histogram_heights, histogram_states);
			algo_ptr algo = &you_ve_been_gnomed;
			algo->reset();
			draw_loop(main_window, histogram_heights_buffer, histogram_heights, histogram_states_buffer,
				histogram_states, quad, histogram_bar_count, algo);
		}
		else if(algos::bogo_sort == algo_selected){
			state_machine::bogo_sort<max_height, decltype(histogram_heights), decltype(histogram_states)>
				bogo_sorter (histogram_heights, histogram_states);
			algo_ptr algo = &bogo_sorter;
			algo->reset();
			draw_loop(main_window, histogram_heights_buffer, histogram_heights, histogram_states_buffer,
				histogram_states, quad, histogram_bar_count, algo);
		}
		else if(algos::radix_sort == algo_selected){
			state_machine::radix_sort<max_height, decltype(histogram_heights), decltype(histogram_states)>
				radix_sorter (histogram_heights, histogram_states);
			algo_ptr algo = &radix_sorter;
			algo->reset();
			draw_loop(main_window, histogram_heights_buffer, histogram_heights, histogram_states_buffer,
				histogram_states, quad, histogram_bar_count, algo);
		}
		else if(algos::heap_sort == algo_selected){
			state_machine::heap_sort<max_height, decltype(histogram_heights), decltype(histogram_states)>
				heap_sorter (histogram_heights, histogram_states);
			algo_ptr algo = &heap_sorter;
			algo->reset();
			draw_loop(main_window, histogram_heights_buffer, histogram_heights, histogram_states_buffer,
				histogram_states, quad, histogram_bar_count, algo);
		}
	}
	return 0;
}
