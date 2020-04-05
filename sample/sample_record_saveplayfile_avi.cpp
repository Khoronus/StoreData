/* @file main.cpp
 * @brief Main file with the example for the hog descriptor and visualization.
 *
 * @section LICENSE
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @author Alessandro Moro <alessandromoro.italy@gmail.com>
 * @bug No known bugs.
 * @version 0.1.0.0
 * 
 */


#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "record/record_headers.hpp"
#include "codify/codify.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief Class to perform an async record
*/
class EnhanceAsyncRecorderManager {
public:
	EnhanceAsyncRecorderManager() {
	}

	void initialize_record(
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
			player_recorder_.setup(fname, 100000000, 100);
		}
		done_ = true;
		raw_data_ = new unsigned char[2048];
		raw_data_size_ = 0;
		is_initialize_recorder_ = false;
		fname_video_path_ = fname;
		source_scale_ = 1.0f;
		std::cout << "done" << std::endl;
	}

	// -----------------------------------
	void record(
		const cv::Mat &img, 
		float t, 
		const unsigned char *raw_data, 
		size_t raw_data_size)
	{
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
				cv::resize(processed_srcs_, tmp, cv::Size(processed_srcs_.cols * source_scale_,
					processed_srcs_.rows * source_scale_));

				// calculates a timestamp
				time_t now;
				char the_date[1024];
				the_date[0] = '\0';
				now = time(NULL);
				if (now != -1)
				{
					strftime(the_date, 1024, "%Y-%m-%d.%X", gmtime(&now));
				}
				std::string timestamp = the_date;
				// add the tick count
				timestamp += " " + std::to_string(cv::getTickCount());
				// record the data
				size_t s = timestamp.size();
				unsigned char msg[2048];
				memcpy(msg, raw_data_, raw_data_size_);
				memcpy(&msg[raw_data_size_], &timestamp[0], s);

				// record video with encoded message
				if (do_save_avi_) {
					// record the video
					int data_block_size = 1;
					int data_block_offset = 1;
					if (m_data_block_.empty()) {
						size_t msg_len_max_bytes = 500;
						storedata::codify::CodifyImage::estimate_data_size(tmp, 
							msg_len_max_bytes, data_block_size, 
							data_block_offset, m_data_block_);
					}
					// convert the message in a image
					int x = data_block_offset, y = data_block_offset;
					std::cout << "msg: " << msg << std::endl;
					storedata::codify::CodifyImage::data2image(msg, raw_data_size_ + s, 
						data_block_size, data_block_offset, m_data_block_,
						x, y);
					// compose the images
					cv::Size frame_size(tmp.cols, tmp.rows + m_data_block_.rows);
					cv::Mat m(frame_size, tmp.type());
					tmp.copyTo(m(cv::Rect(0, 0, tmp.cols, tmp.rows)));
					m_data_block_.copyTo(m(cv::Rect(0, tmp.rows, m_data_block_.cols, m_data_block_.rows)));
					// record
					std::map<int, cv::Mat> sources = { { 0, m } };
					//std::cout << "image size: " << m.size() << " " << timestamp << " size: " << raw_data_size_ + s << std::endl;
					if (!is_initialize_recorder_) {
						//std::cout << "inti_recorder: " << fname_video_path_ << std::endl;
						is_initialize_recorder_ = true;
						player_recorder_.setup_video(sources, fname_video_path_, 30 * 30, 30);
					}
					player_recorder_.record_video(sources);
				} else {
					// use record dat file
					player_recorder_.record(tmp, true, msg, raw_data_size_ + s);
				}
				done_ = true;
			});
			t_player_recorder_.detach();
		}
	}

	/** @brief It gets the filename recorded
	*/
	std::string get_fname_record() {
		std::lock_guard<std::mutex> lk(mtx_);
		return fname_record_;
	}


private:

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

	/** @brief Function called when a new file is created
	*/
	void name_changed(const std::string &fname) {
		std::lock_guard<std::mutex> lk(mtx_);
		fname_record_ = fname;
		std::cout << "The new filename is: " << fname << std::endl;
	}
};


