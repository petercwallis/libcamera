#include <chrono>
#include "core/libcamera_app.hpp"
#include "core/options.hpp"
#include <libcamera/stream.h>

using namespace std::placeholders;
using Stream = libcamera::Stream;

 LibcameraApp app;
 Stream *stream_;
 uint64_t timeOut;

 static LibcameraApp *
startCamera() {
	Options *options = app.GetOptions();
	char arg0[6], *argv[1];
	sprintf(arg0,"hello"); argv[0] = arg0;
	if( !options->Parse(1, argv) ) throw std::runtime_error("impossible error.");
	options->lores_width = 320;  //64;
	options->lores_height = 240; //64;
	options->post_process_file = "togrey.json";
	timeOut = 3000;
	app.OpenCamera();
	app.ConfigureViewfinder();
	app.StartCamera();
	return &app;
}
	
	
// The main event loop for the application.

 static void
event_loop(LibcameraApp &app) {
	auto start_time = std::chrono::high_resolution_clock::now();
	StreamInfo info;
	stream_ = app.LoresStream(&info);
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
		app.ShowPreview(completed_request, stream_);
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
