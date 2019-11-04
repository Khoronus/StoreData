/* @file create_video.cpp
* @brief Body of the class which performs all the main functions.
*
* @section LICENSE
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR/AUTHORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* @author Alessandro Moro <alessandromoro.italy@gmail.com>
* @bug No known bugs.
* @version 0.4.0.0
*
*/

#include "record/inc/record/create_video.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
VideoCaptureManager::VideoCaptureManager() {
}
// ----------------------------------------------------------------------------
VideoCaptureManager::~VideoCaptureManager() {
	release();
}
// ----------------------------------------------------------------------------
void VideoCaptureManager::release() {
	video_.release();
}
// ----------------------------------------------------------------------------
void VideoCaptureManager::setup(unsigned int memory_max_allocable,
	const std::string &filename, 
	int width, int height, int framerate) {

	frames_expected_allocated_ = 0;
	frames_max_allocable_ = memory_max_allocable;
	filename_ = filename;
	height_ = height;
	width_ = width;
	framerate_ = framerate;
#if CV_MAJOR_VERSION == 4
	video_encoder_ = cv::VideoWriter::fourcc('D', 'I', 'V', 'X');
#else
	video_encoder_ = CV_FOURCC('D', 'I', 'V', 'X');
#endif
}
// ----------------------------------------------------------------------------
void VideoCaptureManager::setup_metaframe(cv::Mat &meta_frame) {
	meta_frame_ = meta_frame.clone();
	// if the meta frame is not empty, add at the beginning
	if (!meta_frame_.empty() &&
		video_.isOpened()) {
		video_ << meta_frame_;
		++frames_expected_allocated_;
	}
}
// ----------------------------------------------------------------------------
int VideoCaptureManager::generate(const std::string &appendix) {
	if (!video_.isOpened())
	{
	//if (!video_) {
		std::string filename = filename_ + appendix + ".avi";
		std::cout << filename << std::endl;
		// get the current time
		video_ = cv::VideoWriter(filename, 
			video_encoder_, 
			framerate_, cv::Size(width_, height_));
		frames_expected_allocated_ = 0;
		// if the meta frame is not empty, add at the beginning
		if (!meta_frame_.empty()) {
			video_ << meta_frame_;
			++frames_expected_allocated_;
		}
		return 1;
	}
	return 0;
}
// ----------------------------------------------------------------------------
int VideoCaptureManager::check_memory(const cv::Mat &image) {
	if (frames_expected_allocated_ < frames_max_allocable_) {
		return 1;
	}
	return 0;
}
// ----------------------------------------------------------------------------
int VideoCaptureManager::push(const cv::Mat &image) {
	if (video_.isOpened()) {
		if (check_memory(image)) {
			cv::Mat image_tmp = image;
			if (image.size() != cv::Size(width_, height_)) {
				cv::resize(image, image_tmp, cv::Size(width_, height_));
			}
			//(*video_) << image_tmp;
			video_ << image_tmp;
			// Add the expected image size (raw)
			//memory_expected_allocated_ += image.cols * image.rows * 
			//image.channels();
			// It considers only an image for each frame
			++frames_expected_allocated_;
			return 1;
		}
		else {
			// out of memory
			return -1;
		}
	}
	// The video is not allocated
	return 0;
}
// ----------------------------------------------------------------------------
void VideoCaptureManager::set_video_encoder(int video_encoder) {
	video_encoder_ = video_encoder;
}
// ----------------------------------------------------------------------------
VideoGeneratorManager::VideoGeneratorManager() {
	verbose_ = false;
	under_writing_ = false;
}
// ----------------------------------------------------------------------------
VideoGeneratorManager::~VideoGeneratorManager() {
	close();
}
// ----------------------------------------------------------------------------
int VideoGeneratorManager::setup(
	unsigned int max_memory_allocable_forvideo,
	std::map<int, VideoGeneratorParams> &vgp, int framerate) {

	int return_status = 1;

	// set framerate to record and capture at
	framerate_ = framerate;

	// initialize initial timestamps
	nextFrameTimestamp_ = boost::posix_time::microsec_clock::local_time();
	currentFrameTimestamp_ = nextFrameTimestamp_;
	td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

	// Get the appendix to add to the video
	std::string appendix = storedata::DateTime::time2string();
	for (int i = 0; i < appendix.length(); i++)
	{
		if (appendix[i] == ':') appendix[i] = '_';
	}
	// callback to inform that a new file will be created
	if (callback_createfile_) {
		callback_createfile_(appendix);
	}

	// Create the video writer
	for (auto it = vgp.begin(); it != vgp.end(); it++) {
		video_[it->first] = new VideoCaptureManager();
		video_[it->first]->setup(max_memory_allocable_forvideo,
			it->second.filename(), it->second.width(),
			it->second.height(), framerate_);
		if (!video_[it->first]->generate(appendix)) {
			return_status = 0;
		}
	}

	//number_addframe_requests_ = 0;
	under_writing_ = false;
	return return_status;
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::setup_metaframe(cv::Mat &meta_frame) {
	for (auto &it : video_) {
		it.second->setup_metaframe(meta_frame);
	}
}
// ----------------------------------------------------------------------------
bool VideoGeneratorManager::under_writing() {
	return under_writing_;
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::procedure() {

    boost::mutex::scoped_lock lock(mutex_, boost::try_to_lock);
    if (lock) {
		under_writing_ = true;

		//determine current elapsed time
		currentFrameTimestamp_ = boost::posix_time::microsec_clock::local_time();
		td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

		// wait for X microseconds until 1second/framerate time has passed after previous frame write
		if (td_.total_microseconds() >= 1000000 / framerate_){

			//	 determine time at start of write
			initialLoopTimestamp_ = boost::posix_time::microsec_clock::local_time();

			// Check the memory
			bool memory_ok = true;
			for (auto it = frame_.begin(); it != frame_.end(); it++)
			{
				// Test the videos
				if (video_.find(it->first) != video_.end()) {
					if (!video_[it->first]->check_memory(it->second)) {
						memory_ok = false;
						break;
					}
				}
			}
			// At least one video has not enough memory.
			// Close all the videos and create a new one
			if (!memory_ok) {
				// Get the appendix to add to the video
				std::string appendix = storedata::DateTime::time2string();
				for (int i = 0; i < appendix.length(); i++)
				{
					if (appendix[i] == ':') appendix[i] = '_';
				}
				// callback to inform that a new file will be created
				if (callback_createfile_) {
					callback_createfile_(appendix);
				}
				//std::cout << "Memory failed: " << appendix << std::endl;

				for (auto it = video_.begin(); it != video_.end(); it++)
				{
					it->second->release();
					it->second->generate(appendix);
				}
			}

			// Add the frame
			for (auto it = frame_.begin(); it != frame_.end(); it++)
			{
				// Test the videos
				if (video_.find(it->first) != video_.end()) {
					video_[it->first]->push(it->second);
				}
			}

			//write previous and current frame timestamp to console
			if (verbose_) {
				std::cout << nextFrameTimestamp_ << " " << currentFrameTimestamp_ << " ";
			}

			// add 1second/framerate time for next loop pause
			nextFrameTimestamp_ = nextFrameTimestamp_ + 
				boost::posix_time::microsec(1000000 / framerate_);

			// reset time_duration so while loop engages
			td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

			//determine and print out delay in ms, should be less than 1000/FPS
			//occasionally, if delay is larger than said value, correction will occur
			//if delay is consistently larger than said value, then CPU is not powerful
			// enough to capture/decompress/record/compress that fast.
			finalLoopTimestamp_ = boost::posix_time::microsec_clock::local_time();
			td1_ = (finalLoopTimestamp_ - initialLoopTimestamp_);
			delayFound_ = td1_.total_milliseconds();
			if (verbose_) {
				std::cout << delayFound_ << std::endl;
			}
		}
		under_writing_ = false;
	} 
	//--number_addframe_requests_;
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::check() {
	std::cout << "push " << under_writing_ << std::endl;//" " << number_addframe_requests_ << std::endl;
}
// ----------------------------------------------------------------------------
int VideoGeneratorManager::push_data(const std::map<int, cv::Mat> &frame) {
	if (!under_writing_) {
		boost::mutex::scoped_lock lock(mutex_, boost::try_to_lock);
		if (lock) {
			//++number_addframe_requests_;
			for (auto it = frame.begin(); it != frame.end(); it++)
			{
				frame_[it->first] = it->second.clone();
			}
			boost::thread* thr = new boost::thread(
				boost::bind(&VideoGeneratorManager::procedure, this));
		}
		return 1;
	}
	return 0;
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::close() {
	while (under_writing_ ){ //|| number_addframe_requests_ > 0) {
		//std::cout << under_writing_ << " " << number_addframe_requests_ << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	for (auto it = video_.begin(); it != video_.end(); it++)
	{
		delete it->second;
	}
	video_.clear();
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::set_verbose(bool verbose) {
	verbose_ = verbose;
}
// ----------------------------------------------------------------------------
void VideoGeneratorManager::set_callback_createfile(
	cbk_fname_changed callback_createfile) {
	callback_createfile_ = callback_createfile;
}

} // namespace storedata
