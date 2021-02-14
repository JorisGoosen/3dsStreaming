CXXFLAGS 	= -fPIC -g -W -Wall -Wextra -Werror=return-type -std=c++2a `pkg-config --cflags glfw3 gl glew libpng` -IGereedschap
LIBFLAGS 	= `pkg-config --libs glfw3 glew gl libpng` -LGereedschap -lgereedschap
OBJECTS 	= $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: 3dsReceiver

3dsReceiver: gereedschap $(OBJECTS)
	g++ $(CXXFLAGS) -o $@ $(OBJECTS) $(LIBFLAGS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

gereedschap:
	$(MAKE) -C Gereedschap

clean:
	rm $(OBJECTS) 	|| echo "geen objecten om op te ruimen"
	rm 3dsReceiver	|| echo "er was geen 3dsReceiver"
	$(MAKE) clean -C Gereedschap
