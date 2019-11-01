/**
* @file EnhanceAsyncRecorderManager.hpp
* @brief Header of all the files related to the algorithm
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
* @original author Alessandro Moro
* @bug No known bugs.
* @version 0.1.0.0
*
*/

#ifndef STOREDATA_VIDEO_ENHANCEASYNCRECORDERMANAGER_HPP__
#define STOREDATA_VIDEO_ENHANCEASYNCRECORDERMANAGER_HPP__

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "video_defines.hpp"

#include "record/record_headers.hpp"
#include "codify/codify.hpp"

namespace storedata
{

/** @brief Class to perform an async record of image sources and data 
           information as encoded image.
*/
class EnhanceAsyncRecorderManager {
public:

	enum EARM{
		EARM_OK = 0,
		EARM_ERROR = 1,
		EARM_BUSY = 2
	};

	STOREDATA_VIDEO_EXPORT EnhanceAsyncRecorderManager();

	STOREDATA_VIDEO_EXPORT void initialize_record(
		const std::string &fname,
		bool do_save_avi);

	/** @brief It resets the internal parameters
	*/
	STOREDATA_VIDEO_EXPORT void reset();

	STOREDATA_VIDEO_EXPORT EARM record(
		const cv::Mat &img,
		float t,
		const unsigned char *raw_data,
		size_t raw_data_size);
	/** @brief It records a data with the option to clone the source.
	*/
	STOREDATA_VIDEO_EXPORT EARM record(
		const cv::Mat &img, bool do_clone,
		float t,
		const unsigned char *raw_data,
		size_t raw_data_size);

	STOREDATA_VIDEO_EXPORT bool is_object_initialized();

	/** @brief It gets the filename recorded
	*/
	STOREDATA_VIDEO_EXPORT std::string get_fname_record();

	/** @brief It sets the shared buffer size
	*/
	STOREDATA_VIDEO_EXPORT void set_shared_buffer_size(size_t shared_buffer_size);

	/** @brief It sets the maximum frames recorded in a single video
	*/
	STOREDATA_VIDEO_EXPORT void set_kMaxFramesRecorded(int kMaxFramesRecorded);

	/** @brief It sets the maximum amount of bytes used in a frame
	*/
	STOREDATA_VIDEO_EXPORT void set_msg_len_max_bytes(
		size_t msg_len_max_bytes);

	/** @brief Set expected video fps
	*/
	STOREDATA_VIDEO_EXPORT void set_fps(int fps);

	/** @brief Set Data block size (number of pixels to define 1 bit)
	*/
	STOREDATA_VIDEO_EXPORT void set_data_block_size(int data_block_size);

	/** @brief Offset between bits
	*/
	STOREDATA_VIDEO_EXPORT void set_data_block_offset(int data_block_offset);

	/** @brief It plays an avi with metadata
	*/
	STOREDATA_VIDEO_EXPORT void play_avi(const std::string &fname);

	/** @brief It plays an avi with metadata.
		It plays an avi with metadata.
		@param[in] fname Video file to open
		@param[in] params List of parameters. 
			size_t shared_buffer_size = std::stoi(results[0]);
			int msg_len_max_bytes = std::stoi(results[1]);
			int data_block_size = std::stoi(results[2]);
			int data_block_offset = std::stoi(results[3]);
			int image_width = std::stoi(results[4])
			int image_height = std::stoi(results[5])
	*/
	STOREDATA_VIDEO_EXPORT void play_avi(const std::string &fname,
		std::vector<std::string> &params);

	/** @brief It displayes played data
	*/
	STOREDATA_VIDEO_EXPORT virtual int get_played_data(
		const cv::Mat &img, unsigned char *buf, size_t size);

private:

	/** @brief It defines the size of the first frame (set with the first
	           input).
	*/
	cv::Size size_first_frame_;

	/** @brief If true, this object is initialized and can be used.
			   Otherwise, do nothing
	*/
	bool is_object_initialized_;

	/** @brief How much to rescale the original source
	*/
	float source_scale_;
	/** @brief If true, it records the video and angle information
	*/
	bool do_record_;
	/** @brief If true, it saves in dat format
	*/
	bool do_save_avi_;
	/** @brief Player recorder
	*/
	storedata::PlayerRecorder player_recorder_;
	/** @brief Maximum number of frames recorded in a video
	*/
	int kMaxFramesRecorded_;
	bool done_; // Use an atomic flag.
	unsigned char *raw_data_;
	size_t raw_data_size_;
	bool is_initialize_recorder_;
	std::string fname_video_path_;
	cv::Mat processed_srcs_;
	/** @brief Where the expanded source data is located
	*/
	cv::Mat m_data_block_;

	/** @brief It prevents race condition when a filename is written
	*/
	std::mutex mtx_;
	/** @brief Current filename used to write the data
	*/
	std::string fname_record_;

	/** @brief Buffer used to copy the data in the record data.
	*/
	std::unique_ptr<unsigned char> shared_buffer_;
	/** @brief Size of the shared buffer
	*/
	size_t shared_buffer_size_;

	/** @brief It sets the maximum amount of bytes used in a frame
	*/
	size_t msg_len_max_bytes_;

	/** @brief Data block size (number of pixels to define 1 bit)
	*/
	int data_block_size_;
	/** @brief Offset between bits
	*/
	int data_block_offset_;

	/** @brief Expected video fps
	*/
	int fps_;

	/** @brief Function called when a new file is created
	*/
	STOREDATA_VIDEO_EXPORT void name_changed(const std::string &fname);
};

} // namespace storedata

#endif // STOREDATA_VIDEO_ENHANCEASYNCRECORDERMANAGER_HPP__