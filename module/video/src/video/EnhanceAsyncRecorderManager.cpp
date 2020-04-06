/**
* @file EnhanceAsyncRecorderManager.cpp
* @brief Body of all the files related to the algorithm
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

#include "video/inc/video/EnhanceAsyncRecorderManager.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
EnhanceAsyncRecorderManager::EnhanceAsyncRecorderManager() {
	is_object_initialized_ = false;
	player_recorder_.clear();
	kMaxFramesRecorded_ = 1000;
	msg_len_max_bytes_ = 2048;
	data_block_size_ = 1;
	data_block_offset_ = 1;
	fps_ = 30;
	record_framerate_ = 30;
	shared_buffer_size_ = 0;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::initialize_record(
	const std::string &fname,
	bool do_save_avi) {
	// sanity check
	boost::filesystem::path dir("data");
	if (boost::filesystem::create_directory(dir)) {
		std::cout << "[+] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
	else {
		std::cout << "[-] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
	// set the callback (before the initialization)
	player_recorder_.set_callback_createfile(
		std::bind(&EnhanceAsyncRecorderManager::name_changed,
			this, std::placeholders::_1));
	// Record the images and robot angle
	std::cout << "[i] setup the recorder...";
	// 1 GB for each file (now 100MB)
	do_save_avi_ = do_save_avi;
	if (!do_save_avi_) {
		player_recorder_.setup_file(fname, ".dat", 100000000, 100);
	}
	done_ = true;
	raw_data_ = new unsigned char[2048];
	raw_data_size_ = 0;
	is_initialize_recorder_ = false;
	fname_video_path_ = fname;
	source_scale_ = 1.0f;
	is_object_initialized_ = true;
	std::cout << "done" << std::endl;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::reset() {
	size_first_frame_ = cv::Size(0, 0);
}
// ----------------------------------------------------------------------------
EnhanceAsyncRecorderManager::EARM EnhanceAsyncRecorderManager::record(
	const cv::Mat &img, 
	float t, 
	const unsigned char *raw_data, 
	size_t raw_data_size)
{
	if (!is_object_initialized_) return EARM_ERROR;

	// Set the first frame size
	if (size_first_frame_.width == 0 ||
		size_first_frame_.height == 0) {
		size_first_frame_ = img.size();
	}
	// If the size mismatch, stop the recording
	if (size_first_frame_ != img.size()) return EARM_ERROR;

	// processed the previous frame
	if (done_) {
		done_ = false;
		memcpy(raw_data_, raw_data, raw_data_size);
		raw_data_size_ = raw_data_size;
		processed_srcs_ = img.clone();

		// record on a separated thread
		std::thread t_player_recorder_(
			[this]() {

			bool do_record_internal = true;

			// copy a scaled source
			cv::Mat tmp;
			cv::resize(processed_srcs_, tmp, 
				cv::Size(processed_srcs_.cols * source_scale_,
				processed_srcs_.rows * source_scale_));

			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() + 
				" " + std::to_string(cv::getTickCount());
			// record the data
			size_t timestamp_size = timestamp.size();
			//unsigned char msg[2048];
			//memcpy(msg, raw_data_, raw_data_size_);
			//memcpy(&msg[raw_data_size_], &timestamp[0], timestamp_size);
			//msg[raw_data_size_ + timestamp_size] = '\0';


			memcpy(&shared_buffer_.get()[0], raw_data_, raw_data_size_);
			memcpy(&shared_buffer_.get()[raw_data_size_], &timestamp[0], timestamp_size);
			shared_buffer_.get()[raw_data_size_ + timestamp_size] = '\0';

			// record video with encoded message
			if (do_save_avi_) {
				// record the video
				if (m_data_block_.empty()) {
					storedata::codify::CodifyImage::estimate_data_size(tmp, 
						msg_len_max_bytes_, data_block_size_,
						data_block_offset_, m_data_block_);
				}
				// clean the data block
				m_data_block_ = cv::Scalar::all(0);
				// convert the message in a image
				int x = data_block_offset_, y = data_block_offset_;
				//std::cout << "msg: " << shared_buffer_.get() << std::endl;
				storedata::codify::CodifyImage::data2image(shared_buffer_.get(),
					raw_data_size_ + timestamp_size, data_block_size_,
					data_block_offset_, m_data_block_, x, y);

				// compose the images
				cv::Size frame_size(tmp.cols, tmp.rows + m_data_block_.rows);
				cv::Mat m(frame_size, tmp.type());
				// copy the source
				tmp.copyTo(m(cv::Rect(0, 0, tmp.cols, tmp.rows)));
				// copy the data
				m_data_block_.copyTo(m(cv::Rect(0, tmp.rows, 
					m_data_block_.cols, m_data_block_.rows)));
				// record
				std::map<int, cv::Mat> sources = { { 0, m } };
				//std::cout << "image size: " << m.size() << " " << timestamp << " size: " << raw_data_size_ + s << std::endl;
				if (!is_initialize_recorder_) {
					// create the meta frame
					cv::Mat meta_frame(m.size(), CV_8UC3, cv::Scalar::all(0));
					// data offset, image source size
					std::string msg_meta =
						std::to_string(shared_buffer_size_) + " " +
						std::to_string(msg_len_max_bytes_) + " " +
						std::to_string(data_block_size_) + " " +
						std::to_string(data_block_offset_) + " " +
						std::to_string(tmp.cols) + " " +
						std::to_string(tmp.rows);
					std::cout << "msg_meta: " << msg_meta << std::endl;
					int meta_data_block_size = 1;
					int meta_data_block_offset = 1;
					int meta_x = 0, meta_y = 0;
					storedata::codify::CodifyImage::merge_strings2image(
						std::vector<std::string>() = { msg_meta },
						meta_data_block_size, meta_data_block_offset, 
						meta_frame, meta_x, meta_y);
					std::string msg_meta_decoded;
					meta_x = 0, meta_y = 0;
					storedata::codify::CodifyImage::image2string(
						meta_frame, meta_x, meta_y,
						meta_data_block_size, meta_data_block_offset,
						msg_meta_decoded);
					std::cout << "msg_meta_decoded: " << msg_meta_decoded << 
						std::endl;
					//cv::imshow("meta_frame", meta_frame);
					//cv::waitKey(1);

					//std::cout << "inti_recorder: " << fname_video_path_ << std::endl;
					is_initialize_recorder_ = true;
					player_recorder_.setup_video(sources, 
						fname_video_path_, kMaxFramesRecorded_, fps_, fps_);
					player_recorder_.setup_metaframe(meta_frame);
				}
				// record the source
				player_recorder_.record_video(sources);
			} else {
				// use record dat file
				player_recorder_.record_file(tmp, true, shared_buffer_.get(),
					raw_data_size_ + timestamp_size);
			}
			done_ = true;
		});
		t_player_recorder_.detach();
		return EARM_OK;
	}

	return EARM_BUSY;
}
// ----------------------------------------------------------------------------
EnhanceAsyncRecorderManager::EARM EnhanceAsyncRecorderManager::record(
	const cv::Mat &img, bool do_clone,
	float t,
	const unsigned char *raw_data,
	size_t raw_data_size)
{
	if (!is_object_initialized_) return EARM_ERROR;

	// Set the first frame size
	if (size_first_frame_.width == 0 ||
		size_first_frame_.height == 0) {
		size_first_frame_ = img.size();
	}
	// If the size mismatch, stop the recording
	if (size_first_frame_ != img.size()) return EARM_ERROR;

	// processed the previous frame
	if (done_) {
		done_ = false;
		memcpy(raw_data_, raw_data, raw_data_size);
		raw_data_size_ = raw_data_size;
		if (do_clone) {
			processed_srcs_ = img.clone();
		} else {
			processed_srcs_ = img;
		}
		// record on a separated thread
		std::thread t_player_recorder_(
			[this]() {

			bool do_record_internal = true;

			// copy a scaled source
			cv::Mat tmp;
			cv::resize(processed_srcs_, tmp,
				cv::Size(processed_srcs_.cols * source_scale_,
					processed_srcs_.rows * source_scale_));

			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() +
				" " + std::to_string(cv::getTickCount());
			// record the data
			size_t timestamp_size = timestamp.size();
			//unsigned char msg[2048];
			//memcpy(msg, raw_data_, raw_data_size_);
			//memcpy(&msg[raw_data_size_], &timestamp[0], timestamp_size);
			//msg[raw_data_size_ + timestamp_size] = '\0';


			memcpy(&shared_buffer_.get()[0], raw_data_, raw_data_size_);
			memcpy(&shared_buffer_.get()[raw_data_size_], &timestamp[0], timestamp_size);
			shared_buffer_.get()[raw_data_size_ + timestamp_size] = '\0';

			// record video with encoded message
			if (do_save_avi_) {
				// record the video
				if (m_data_block_.empty()) {
					storedata::codify::CodifyImage::estimate_data_size(tmp,
						msg_len_max_bytes_, data_block_size_,
						data_block_offset_, m_data_block_);
				}
				// clean the data block
				m_data_block_ = cv::Scalar::all(0);
				// convert the message in a image
				int x = data_block_offset_, y = data_block_offset_;
				//std::cout << "msg: " << shared_buffer_.get() << std::endl;
				storedata::codify::CodifyImage::data2image(shared_buffer_.get(),
					raw_data_size_ + timestamp_size, data_block_size_,
					data_block_offset_, m_data_block_, x, y);

				// compose the images
				cv::Size frame_size(tmp.cols, tmp.rows + m_data_block_.rows);
				cv::Mat m(frame_size, tmp.type());
				// copy the source
				tmp.copyTo(m(cv::Rect(0, 0, tmp.cols, tmp.rows)));
				// copy the data
				m_data_block_.copyTo(m(cv::Rect(0, tmp.rows,
					m_data_block_.cols, m_data_block_.rows)));
				// record
				std::map<int, cv::Mat> sources = { { 0, m } };
				//std::cout << "image size: " << m.size() << " " << timestamp << " size: " << raw_data_size_ + s << std::endl;
				if (!is_initialize_recorder_) {
					// create the meta frame
					cv::Mat meta_frame(m.size(), CV_8UC3, cv::Scalar::all(0));
					// data offset, image source size
					std::string msg_meta =
						std::to_string(shared_buffer_size_) + " " +
						std::to_string(msg_len_max_bytes_) + " " +
						std::to_string(data_block_size_) + " " +
						std::to_string(data_block_offset_) + " " +
						std::to_string(tmp.cols) + " " +
						std::to_string(tmp.rows);
					std::cout << "msg_meta: " << msg_meta << std::endl;
					int meta_data_block_size = 1;
					int meta_data_block_offset = 1;
					int meta_x = 0, meta_y = 0;
					storedata::codify::CodifyImage::merge_strings2image(
						std::vector<std::string>() = { msg_meta },
						meta_data_block_size, meta_data_block_offset,
						meta_frame, meta_x, meta_y);
					std::string msg_meta_decoded;
					meta_x = 0, meta_y = 0;
					storedata::codify::CodifyImage::image2string(
						meta_frame, meta_x, meta_y,
						meta_data_block_size, meta_data_block_offset,
						msg_meta_decoded);
					std::cout << "msg_meta_decoded: " << msg_meta_decoded <<
						std::endl;
					//cv::imshow("meta_frame", meta_frame);
					//cv::waitKey(1);

					//std::cout << "inti_recorder: " << fname_video_path_ << std::endl;
					is_initialize_recorder_ = true;
					player_recorder_.setup_video(sources,
						fname_video_path_, kMaxFramesRecorded_, fps_, record_framerate_);
					player_recorder_.setup_metaframe(meta_frame);
				}
				// record the source
				player_recorder_.record_video(sources);
			}
			else {
				// use record dat file
				player_recorder_.record_file(tmp, true, shared_buffer_.get(),
					raw_data_size_ + timestamp_size);
			}
			done_ = true;
		});
		t_player_recorder_.detach();
		return EARM_OK;
	}

	return EARM_BUSY;
}
// ----------------------------------------------------------------------------
bool EnhanceAsyncRecorderManager::is_object_initialized() {
	return is_object_initialized_;
}
// ----------------------------------------------------------------------------
std::string EnhanceAsyncRecorderManager::get_fname_record() {
	std::lock_guard<std::mutex> lk(mtx_);
	return fname_record_;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::name_changed(const std::string &fname) {
	std::lock_guard<std::mutex> lk(mtx_);
	fname_record_ = fname;
	//std::cout << "The new filename is: " << fname << std::endl;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_shared_buffer_size(
	size_t shared_buffer_size) {
	shared_buffer_ = std::unique_ptr<unsigned char>(
		new unsigned char[shared_buffer_size]);
	shared_buffer_size_ = shared_buffer_size;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_kMaxFramesRecorded(
	int kMaxFramesRecorded) {
	kMaxFramesRecorded_ = kMaxFramesRecorded;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_msg_len_max_bytes(size_t msg_len_max_bytes) {
	msg_len_max_bytes_ = msg_len_max_bytes;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_fps(int fps) {
	fps_ = fps;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_record_framerate(int record_framerate) {
	record_framerate_ = record_framerate;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_data_block_size(int data_block_size) {
	data_block_size_ = data_block_size;
}
// ----------------------------------------------------------------------------
void EnhanceAsyncRecorderManager::set_data_block_offset(int data_block_offset) {
	data_block_offset_ = data_block_offset;
}
// ----------------------------------------------------------------------------
bool EnhanceAsyncRecorderManager::read_video_with_meta_header(
	const std::string &fname) {
	cv::VideoCapture vc(fname);
	if (!vc.isOpened()) {
		std::cout << "Unable to open: " << fname << std::endl;
		return EARM_ERROR;
	}
	// read the meta frame
	cv::Mat meta_frame;
	vc >> meta_frame;
	// Extract the parameters for the current video
	std::string msg_meta_decoded;
	int meta_data_block_size = 1;
	int meta_data_block_offset = 1;
	int meta_x = 0, meta_y = 0;
	storedata::codify::CodifyImage::image2string(
		meta_frame, meta_x, meta_y,
		meta_data_block_size, meta_data_block_offset,
		msg_meta_decoded);
	std::cout << "msg_meta_decoded: " << msg_meta_decoded <<
		std::endl;
	std::istringstream iss(msg_meta_decoded);
	std::vector<std::string> results(std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>());
	vc.release();

	// Read the video with the parameters from the header
	return read_video(fname, results, true);
}
// ----------------------------------------------------------------------------
bool EnhanceAsyncRecorderManager::read_video(const std::string &fname,
	std::vector<std::string> &params, bool do_skip_first_frame) {
	cv::VideoCapture vc(fname);
	if (!vc.isOpened()) {
		std::cout << "Unable to open: " << fname << std::endl;
		return EARM_ERROR;
	}

	// Initialize
	int data_block_size = std::stoi(params[2]);
	int data_block_offset = std::stoi(params[3]);
	size_t shared_buffer_size = std::stoi(params[0]);
	size_t msg_len_max_bytes = std::stoi(params[1]);
	cv::Mat tmp(std::stoi(params[5]), std::stoi(params[4]), CV_8UC3);
	cv::Mat m_data_block;
	storedata::codify::CodifyImage::estimate_data_size(tmp, msg_len_max_bytes,
		data_block_size, data_block_offset, m_data_block);
	std::shared_ptr<unsigned char> shared_buffer =
		std::unique_ptr<unsigned char>(new unsigned char[shared_buffer_size]);

	// Check the video
	bool write_img = true;
	if (vc.isOpened()) {
		int num_frame = 0;
		while (true)
		{
			cv::Mat m;
			vc >> m;
			if (m.empty()) {
				break;
			}
			int x = data_block_offset, y = data_block_offset;

			size_t len = 0;
			storedata::codify::CodifyImage::image2data(m(cv::Rect(0, tmp.rows,
				tmp.cols, m.rows - tmp.rows)), x, y, data_block_size,
				data_block_offset, sizeof(len), shared_buffer.get(), shared_buffer_size - 1, len);

			// callback with the information
			if (!get_read_data(m(cv::Rect(0, 0,
				tmp.cols, tmp.rows)),
				shared_buffer.get(), shared_buffer_size - 1)) break;

			// in this example data is consider as a string (but it it can be 
			// anything)
			//shared_buffer.get()[len] = '\0';
			//std::string sData(reinterpret_cast<char*>(shared_buffer.get()));
			//std::cout << "data: " << sData << std::endl;
			//// save the data here
			//// show the image
			//cv::resize(m, m, cv::Size(512, 512));
			//cv::imshow("m", m);
			//if (cv::waitKey(1) == 27) break;
		}
	}

	return EARM_OK;
}
// ----------------------------------------------------------------------------
int EnhanceAsyncRecorderManager::get_read_data(
	const cv::Mat &img, unsigned char *buf, size_t size) {
	std::cout << "EARM" << std::endl;
	return EARM_OK;
}

} // namespace storedata
