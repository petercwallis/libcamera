
#include <libcamera/stream.h>
#include "core/libcamera_app.hpp"
#include "post_processing_stages/post_processing_stage.hpp"

#include "apriltag/apriltag.h"
#include "apriltag/tagStandard41h12.h"

using Stream = libcamera::Stream;

class ApriltagStage : public PostProcessingStage {
	public:
	ApriltagStage(LibcameraApp *app) : PostProcessingStage(app) {}
	char const *Name() const override;
	void Read(boost::property_tree::ptree const &params) override {}
	void Configure() override;
	bool Process(CompletedRequestPtr &completed_request) override;
	private:
	Stream *stream_;
	StreamInfo info;
	image_u8_t *im;
	apriltag_family_t *tf;
	apriltag_detector_t *td;
	int ctr_;
	std::mutex mutex_;
    };

#define NAME "apriltag_detect"

char const *
ApriltagStage::Name() const { return NAME; }

 void
ApriltagStage::Configure() {
	stream_ = app_->LoresStream(&info);
	if(!stream_) throw std::runtime_error(
	  "could not create Lores stream. Have you set options->lores_width and height?" );
	// create the buffer to copy the image into ready for tag detection
	im = image_u8_create(info.stride,info.height);
	// create the tag detector. see apriltag.h for parrameters that can be set
	//tf = tag16h5_create();
	tf = tagStandard41h12_create();
	td = apriltag_detector_create();
	apriltag_detector_add_family(td,tf);
	ctr_ = 0;
    }

 bool
ApriltagStage::Process(CompletedRequestPtr &completed_request) {
	libcamera::Span<uint8_t> buffer = app_->Mmap(completed_request->buffers[stream_])[0];
	// convert YUV420 to grey scale image_u8_t
	uint8_t *ptr = (uint8_t *) buffer.data();
	int sxh = info.stride * info.height;
	// We need to protect access to the detector
	std::lock_guard<std::mutex> lock(mutex_);
	for(int y=0; y<info.height; y++) {
		for(int x=0; x<info.stride; x++) {
			im->buf[y*im->stride + x] = *(ptr++);
		}
	}
	// grey the actual image
	//for(int i=sxh; i<buffer.size(); i++)
	//	*(ptr++) = 0x80; // the "neutral" colour
	//if(ctr_++ == 10) { image_u8_write_pnm(im,"sample10.pnm"); } // nice!
	zarray_t *detections = apriltag_detector_detect(td,im);
	for(int i=0; i<zarray_size(detections); i++) {
		apriltag_detection_t *det; // see apriltag.h
		zarray_get(detections, i, &det);
		//printf("detection %3d: id (%2dx%2d)-%-4d\n",i, det->family->nbits,
		//  det->family->h, det->id); //,det->c[0],det->c[1]);
		if(det->id >=0 && det->id<4) {
			char l1[30], l2[30];
			sprintf(l1,"x%d",det->id);
			sprintf(l2,"y%d",det->id);
			int x = det->c[0];
			int y = det->c[1];
			completed_request->post_process_metadata.Set(l1,x);
			completed_request->post_process_metadata.Set(l2,y);
		}
	}
	apriltag_detections_destroy(detections); //reqd
	return false;
}

 static PostProcessingStage *
Create(LibcameraApp *app) { return new ApriltagStage(app); }

static RegisterStage reg(NAME, &Create);
