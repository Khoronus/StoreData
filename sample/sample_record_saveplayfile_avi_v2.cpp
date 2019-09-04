/* @file sample_record_saveplayfile_avi_v2.cpp
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

//TODO : Create a meta frame
//	   The meta frame is add at the beginning of the video (optional)
//	   Create a base class with the option of record and play

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
#include "video/video_headers.hpp"
#include "version/version.hpp"
#include "../StoreData/StoreData.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief Record the video scene
*/
void record(bool do_save_avi) {
	storedata::EnhanceAsyncRecorderManager earm;
	earm.initialize_record("data\\", do_save_avi);
	earm.set_kMaxFramesRecorded(900);
	earm.set_shared_buffer_size(2048);
	earm.set_msg_len_max_bytes(500);
	earm.set_fps(30);
	earm.set_data_block_size(1);
	earm.set_data_block_offset(1);

	cv::VideoCapture vc(0);
	if (vc.isOpened()) {

		//vc.set(cv::CAP_PROP_FRAME_WIDTH, 2880);
		//vc.set(cv::CAP_PROP_FRAME_HEIGHT, 2880);

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
	size_t msg_len_max_bytes = 500;

	cv::Mat tmp(480, 640, CV_8UC3);
	//cv::Mat m_data_block(30, frame_width, CV_8UC3, cv::Scalar::all(0));
	cv::Mat m_data_block;
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


/** @brief Play a dat file
*/
void play_avi_v2(const std::string &fname) {
	cv::VideoCapture vc(fname);
	if (!vc.isOpened()) {
		std::cout << "Unable to open: " << fname << std::endl;
		return;
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
	//for (auto &it : results) {
	//	std::cout << "res: " << it << std::endl;
	//}
	//cv::imshow("meta_frame", meta_frame);
	//cv::waitKey(0);

	// Initialize
	int data_block_size = std::stoi(results[2]);
	int data_block_offset = std::stoi(results[3]);
	size_t shared_buffer_size = std::stoi(results[0]);
	size_t msg_len_max_bytes = std::stoi(results[1]);
	cv::Mat tmp(std::stoi(results[5]), std::stoi(results[4]), CV_8UC3);
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
			if (write_img) {
				write_img = false;
				cv::imwrite("m.png", m);
			}
			int x = data_block_offset, y = data_block_offset;

			size_t len = 0;
			storedata::codify::CodifyImage::image2data(m(cv::Rect(0, tmp.rows,
				tmp.cols, m.rows - tmp.rows)), x, y, data_block_size,
				data_block_offset, sizeof(len), shared_buffer.get(), shared_buffer_size - 1, len);
			// in this example data is consider as a string (but it it can be 
			// anything)
			shared_buffer.get()[len] = '\0';
			std::string sData(reinterpret_cast<char*>(shared_buffer.get()));
			std::cout << "data: " << sData << std::endl;
			// save the data here
			// show the image
			cv::resize(m, m, cv::Size(512, 512));
			cv::imshow("m", m);
			if (cv::waitKey(1) == 27) break;
		}
	}

}

class DerivateEARM : public storedata::EnhanceAsyncRecorderManager
{
public:
	int get_played_data(
		const cv::Mat &img, 
		unsigned char *buf, 
		size_t size) {

		std::cout << "DerivateEARM" << std::endl;
		return 1;
	}
};

} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[], char *window_name)
{
	std::cout << "version_compiled: " << SD_VERSION_INT << std::endl;
	std::cout << "version_dll: " << StoreDataVersion::get_version_int() << std::endl;
	if (SD_VERSION_INT != StoreDataVersion::get_version_int()) {
		std::cout << "DLL version mismatch" << std::endl;
		return 0;
	}
	bool do_save_as_avi = true;
	record(do_save_as_avi);
	//play_avi_v2("data\\2019_03_21_10_58_07.avi");
	DerivateEARM dearm;
	dearm.play_avi("data\\2019_03_21_10_58_07.avi");

	//play_dat("data\\2019_03_14_11_29_33.dat");
	//play_avi("data\\2019_03_14_11_28_18.avi");
	return 0;
}
