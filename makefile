CFLAGS = -g -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: bin/fs

clean:
	rm -rf bin obj devices

bin: 
	mkdir -p bin
	mkdir -p devices

bin/fs: bin obj/fs.o obj/device.o obj/bitmap.o obj/main.o obj/device_update.o
	g++ -g -o bin/fs obj/* $(LINKFLAGS) -lstdc++fs

obj:
	mkdir -p obj

obj/main.o: obj main.cpp hppsrc/fs.hpp
	g++ -g $(CFLAGS) -std=c++11 -c main.cpp -o $@ -lstdc++fs

obj/fs.o: obj cppsrc/fs.cpp hppsrc/fs.hpp
	g++ -g $(CFLAGS) -std=c++11 -c cppsrc/fs.cpp -o $@ -lstdc++fs

obj/device.o: obj cppsrc/device.cpp hppsrc/device.hpp
	g++ -g $(CFLAGS) -std=c++11 -c cppsrc/device.cpp -o $@ -lstdc++fs

obj/bitmap.o: obj cppsrc/bitmap.cpp hppsrc/bitmap.hpp
	g++ -g $(CFLAGS) -std=c++11 -c cppsrc/bitmap.cpp -o $@ -lstdc++fs

obj/device_update.o: obj cppsrc/device_update.cpp hppsrc/device_update.hpp
	g++ -g $(CFLAGS) -std=c++11 -c cppsrc/device_update.cpp -o $@ -lstdc++fs	
