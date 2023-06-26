
BASELIBS := -lstdc++ -lcamera-base -lcamera -lcamera_app

# hello.cpp is basically a copy of libcamera_hello, without the command line arguments
helloWorld:  helloWorld.o
        gcc -std=c++17 -o $@ $^ $(BASELIBS)

# the helloWorld example modified to use the in-built motion_detect_stage (and write out a response)
# use Ctl-C to terminate it
motionDetector: motion_detect.o
        gcc -std=c++17 -o $@ $^ $(BASELIBS)

# togrey runs a stage that converts the image to low resolution (lores) grey
# note togrey_stage follows the structure of things in libcamera-apps/post_processing_stages
# It could be part of the libcamera-app, but this is how you do it without recompiling the package
togrey: togrey.o togrey_stage.o
        gcc -std=c++17 -o $@ $^ $(BASELIBS) -lpost_processing_stages
        

# apriltag needs to be downloaded and installed or else you'll get an error
# remember you may need to run ldconfig after installing any libraries
apriltag: apriltag_detect.o apriltag_detect_stage.o
        gcc -std=c++17 -o $@ $^ $(BASELIBS) -lpost_processing_stages -L/usr/local/lib/arm-linux-gnueabihf/ -lapriltag

%.o:%.cpp
        gcc -std=c++17 -I/home/peter/libcamera-apps -I/usr/include/libcamera -c $<

clean:
        rm -f *.o helloWorld motionDetector togrey apriltag
