CXXFLAGS = -Wall -O2 -std=c++17 $(shell pkg-config --cflags gstreamer-1.0)
LIBS     = $(shell pkg-config --libs gstreamer-1.0)

pcm_app: pcm_app.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

run: pcm_app
	./pcm_app A && ./pcm_app B && ./pcm_app C

clean:
	rm -rf pcm_app saida
