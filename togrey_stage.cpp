
#include <libcamera/stream.h>
#include "core/libcamera_app.hpp"
#include "post_processing_stages/post_processing_stage.hpp"

using Stream = libcamera::Stream;

class ToGreyStage : public PostProcessingStage {
	public:
	ToGreyStage(LibcameraApp *app) : PostProcessingStage(app) {}
	char const *Name() const override;
	void Read(boost::property_tree::ptree const &params) override {}
	void Configure() override;
	bool Process(CompletedRequestPtr &completed_request) override;
	private:
	Stream *stream_;
	StreamInfo info;
    };

#define NAME "toGrey"

char const *
ToGreyStage::Name() const { return NAME; }

 void
ToGreyStage::Configure() {
	stream_ = app_->LoresStream(&info);
	if(!stream_) throw std::runtime_error(
	  "could not create Lores stream. Have you set options->lores_width and height?" );
    }


 bool
ToGreyStage::Process(CompletedRequestPtr &completed_request) {
	libcamera::Span<uint8_t> buffer = app_->Mmap(completed_request->buffers[stream_])[0];
	// convert YUV420 to grey scale
	// the first sxh bytes are Y, the brightness compared to 0x80, the rest adjust two colors
	uint8_t *ptr = (uint8_t *) buffer.data();
	int sxh = info.stride * info.height;
	for(int i=0; i<buffer.size(); i++) {
		if(i< sxh) ptr++;
		else *(ptr++) = 0x80; // the "neutral" colour
	}
	return false;
}

 static PostProcessingStage *
Create(LibcameraApp *app) { return new ToGreyStage(app); }

static RegisterStage reg(NAME, &Create);
