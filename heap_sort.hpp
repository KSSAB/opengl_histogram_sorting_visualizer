#include"i_state_machine.hpp"

namespace state_machine {
	template<GLuint max_value, typename array_t, typename state_array_t>
	struct heap_sort : public i_state_machine<max_value, array_t, state_array_t> {
		using base = i_state_machine<max_value, array_t, state_array_t>;
		using base::collection;
		using base::collection_state;
		using bar_states = typename base::bar_states; //unsorted, compared, to_swap, sorted
		using state_underlying_t = typename state_array_t::value_type;
		using value_t = typename array_t::value_type;

		enum class states {
			find_last_parent, //we will set last parent as a root to downsift from, and the next last parent for next iteration
			downsift_prepare_compare_children,
			downsift_compare_children,
			downsift_compare_parent_to_child,
			downsift_swap_parent_greatest_child,
			pop_prepare_swap,
			pop_swap,
			complete
		} state = states::find_last_parent;

		inline std::size_t heap_parent(std::size_t i){
			return { (i-1) / 2};
		}
		inline std::size_t heap_child_l(std::size_t i){
			return { (2 * i) + 1 };
		}
		inline std::size_t heap_child_r(std::size_t i){
			return {heap_child_l(i) + 1};
		}

		std::size_t heap_end;
		std::size_t downsift_root;
		std::size_t next_downsift;
		bool find_last_parent(){
			downsift_root = heap_parent(heap_end);
			next_downsift = downsift_root - 1;
			state = states::downsift_prepare_compare_children;
			return false;
		}

		std::size_t downsift_greatest_child;
		bool downsift_prepare_compare_children(){
			std::size_t const right_pos = heap_child_r(downsift_root);
			if( right_pos <= heap_end ){ //if there is a left and right child mark
				collection_state[right_pos] = static_cast<state_underlying_t>(bar_states::compared);
				collection_state[right_pos-1] = static_cast<state_underlying_t>(bar_states::compared);
				state = states::downsift_compare_children;
				return true;
			}
			else if (right_pos - 1 <= heap_end) { //left child only, prepare to compare to parent instead
				collection_state[downsift_root] = static_cast<state_underlying_t>(bar_states::compared);
				collection_state[right_pos-1] = static_cast<state_underlying_t>(bar_states::compared);
				downsift_greatest_child = right_pos - 1;
				state = states::downsift_compare_parent_to_child;
				return true;
			}
			else { //no children, move to next root
				if(next_downsift > downsift_root){ //if underflow
					state = states::pop_prepare_swap;
					return true;
				}
				downsift_root = next_downsift;
				--next_downsift;
				state = states::downsift_prepare_compare_children;
			}
			return {};
		}

		bool downsift_compare_children(){
			value_t const right_val = collection[heap_child_r(downsift_root)];
			value_t const left_val = collection[heap_child_l(downsift_root)];
			if(right_val < left_val){
				downsift_greatest_child = heap_child_l(downsift_root);
			}
			else {
				downsift_greatest_child = heap_child_r(downsift_root);
			}
			collection_state[heap_child_l(downsift_root)] =
				static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[heap_child_r(downsift_root)] =
				static_cast<state_underlying_t>(bar_states::unsorted);
			state = states::downsift_compare_parent_to_child;
			return true;
		}

		bool downsift_compare_parent_to_child(){
			value_t const parent = collection[downsift_root];
			value_t const great_child = collection[downsift_greatest_child];
			if(parent > great_child){
				collection_state[downsift_root] = static_cast<state_underlying_t>(bar_states::unsorted);
				collection_state[downsift_greatest_child] = static_cast<state_underlying_t>(bar_states::unsorted);
				if(next_downsift > downsift_root){ //if underflow
					state = states::pop_prepare_swap;
					return true;
				}
				downsift_root = next_downsift;
				--next_downsift;
				state = states::downsift_prepare_compare_children;
			}
			else if(parent < great_child){
				collection_state[downsift_root] = static_cast<state_underlying_t>(bar_states::to_swap);
				collection_state[downsift_greatest_child] = static_cast<state_underlying_t>(bar_states::to_swap);
				state = states::downsift_swap_parent_greatest_child;
			}
			return true;
		}

		bool downsift_swap_parent_greatest_child(){
			collection_state[downsift_root] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[downsift_greatest_child] = static_cast<state_underlying_t>(bar_states::unsorted);
			std::swap(collection[downsift_root], collection[downsift_greatest_child]);
			downsift_root = downsift_greatest_child;
			state = states::downsift_prepare_compare_children;
			return true;
		}

		bool pop_prepare_swap(){
			collection_state[0] = static_cast<state_underlying_t>(bar_states::to_swap);
			collection_state[heap_end] = static_cast<state_underlying_t>(bar_states::to_swap);
			state = states::pop_swap;
			return true;
		}

		bool pop_swap(){
			std::swap(collection[0], collection[heap_end]);
			collection_state[0] = static_cast<state_underlying_t>(bar_states::unsorted);
			collection_state[heap_end] = static_cast<state_underlying_t>(bar_states::sorted);
			heap_end--;
			if(heap_end == 0){
				collection_state[0] = static_cast<state_underlying_t>(bar_states::sorted);
				state = states::complete;
				return true;
			}
			downsift_root = 0;
			state = states::downsift_prepare_compare_children;
			return true;
		}

		void complete(){
			
		}

		void step () final {
			bool external_state_change {false};
			while(!external_state_change){
				switch (state){
					case states::find_last_parent:
						external_state_change = find_last_parent();
						break;
					case states::downsift_prepare_compare_children:
						external_state_change = downsift_prepare_compare_children();
						break;
					case states::downsift_compare_children:
						external_state_change = downsift_compare_children();
						break;
					case states::downsift_compare_parent_to_child:
						external_state_change = downsift_compare_parent_to_child();
						break;
					case states::downsift_swap_parent_greatest_child:
						external_state_change = downsift_swap_parent_greatest_child();
						break;
					case states::pop_prepare_swap:
						external_state_change = pop_prepare_swap();
						break;
					case states::pop_swap:
						external_state_change = pop_swap();
						break;
					case states::complete:
						complete();
					default:
						return;
				}
			}
		}

		heap_sort(array_t & collection, state_array_t & collection_state) :
			base(collection, collection_state),
			heap_end{ collection.size() - 1} {}
	};
}
