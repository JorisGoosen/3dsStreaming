CXXFLAGS 	= -fPIC -g -W -Wall -Wextra -Werror=return-type -std=c++2a `pkg-config --cflags glfw3 gl glew libpng` -IGereedschap
LIBFLAGS 	= `pkg-config --libs glfw3 glew gl libpng` -LGereedschap -lgereedschap
OBJECTS 	= $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: terrarium

terrarium: gereedschap $(OBJECTS)
	g++ $(CXXFLAGS) -o $@ $(OBJECTS) $(LIBFLAGS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

gereedschap:
	$(MAKE) -C Gereedschap

clean:
	rm $(OBJECTS) 	|| echo "geen objecten om op te ruimen"
	rm terrarium	|| echo "er was geen terrarium"
	$(MAKE) clean -C Gereedschap
