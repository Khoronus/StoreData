/* @file create_video.hpp
* @brief Header of the class which performs all the main functions.
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

#ifndef STOREDATA_RECORD_CREATE_VIDEO_HPP__
#define STOREDATA_RECORD_CREATE_VIDEO_HPP__

// Include Boost headers for system time and threading
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/thread.hpp"

#include <opencv2/opencv.hpp>

#include "storedata_typedef.hpp"
#include "storedata_time.hpp"
#include "record_defines.hpp"

namespace storedata
{

/** @brief Class to capture video with a maximum size.
*/
class VideoCaptureManager
{
  public:

	  STOREDATA_RECORD_EXPORT VideoCaptureManager();

	  STOREDATA_RECORD_EXPORT ~VideoCaptureManager();

	  /** @brief Release the allocated data
	  */
	  STOREDATA_RECORD_EXPORT void release();

	  /** @brief Setup the data
	  */
	  STOREDATA_RECORD_EXPORT void setup(
		  unsigned int frames_max_allocable,
		  const std::string &filename,
		  int width, int height, int framerate);

	  /** @brief Setup the meta frame. It is the first frame of each new video.

		  Setup the recorder properties. It is the first frame of each new 
		  video.
		  @param[in] meta_frame Frame to place at the beginning of each new 
		                        video
	  */
	  STOREDATA_RECORD_EXPORT void setup_metaframe(cv::Mat &meta_frame);

	  /** @brief Generate a video capture file.
	  */
	  STOREDATA_RECORD_EXPORT int generate(const std::string &appendix);

	  /** @brief Check if the current video can memorize the frame.
	  */
	  STOREDATA_RECORD_EXPORT int check_memory(const cv::Mat &image);

	  /** @brief Push an image in the video container
	  */
	  STOREDATA_RECORD_EXPORT int push(const cv::Mat &image);

	  /** @brief Change the encoder mode.

		  Change the encoder mode. Usually is 
		  CV_FOURCC('D', 'I', 'V', 'X')
	  */
	  STOREDATA_RECORD_EXPORT void set_video_encoder(int video_encoder);

  private:

	  /** @brief Path and name of the file to memorize
	  */
	  std::string filename_;
	  /** @brief Memory allocated
	  */
	  unsigned int frames_expected_allocated_;
	  /** @brief Max memory allocable
	  */
	  unsigned int frames_max_allocable_;
	  /** @brief Video width (image MUST be same size)
	  */
	  int width_;
	  /** @brief Video height (image MUST be same size)
	  */
	  int height_;
	  /** @brief Video framerate.
	  */
	  int framerate_;

	  /** @brief Meta frame added to each frame (if not empty)
	  */
	  cv::Mat meta_frame_;

	  /** @brief Video writer
	  */
	  cv::VideoWriter video_;

	  /** @brief How the video is encoded
		  CV_FOURCC('D', 'I', 'V', 'X')
	  */
	  int video_encoder_;

};

/** @brief Class to define the property for each video
*/
class VideoGeneratorParams
{
public:

	void set_filename(const std::string &filename) {
		filename_ = filename;
	}
	const std::string& filename() const {
		return filename_;
	}
	void set_height(int height) {
		height_ = height;
	}
	int height() {
		return height_;
	}
	void set_width(int width) {
		width_ = width;
	}
	int width() {
		return width_;
	}
private:

	std::string filename_;
	int width_;
	int height_;
};


/** @brief Class to manage the 
*/
class VideoGeneratorManager
{
public:

	STOREDATA_RECORD_EXPORT VideoGeneratorManager();

	STOREDATA_RECORD_EXPORT ~VideoGeneratorManager();

	/** @brief Setup the videos to memorize
	*/
	STOREDATA_RECORD_EXPORT int setup(
		unsigned int max_memory_allocable_forvideo,
		std::map<int, VideoGeneratorParams> &vgp, int framerate);

	/** @brief Setup the meta frame. It is the first frame of each new video.

		Setup the recorder properties. It is the first frame of each new
		video.
		@param[in] meta_frame Frame to place at the beginning of each new
							  video
        IMPORTANT: Call this function after setup.
	*/
	STOREDATA_RECORD_EXPORT void setup_metaframe(cv::Mat &meta_frame);

	/** @brief Check if the video stream is in writing mode
	*/
	STOREDATA_RECORD_EXPORT bool under_writing();

	/** @brief Function to add the frames to videos. It is run in a separate
	           thread.
	*/
	STOREDATA_RECORD_EXPORT void procedure();

	STOREDATA_RECORD_EXPORT void check();

	/** @brief Try to push the frame data in a video.
	*/
	STOREDATA_RECORD_EXPORT int push_data(const std::map<int, cv::Mat> &frame);

	/** @brief Close the video
	*/
	STOREDATA_RECORD_EXPORT void close();

	/** @brief Set the verbose status
	*/
	STOREDATA_RECORD_EXPORT void set_verbose(bool verbose);

	/** @brief It sets the callback for the function that create a new file
	*/
	STOREDATA_RECORD_EXPORT void set_callback_createfile(
		cbk_fname_changed callback_createfile);

  private:

	/** @brief Writing thread
	*/
	boost::thread* thr_;
	boost::mutex mutex_;

	/** @brief If TRUE the variable is under writing.
	*/
	bool under_writing_;

	// set framerate to record and capture at
	int framerate_;

	// Get the properties from the camera
	double width_;
	double height_;

	// Create a matrix to keep the retrieved frame
	std::map<int, cv::Mat> frame_;

	//vars
	boost::posix_time::time_duration td_, td1_;
	boost::posix_time::ptime nextFrameTimestamp_, 
		currentFrameTimestamp_, initialLoopTimestamp_, finalLoopTimestamp_;
	int delayFound_;
	int totalDelay_;

	/** @brief Create Video
	*/
	std::map<int, VideoCaptureManager* > video_;

	/** @brief Count the number of frames that has to be memorized.
	           It should avoid the use of the mutex.
	*/
	//int number_addframe_requests_;

	bool verbose_;

	/** @brief Callback function when a file is created
	*/
	cbk_fname_changed callback_createfile_;
};

} // namespace storedata

#endif // STOREDATA_RECORD_CREATE_VIDEO_HPP__
