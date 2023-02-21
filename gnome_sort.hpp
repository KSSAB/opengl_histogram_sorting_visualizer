#pragma once
#include"i_state_machine.hpp"

namespace state_machine{
	template<GLuint max_value, typename array_t, typename state_array_t>
	struct gnome_sort : public i_state_machine<max_value, array_t, state_array_t> {
		using base = i_state_machine<max_value, array_t, state_array_t>;
		using base::collection;
		using base::collection_state; //stores one of bar_states below per index
		using bar_states = typename base::bar_states;//enum class unsorted, compared, to_swap, or sorted
		using state_underlying_t = typename state_array_t::value_type;

		enum class states {
			pend_compare,
			compare,
			pend_swap,
			swap,
			complete
		} 	state = states::pend_compare;

		std::size_t gnome_pos;
		using value_t = typename array_t::value_type;

		void complete(){

		}

		bool pend_compare(){
			collection_state[gnome_pos] = static_cast<state_underlying_t>(bar_states::compared);
			collection_state[gnome_pos + 1] = static_cast<state_underlying_t>(bar_states::compared);
			state = states::compare;
			return true;
		}

		bool compare(){
			value_t const here = collection[gnome_pos];
			value_t const next = collection[gnome_pos + 1];
			if(here > next){
				state = states::pend_swap;
				return false;
			}
			else{
				collection_state[gnome_pos] = static_cast<state_underlying_t>(bar_states::unsorted);
				collection_state[gnome_pos + 1] = static_cast<state_underlying_t>(bar_states::unsorted);
				if (gnome_pos + 2 < collection.size()){
					gnome_pos += 1;
					state = states::pend_compare;
				}
				else{
					for(auto i {collection_state.begin()}; i < collection_state.cend(); ++i){
						*i = static_cast<state_underlying_t>(bar_states::sorted);
					}
					state = states::complete;
				}
			}
			return true;
		}

		bool pend_swap(){
			collection_state[gnome_pos] = static_cast<state_underlying_t>(bar_states::to_swap);
			collection_state[gnome_pos + 1] = static_cast<state_underlying_t>(bar_states::to_swap);
			state = states::swap;
			return true;
		}

		bool swap(){
			collection_state[gnome_pos] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[gnome_pos + 1] = static_cast<state_underlying_t>(bar_states::unsorted);
			std::swap(collection[gnome_pos], collection[gnome_pos+1]);
			if(gnome_pos > 0){
				gnome_pos -= 1;
			}
			state = states::pend_compare;
			return true;
		}

		void step () final {
			bool external_state_change {false}; /*we will not exit a 'step' until externally visible change
				to avoid staring at the histogram doing nothing*/
			while(!external_state_change){
				switch (state){
					case states::pend_compare:
						external_state_change = pend_compare();
						break;
					case states::compare:
						external_state_change = compare();
						break;
					case states::pend_swap:
						external_state_change = pend_swap();
						break;
					case states::swap:
						external_state_change = swap();
						break;
					case states::complete:
						return;
					default:
						return;
				}
			}
		}

		gnome_sort(array_t & collection, state_array_t& collection_state) : 
			base(collection, collection_state), gnome_pos(0) {}
	};
}
