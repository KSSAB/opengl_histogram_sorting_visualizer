#pragma once
#include<random>
#include<algorithm>

namespace state_machine {
	template <GLuint max_value, typename array_t, typename state_array_t>
	struct i_state_machine {
		using state_value_t = typename state_array_t::value_type;

		enum class bar_states : state_value_t {
			unsorted = 0,
			compared = 1,
			to_swap = 2,
			sorted = 3
		};

		virtual void step() = 0;
		array_t& collection;
		state_array_t& collection_state;

		i_state_machine(array_t & collection, state_array_t & collection_state) :
			collection{collection},
			collection_state{collection_state}
		{}
		void reset() {
			std::generate(collection.begin(), collection.end(), [&](){
				return distro(mersenne_gen);
			});
			std::generate(collection_state.begin(), collection_state.end(),
				[](){ return static_cast<state_value_t>(bar_states::unsorted); });
		}
		virtual ~i_state_machine() = default;
	private: //not threadsafe
		static std::random_device rnd;
		static std::mt19937 mersenne_gen;
		static std::uniform_int_distribution<typename array_t::value_type> distro;
	};

	template<GLuint max_value, typename array_t, typename state_array_t>
	std::random_device i_state_machine<max_value, array_t, state_array_t>::rnd {};

	template<GLuint max_value, typename array_t, typename state_array_t>
	std::mt19937 i_state_machine<max_value, array_t, state_array_t>::mersenne_gen {rnd()};

	template<GLuint max_value, typename array_t, typename state_array_t>
	std::uniform_int_distribution<typename array_t::value_type>
		 i_state_machine<max_value, array_t, state_array_t>::distro {0, max_value};
}


