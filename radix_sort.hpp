#pragma once
#include"i_state_machine.hpp"
#include<random>
#include<iostream>
#include<stack>

namespace state_machine {
	template<GLuint max_value, typename array_t, typename state_array_t>
	struct radix_sort : public i_state_machine<max_value, array_t, state_array_t> {
		using base = i_state_machine<max_value, array_t, state_array_t>;
		using base::collection;
		using base::collection_state;
		using bar_states = typename base::bar_states;//enum class unsorted, compared, to_swap, or sorted
		using state_underlying_t = typename state_array_t::value_type;
		using value_t = typename array_t::value_type;
		static constexpr std::size_t bit_width {sizeof(value_t) * 8};

		enum class states {
			pend_unary,
			unary,
			to_swap,
			complete
		} state = states::pend_unary;

		struct sub_task {
			std::size_t zero_bin;
			std::size_t one_bin;
			std::size_t zero_origin;
			std::size_t one_origin;
			std::size_t pass_num;
		};
		std::stack<sub_task> pending_tasks;
		sub_task this_task;

		void next_task(){
			if(this_task.pass_num == (bit_width -1)){
				for(std::size_t i {this_task.zero_origin}; i < this_task.one_origin; ++i){
					collection_state[i] = static_cast<state_underlying_t>(bar_states::sorted);
				}
			}
			if(pending_tasks.empty()){
				state = states::complete;
				return;
			}
			state = states::pend_unary;
			this_task = pending_tasks.top();
			pending_tasks.pop();
		}

		bool pend_unary(){
			if(this_task.zero_bin == this_task.one_bin){
				if(this_task.pass_num < (bit_width - 1)){
					bool low_range_complete {true};
					bool high_range_complete {true};
					if(this_task.zero_bin - this_task.zero_origin > 1){ //zero_origin->element, element<-zero_bin at least
						pending_tasks.push({this_task.zero_origin, this_task.zero_bin,
							this_task.zero_origin, this_task.zero_bin, this_task.pass_num + 1});
						low_range_complete = false;
					}
					if(this_task.one_origin - this_task.one_bin > 1){
						pending_tasks.push({this_task.one_bin, this_task.one_origin,
							this_task.one_bin, this_task.one_origin, this_task.pass_num + 1});
						high_range_complete =false;
					}
					if(low_range_complete){
						for(std::size_t i {this_task.zero_origin}; i < this_task.zero_bin; ++i){
							collection_state[i] = static_cast<state_underlying_t>(bar_states::sorted);
						}
					}
					if(high_range_complete){
						for(std::size_t i {this_task.one_bin}; i < this_task.one_origin; ++i){
							collection_state[i] = static_cast<state_underlying_t>(bar_states::sorted);
						}
					}
				}
				next_task();
				if(states::complete == state){
					return false;
				}
			}
			collection_state[this_task.zero_bin] = static_cast<state_underlying_t>(bar_states::compared);
			state = states::unary;
			return true;
		}

		bool unary(){
			value_t bitmask = (value_t{1} << (bit_width - (this_task.pass_num + 1)));
			bool const well_placed = !(collection[this_task.zero_bin] & bitmask);
			if(well_placed){
				collection_state[this_task.zero_bin] = static_cast<state_underlying_t>(bar_states::unsorted);
				++this_task.zero_bin;
				state = states::pend_unary;
			}
			else{
				--this_task.one_bin;
				if(this_task.one_bin == this_task.zero_bin){ //point at same element no need to swap
					collection_state[this_task.zero_bin] = static_cast<state_underlying_t>(bar_states::unsorted);
					state = states::pend_unary;
				}
				else{
					collection_state[this_task.zero_bin] = static_cast<state_underlying_t>(bar_states::to_swap);
					collection_state[this_task.one_bin] = static_cast<state_underlying_t>(bar_states::to_swap);
					state = states::to_swap;
				}
			}
			return true;
		}

		bool to_swap(){
			collection_state[this_task.zero_bin] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[this_task.one_bin] = static_cast<state_underlying_t>(bar_states::unsorted);
			std::swap(collection[this_task.zero_bin], collection[this_task.one_bin]);
			state = states::pend_unary;
			return true;
		}

		void complete(){}

		void step() final {
			bool external_state_change {false};
			while(!external_state_change){
				switch (state){
					case states::pend_unary:
						external_state_change = pend_unary();
						break;
					case states::unary:
						external_state_change = unary();
						break;
					case states::to_swap:
						external_state_change = to_swap();
						break;
					case states::complete:
						complete();
					default:
						return;
				}
			}
		}

		radix_sort(array_t & collection, state_array_t & collection_state) :
				base(collection, collection_state),
				this_task{0, collection.size(), 0, collection.size(), 0}
		{}
	};


}
