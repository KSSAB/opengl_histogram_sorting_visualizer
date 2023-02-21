CPPFLAGS := -g -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,leak
LDFLAGS := -static-libasan -lglfw -lGL -lGLEW
DOTOS := glfw_wrap.o gl_wrap.o
MAIN_DEPENDENCY := \
	glfw_wrap.hpp gl_wrap.hpp \
	histogram_vertex.hpp histogram_frag.hpp \
	gnome_sort.hpp \
	bogo_sort.hpp \
	radix_sort.hpp \
	heap_sort.hpp \
	i_state_machine.hpp
BAR_COUNT := 64
FRAME_TIME := 16

all: ${DOTOS} main.o
	g++ ${CPPFLAGS} main.o ${DOTOS} ${LDFLAGS}

clean:
	rm -f a.out
	rm -f main.o
	rm -f ${DOTOS}

remake:
	rm -f main.o
	$(MAKE)

nosan: new_cpp := -g -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined
nosan: new_ld := -lglfw -lGL -lGLEW
nosan:
	$(MAKE) clean
	$(MAKE) all CPPFLAGS='$(new_cpp)' LDFLAGS='$(new_ld)'

optim: new_cpp := -O3 -std=c++17 -Wall -Wextra -Wpedantic -Werror
optim: new_ld := -lglfw -lGL -lGLEW
optim:
	$(MAKE) clean
	$(MAKE) all CPPFLAGS='$(new_cpp)' LDFLAGS='$(new_ld)'


main.o: main.cpp ${MAIN_DEPENDENCY} ${SHADERS}
	g++ -c main.cpp ${CPPFLAGS} -DBAR_COUNT=${BAR_COUNT} -DFRAME_TIME=${FRAME_TIME}

glfw_wrap.o: glfw_wrap.cpp glfw_wrap.hpp
	g++ -c glfw_wrap.cpp ${CPPFLAGS}

gl_wrap.o: gl_wrap.cpp gl_wrap.hpp
	g++ -c gl_wrap.cpp ${CPPFLAGS}
