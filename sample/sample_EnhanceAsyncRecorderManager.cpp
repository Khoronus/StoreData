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

// Filename of the last saved file.
std::string global_filename_record;

// ----------------------------------------------------------------------------
namespace
{

/** @brief Record the video scene
*/
void sample_record_async(bool do_save_avi) {
	storedata::EnhanceAsyncRecorderManager earm;
	earm.initialize_record("data\\", do_save_avi);
	earm.set_kMaxFramesRecorded(900);
	earm.set_shared_buffer_size(2048);
	earm.set_msg_len_max_bytes(500);
	earm.set_fps(30);
	earm.set_record_framerate(30);
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
			float timestamp = frame_num * 0.03f;
			earm.record(m, timestamp, 
				reinterpret_cast<const unsigned char*>(msg.c_str()),
				msg.size());
			global_filename_record = earm.get_fname_record();

			if (cv::waitKey(1) == 27) break;
			++frame_num;
		}
	}
}

/** @brief Play a dat file
*/
void sample_read_dat(const std::string &fname) {
	// Record a video and save some simple data
	storedata::PlayerRecorder pr;
	unsigned int index_start = 0;
	// play
	pr.read_file(fname, 60);
}


/** @brief Play a dat file
*/
void sample_read_and_unpack_dat(const std::string &fname) {
	// Record a video and save some simple data
	storedata::PlayerRecorder pr;
	unsigned int index_start = 0;
	// unpack
	pr.unpack(fname, 60,
		"unpack\\", index_start);

	std::cout << "Open the dat file unpacked" << std::endl;
	// read the txt files
	for (int i = 0; i < 100; ++i) {
		cv::Mat img = cv::imread("unpack\\" + std::to_string(i) + ".jpg");
		if (img.empty()) continue;
		std::ifstream f("unpack\\" + std::to_string(i) + ".txt", std::ios::binary | std::ios::ate);
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


/** @brief Example derivate class.

	Example derivate class. The video and message data is only display for demo.
*/
class DerivateEARM : public storedata::EnhanceAsyncRecorderManager
{
public:

	/** @brief Get played data
	*/
	int get_read_data(
		const cv::Mat &img, 
		unsigned char *buf, 
		size_t size) override {

		cv::imshow("img", img);
		std::string msg(reinterpret_cast<char*>(buf), size);
		msg[size] = '\0';
		std::cout << "DerivateEARM: " << msg << std::endl;

		cv::waitKey(1);
		return storedata::EnhanceAsyncRecorderManager::EARM_OK;
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
	// Record the current webcam image
	bool do_save_as_avi = false;
	sample_record_async(do_save_as_avi);
	//play_avi_v2("data\\2019_03_21_10_58_07.avi");
	DerivateEARM dearm;
	std::string fname = "data\\" + global_filename_record + ".avi";
	if (dearm.read_video_with_meta_header(fname) == 
		storedata::EnhanceAsyncRecorderManager::EARM_ERROR) {
		std::cout << "[-] read:" << fname << std::endl;
		fname = "data\\" + global_filename_record + ".dat";
		std::cout << "[!] Try to read:" << fname << std::endl;
		sample_read_dat(fname);
		cv::waitKey();
		sample_read_and_unpack_dat(fname);
	}

	//play_dat("data\\2019_03_14_11_29_33.dat");
	//play_avi("data\\2019_03_14_11_28_18.avi");
	return 0;
}
