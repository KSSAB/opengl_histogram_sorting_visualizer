#pragma once
#include"i_state_machine.hpp"
#include<random>

namespace state_machine {
	template<GLuint max_value, typename array_t, typename state_array_t>
	struct bogo_sort : public i_state_machine<max_value, array_t, state_array_t> {
		using base = i_state_machine<max_value, array_t, state_array_t>;
		using base::collection;
		using base::collection_state;
		using bar_states = typename base::bar_states;//enum class unsorted, compared, to_swap, or sorted
		using state_underlying_t = typename state_array_t::value_type;

		enum class states {
			shuffle,
			shuffle_swap,
			pend_compare,
			compare,
			complete
		} state = states::shuffle;

		void complete(){
		}

		std::size_t scanner_pos; //position for linear scan to check if sorted
		bool pend_compare(){
			collection_state[scanner_pos] = static_cast<state_underlying_t>(bar_states::compared);
			collection_state[scanner_pos + 1] = static_cast<state_underlying_t>(bar_states::compared);
			state = states::compare;
			return true;
		}

		using value_t = typename array_t::value_type;
		bool compare(){
			value_t const here = collection[scanner_pos];
			value_t const next = collection[scanner_pos + 1];
			collection_state[scanner_pos] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[scanner_pos + 1] = static_cast<state_underlying_t>(bar_states::unsorted);
			if(here > next){
				state = states::shuffle;
				shuffled_so_far = 0;
				return true;
			}
			else{
				if(scanner_pos + 2 < collection.size()){
					scanner_pos += 1;
					state = states::pend_compare;
				}
				else {
					state = states::complete;
					for(auto i {collection_state.begin()}; i < collection_state.cend(); ++i){
						*i = static_cast<state_underlying_t>(bar_states::sorted);
					}
				}
			}
			return false;
		}

		std::size_t pending_swap_a;
		std::size_t pending_swap_b;
		bool shuffle_swap(){
			collection_state[pending_swap_a] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[pending_swap_b] = static_cast<state_underlying_t>(bar_states::unsorted);
			std::swap(collection[pending_swap_a], collection[pending_swap_b]);
			state = states::shuffle;
			return true;
		}

		std::random_device rnd {};
		std::mt19937 mersenne_gen {rnd()};
		std::size_t shuffled_so_far {};
		bool shuffle(){
			if(shuffled_so_far == collection.size() -1){
				scanner_pos = 0;
				state = states::pend_compare;
			}
			std::uniform_int_distribution<std::size_t> un_shuffled {shuffled_so_far, collection.size() -1};
			std::size_t const to_swap = un_shuffled(mersenne_gen);
			if(to_swap != shuffled_so_far){
				pending_swap_a = shuffled_so_far;
				pending_swap_b = to_swap;
				collection_state[pending_swap_a] = static_cast<state_underlying_t>(bar_states::to_swap);
				collection_state[pending_swap_b] = static_cast<state_underlying_t>(bar_states::to_swap);
				state = states::shuffle_swap;
				shuffled_so_far += 1;
				return true;
			}
			shuffled_so_far += 1;
			return false;
		}


		void step() final {
			bool external_state_change {false};
			while(!external_state_change){
				switch (state){
					case states::shuffle:
						external_state_change = shuffle();
						break;
					case states::shuffle_swap:
						external_state_change = shuffle_swap();
						break;
					case states::pend_compare:
						external_state_change = pend_compare();
						break;
					case states::compare:
						external_state_change = compare();
						break;
					case states::complete:
						complete();
						//fallthrough intended
					default:
						return;
				}
			}
		}

		bogo_sort(array_t & collection, state_array_t & collection_state) :
			base(collection, collection_state) {}
	};


}
