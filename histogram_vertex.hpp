#pragma once
#include<string>
#include<string_view>

template<std::size_t num_elements>
struct histogram_vertex {
	//constexpr would give me big headache
	const static std::string shader;
	static constexpr std::string_view values_uniform = "values";
	static constexpr std::string_view height_scalar = "height_scalar";
	static constexpr std::string_view in_vertex = "in_vertex";
};

template<std::size_t num_elements>
const std::string histogram_vertex<num_elements>::shader = R"(#version 430
in vec3 in_vertex;
flat out int index;
out vec2 texcoord;
uniform float height_scalar; //as screen space in +- 1.0, we will pass in max value and make height inverse proportion
layout(std430, binding=1) readonly buffer values {
	uint height[)" +
		std::to_string(num_elements) +
	R"(];
};
void main(){
	texcoord = in_vertex.xy;

	index = gl_InstanceID;

	gl_Position = vec4(
		((2 * in_vertex.x) / height.length()) - 1.0f + ((2.0f / height.length()) * index),
		((in_vertex.y / height_scalar) * height[index] * 1.9f) -1.0f,
		in_vertex.z, 1.0f);
})";
