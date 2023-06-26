/**
	See the raspberry-pi documentation on the camera software to see what "stages" are all about.
	This uses the motion_detect_stage provided
 */
#include <chrono>
#include "core/libcamera_app.hpp"
#include "core/options.hpp"

using namespace std::placeholders;

 LibcameraApp app;
 uint64_t timeOut;


 static LibcameraApp *
startCamera() {
	StreamInfo info;
	Options *options = app.GetOptions();
	char arg0[6], *argv[1]; // fake a commandline
	sprintf(arg0,"hello"); argv[0] = arg0;
	if( !options->Parse(1, argv) ) throw std::runtime_error("impossible!");
	options->lores_width = 320; // these need to be set for motion_detect to work
	options->lores_height = 240;//
	options->nopreview = true;
	options->post_process_file = "/home/peter/libcamera-apps/assets/motion_detect.json";
	timeOut = 3000;
	app.OpenCamera();
	app.ConfigureViewfinder(); // required even if nopreview is set
	app.StartCamera();
	return &app;
}
	
	
// The main event loop for the application.

 static void
event_loop(LibcameraApp &app) {
	auto start_time = std::chrono::high_resolution_clock::now();
	bool detected = false;

	for (unsigned int count = 0; ; count++) {
		LibcameraApp::Msg msg = app.Wait();
		if (msg.type == LibcameraApp::MsgType::Timeout) {
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app.StopCamera();
			app.StartCamera();
			continue;
		    }
		if (msg.type == LibcameraApp::MsgType::Quit)
			return;
		if (msg.type != LibcameraApp::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");
		LOG(2, "Viewfinder frame " << count);
		auto now = std::chrono::high_resolution_clock::now();
		if (timeOut && now - start_time > std::chrono::milliseconds(timeOut))
			return;
		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		bool detected_ = false;
		completed_request->post_process_metadata.Get("motion_detect.result",detected_);
		if( detected_ != detected) {
			detected = detected_;
			std::cout << (detected?"got it!":"nope") << std::endl;
		}
	    }
    }

 int
main(int argc, char *argv[]) {
	try {
		LibcameraApp *appPtr = startCamera();
		event_loop(*appPtr);
	} catch (std::exception const &e) {
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return -1;
	    }
	return 0;
    }
