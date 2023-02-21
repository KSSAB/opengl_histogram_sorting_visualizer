#pragma once
#include<string_view>
template<std::size_t num_elements>
struct histogram_frag {
	const static std::string shader;
	static constexpr std::string_view states_uniform = "states";
};

template<std::size_t num_elements>
const std::string histogram_frag<num_elements>::shader = R"(#version 430
flat in int index; //gl_InstanceID from vert
in vec2 texcoord;
out vec4 outcolor;
uniform float height_scalar;
layout(std430, binding = 1) readonly buffer values {
	uint height[)"+
		std::to_string(num_elements) +
  R"(];
};
layout(std430, binding = 2) readonly buffer states {
	uint state[)" +
		std::to_string(num_elements) +
	R"(];
};
void main(){
	uint this_state = state[index];
	if( abs(texcoord.x - 0.5f) > 0.40 && state.length() <= 256 /*black bars won't display properly if too small*/){
		outcolor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (this_state == 0) /*unsorted*/ {
		outcolor = vec4(0.85f, 0.85f, 0.85f, 1.0f);
	}
	else if (this_state == 1) /*compared*/ {
		outcolor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (this_state == 2) /*pending swap*/ {
		outcolor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else /*sorted*/ {
		outcolor = vec4(0.0f, 1.0f, height[index] / (height_scalar), 1.0f);
	}
})";
