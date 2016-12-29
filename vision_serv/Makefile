all: vision_serv

vision_serv: main.o findHue.o
	g++ main.o findHue.o -lfreenect2 -R /usr/local/lib /usr/local/lib/libopencv_core.so.3.2 /usr/local/lib/libopencv_imgproc.so.3.2 /usr/local/lib/libopencv_imgcodecs.so.3.2 /usr/local/lib/libopencv_cudaimgproc.so.3.2 -o vision_serv

main.o: main.cpp
	g++ -c main.cpp

findHue.o: findHue.cpp
	g++ -c findHue.cpp

clean: 
	rm main.o findHue.o