/** @brief Record the video scene
*/
void record(bool do_save_avi) {
	EnhanceAsyncRecorderManager earm;
	earm.initialize_record("data\\", do_save_avi);
	cv::VideoCapture vc(0);
	if (vc.isOpened()) {
		int frame_num = 0;
		while (true) {
			cv::Mat m;
			vc >> m;
			if (m.empty()) continue;
			cv::imshow("m", m);
			std::string msg = "frame_num: " + std::to_string(frame_num) + "|";
			earm.record(m, 0, reinterpret_cast<const unsigned char*>(msg.c_str()),
				msg.size());
			if (cv::waitKey(1) == 27) break;
			++frame_num;
		}
	}
}

/** @brief Play a dat file
*/
void play_dat(const std::string &fname) {
	// Record a video and save some simple data
	storedata::PlayerRecorder pr;
	unsigned int index_start = 0;
	// play
	pr.play(fname, 60);
	// unpack
	pr.unpack(fname, 60,
		"data\\", index_start);

	std::cout << "Open the dat file unpacked" << std::endl;
	// read the txt files
	for (int i = 0; i < 100; ++i) {
		cv::Mat img = cv::imread("data\\" + std::to_string(i) + ".jpg");
		if (img.empty()) continue;
		std::ifstream f("data\\" + std::to_string(i) + ".txt", std::ios::binary | std::ios::ate);
		if (f.is_open()) {
			std::cout << "file: " << i << std::endl;
			std::streamsize size = f.tellg();
			std::cout << "size: " << size << std::endl;
			f.seekg(0, std::ios::beg);
			std::vector<char> buffer(size);
			if (f.read(buffer.data(), size))
			{
				std::string str(buffer.begin(), buffer.end());
				std::cout << "buffer: " << str << std::endl;
			}

			//if (size >= 384) {
			//	size = 384;
			//	f.seekg(0, std::ios::beg);
			//	std::vector<char> buffer(size);
			//	if (f.read(buffer.data(), size))
			//	{
			//		udp_komatsu_2arm.convert(buffer.data(), size);
			//	}
			//}
		}
		cv::imshow("img", img);
		cv::waitKey(1);
	}

}

/** @brief Play a dat file
*/
void play_avi(const std::string &fname) {
	cv::VideoCapture vc(fname);
	if (!vc.isOpened()) {
		std::cout << "Unable to open: " << fname << std::endl;
		return;
	}
	int data_block_size = 1;
	int data_block_offset = 1;

	cv::Mat tmp(480, 640, CV_8UC3);
	//cv::Mat m_data_block(30, frame_width, CV_8UC3, cv::Scalar::all(0));
	cv::Mat m_data_block;
	size_t msg_len_max_bytes = 500;
	storedata::codify::CodifyImage::estimate_data_size(tmp, msg_len_max_bytes, 
		data_block_size, data_block_offset, m_data_block);

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
			if (write_img) {
				write_img = false;
				cv::imwrite("m.png", m);
			}
			int x = data_block_offset, y = data_block_offset;

			unsigned char data[2049];
			size_t len = 0;
			storedata::codify::CodifyImage::image2data(m(cv::Rect(0, tmp.rows, 
				tmp.cols, m.rows - tmp.rows)), x, y, data_block_size, 
				data_block_offset, sizeof(len), data, 2048, len);
			// in this example data is consider as a string (but it it can be 
			// anything)
			data[len] = '\0';
			std::string sData(reinterpret_cast<char*>(data));
			std::cout << "data: " << sData << std::endl;
			// save the data here
			// show the image
			cv::resize(m, m, cv::Size(512, 512));
			cv::imshow("m", m);
			if (cv::waitKey(1) == 27) break;
		}
	}

}


} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[], char *window_name)
{
	// DELETE THIS FILE
	bool do_save_as_avi = false;
	record(do_save_as_avi);

	play_dat("data\\2019_03_14_11_29_33.dat");
	play_avi("data\\2019_03_14_11_28_18.avi");
	return 0;
}
