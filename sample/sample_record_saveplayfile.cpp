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
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "record/record_headers.hpp"

#include "UDPKomatsu2Arm.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief It records a video
*/
int record() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::PlayerRecorder pr;
	pr.setup("data\\record_", 100000000, 100);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		pr.record(curr, 1, msg);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}
	return 0;
}

/** @brief It records a video with a raw recorder
*/
int record2() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::RawRecorder pr;
	pr.setup("data\\record_", 10000, 100);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		//std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 " + std::to_string((float)rand() / RAND_MAX);
		pr.record(msg);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}
	return 0;
}

/** @brief It records a video
*/
int record3() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::PlayerRecorder pr;
	std::map<int, cv::Mat> sources;
	sources[0] = cv::Mat(480, 640, CV_8UC3);
	pr.setup_video(sources, "data\\record_", 30 * 10, 30);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		//std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		//pr.record(curr, 1, msg);

		sources[0] = curr;
		pr.record_video(sources);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}
	return 0;
}


/** @brief It creates a folder if necessary
*/
void create_folder(const std::string &folder) {
	boost::filesystem::path dir(folder);
	if (boost::filesystem::create_directory(dir)) {
		std::cout << "[+] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
	else {
		std::cout << "[-] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
}


void create_imagesource() {
	cv::Mat img = cv::imread("D:\\workspace\\work\\ImPACT\\dev\\UDPKomatsu2Arm\\data\\0.jpg");
	cv::Mat img0 = img(cv::Rect(0, 0, img.cols / 2, img.rows / 2));
	cv::Mat img1 = img(cv::Rect(img.cols / 2, 0, img.cols / 2, img.rows / 2));
	cv::Mat img2 = img(cv::Rect(0, img.rows/ 2, img.cols / 2, img.rows / 2));
	cv::Mat img3 = img(cv::Rect(img.cols / 2, img.rows / 2, img.cols / 2, img.rows / 2));
	cv::resize(img0, img0, cv::Size(2048, 2048));
	cv::resize(img1, img1, cv::Size(2048, 2048));
	cv::resize(img2, img2, cv::Size(2048, 2048));
	cv::resize(img3, img3, cv::Size(2048, 2048));
	cv::imwrite("0.jpg", img0);
	cv::imwrite("1.jpg", img1);
	cv::imwrite("2.jpg", img2);
	cv::imwrite("3.jpg", img3);
}
void create_robotfile() {
	// Create a binary file where to save the received data
	std::string str = "_test";
	std::ofstream myFile;
	myFile.open("data" + str + ".bin", std::ios::out | std::ios::binary);
	char msg[64] = "UDPKomatsu2Arm v1.0.0 64byte";
	myFile.write(msg, 64);

	for (int i = 0; i < 100; ++i) {
		std::ifstream f("D:\\workspace\\work\\ImPACT\\dev\\UDPKomatsu2Arm\\data\\" + std::to_string(i) + ".txt", std::ios::binary | std::ios::ate);
		if (f.is_open()) {
			std::cout << "file: " << i << std::endl;
			std::streamsize size = f.tellg();
			std::cout << "size: " << size << std::endl;
			if (size >= 384) {
				size = 384;
				f.seekg(0, std::ios::beg);
				std::vector<char> buffer(size);
				if (f.read(buffer.data(), size))
				{
					/* worked! */
					myFile.write(&buffer[0], size);
				}
			}
		}
	}

}


#ifdef DEF_LIB_ZLIB

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}
void test_compress(
	Byte *data, uLong len,
	Byte *compr, uLong &comprLen)
{
	int err;
	err = compress(compr, &comprLen, (const Bytef*)data, len);
	CHECK_ERR(err, "compress");

	//strcpy((char*)uncompr, "garbage");

	//err = uncompress(uncompr, &uncomprLen, compr, comprLen);
	//CHECK_ERR(err, "uncompress");
}




void compress_memory(void *in_data, size_t in_data_size, std::vector<uint8_t> &out_data)
{
	std::vector<uint8_t> buffer;

	const size_t BUFSIZE = 128 * 1024;
	uint8_t temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<uint8_t *>(in_data);
	strm.avail_in = in_data_size;
	strm.next_out = temp_buffer;
	strm.avail_out = BUFSIZE;

	deflateInit(&strm, Z_BEST_COMPRESSION);

	while (strm.avail_in != 0)
	{
		int res = deflate(&strm, Z_NO_FLUSH);
		assert(res == Z_OK);
		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
	}

	int deflate_res = Z_OK;
	while (deflate_res == Z_OK)
	{
		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
		deflate_res = deflate(&strm, Z_FINISH);
	}

	assert(deflate_res == Z_STREAM_END);
	buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
	deflateEnd(&strm);

	out_data.swap(buffer);
}
#endif // DEF_LIB_ZLIB

int record_rgbxyz() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}


#ifdef DEF_LIB_ZLIB
	static const char* myVersion = ZLIB_VERSION;
	if (zlibVersion()[0] != myVersion[0]) {
		fprintf(stderr, "incompatible zlib version\n");
		exit(1);

	}
	else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
		fprintf(stderr, "warning: different zlib version\n");
	}

	printf("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
		ZLIB_VERSION, ZLIB_VERNUM, zlibCompileFlags());
#endif // DEF_LIB_ZLIB

	storedata::RawRecorder pr;
	pr.setup("data\\record_", 10000000, 100);

	//Byte* compress = nullptr;
	//uLong compress_len = 0;

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		cv::Mat tmp(curr.size(), CV_32FC3);

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		size_t num_items = 2;
		size_t s0 = curr.cols;
		size_t s1 = curr.rows;
		std::vector<uint8_t> items(sizeof(num_items)); /*space must be allocated at this point*/
		::memcpy(items.data(), &num_items, sizeof(num_items));
		std::vector<uint8_t> head0(sizeof(s0)); /*space must be allocated at this point*/
		::memcpy(head0.data(), &s0, sizeof(s0));
		std::vector<uint8_t> head1(sizeof(s1)); /*space must be allocated at this point*/
		::memcpy(head1.data(), &s1, sizeof(s1));
		// copy all in the buffer
		std::vector<uint8_t> buf_fin;
		for (size_t i = 0; i < sizeof(num_items); ++i) {
			buf_fin.push_back(items[i]);
		}
		for (size_t i = 0; i < head0.size(); ++i) {
			buf_fin.push_back(head0[i]);
		}
		for (size_t i = 0; i < head1.size(); ++i) {
			buf_fin.push_back(head1[i]);
		}
		for (size_t i = 0; i < curr.cols * curr.rows * curr.channels(); ++i) {
			buf_fin.push_back(curr.data[i]);
		}
		for (size_t i = 0; i < sizeof(float) * tmp.cols * tmp.rows * tmp.channels(); ++i) {
			buf_fin.push_back(tmp.data[i]);
		}

#ifdef DEF_LIB_ZLIB
		std::vector<uint8_t> buf_fin_compress;
		compress_memory(buf_fin.data(), buf_fin.size(), buf_fin_compress);
		// record the data
		pr.record(buf_fin_compress);
#else
		pr.record(buf_fin);
#endif // DEF_LIB_ZLIB

		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "duration: " << duration << std::endl;

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}


/** @brief Multithread version to record the data
*/
int record_rgbxyz_multithread() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}


#ifdef DEF_LIB_ZLIB
	static const char* myVersion = ZLIB_VERSION;
	if (zlibVersion()[0] != myVersion[0]) {
		fprintf(stderr, "incompatible zlib version\n");
		exit(1);

	}
	else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
		fprintf(stderr, "warning: different zlib version\n");
	}

	printf("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
		ZLIB_VERSION, ZLIB_VERNUM, zlibCompileFlags());
#endif // DEF_LIB_ZLIB

	storedata::RawRecorder pr;
	pr.setup("data\\record_", 100000000, 100);

	//Byte* compress = nullptr;
	//uLong compress_len = 0;
	bool is_running = false;
	cv::Mat curr, tmp;

	bool do_continue = true;
	while (do_continue) //Show the image captured in the window and repeat
	{
		cv::Mat src;
		vc >> src;
		if (src.empty()) continue;


		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		if (!is_running) {
			is_running = true;
			curr = src.clone();
			tmp = cv::Mat(curr.size(), CV_32FC3);
			std::thread t([&] {
				std::cout << "thread function\n";

				size_t num_items = 2;
				size_t s0 = curr.cols;
				size_t s1 = curr.rows;
				std::vector<uint8_t> items(sizeof(num_items)); /*space must be allocated at this point*/
				::memcpy(items.data(), &num_items, sizeof(num_items));
				std::vector<uint8_t> head0(sizeof(s0)); /*space must be allocated at this point*/
				::memcpy(head0.data(), &s0, sizeof(s0));
				std::vector<uint8_t> head1(sizeof(s1)); /*space must be allocated at this point*/
				::memcpy(head1.data(), &s1, sizeof(s1));
				// copy all in the buffer
				std::vector<uint8_t> buf_fin;
				for (size_t i = 0; i < sizeof(num_items); ++i) {
					buf_fin.push_back(items[i]);
				}
				for (size_t i = 0; i < head0.size(); ++i) {
					buf_fin.push_back(head0[i]);
				}
				for (size_t i = 0; i < head1.size(); ++i) {
					buf_fin.push_back(head1[i]);
				}
				for (size_t i = 0; i < curr.cols * curr.rows * curr.channels(); ++i) {
					buf_fin.push_back(curr.data[i]);
				}
				for (size_t i = 0; i < sizeof(float) * tmp.cols * tmp.rows * tmp.channels(); ++i) {
					buf_fin.push_back(tmp.data[i]);
				}

#ifdef DEF_LIB_ZLIB
				std::vector<uint8_t> buf_fin_compress;
				compress_memory(buf_fin.data(), buf_fin.size(), buf_fin_compress);
				// record the data
				pr.record(buf_fin_compress);
#else
				pr.record(buf_fin);
#endif // DEF_LIB_ZLIB

				is_running = false;
			});
			t.detach();
		}

		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		//std::cout << "duration: " << duration << std::endl;

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", src);
		if (cv::waitKey(1) == 27) do_continue = false;

		if (!do_continue) {
			while (is_running) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));
				continue;
			}
		}
	}
	return 0;
}



} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[], char *window_name)
{
	// code to read Komatsu robot data
	//create_imagesource();
	//return 0;
	//create_robotfile();
	//return 0;

	create_folder("data");

	// example to read the dat file
	if (false) {
		if (argc < 2) {
			std::cout << "expected: " << argv[0] << " <fname_dat>" << std::endl;
			return 0;
		}
		// Record a video and save some simple data
		storedata::PlayerRecorder pr;
		unsigned int index_start = 0;
		pr.unpack(argv[1], 60,
			"data\\", index_start);

		std::cout << "Open the dat file unpacked" << std::endl;
		UDPKomatsu2Arm udp_komatsu_2arm;

		// read the txt files
		for (int i = 0; i < 100; ++i) {
			cv::Mat img = cv::imread("data\\" + std::to_string(i) + ".jpg");
			if (img.empty()) continue;
			std::ifstream f("data\\" + std::to_string(i) + ".txt", std::ios::binary | std::ios::ate);
			if (f.is_open()) {
				std::cout << "file: " << i << std::endl;
				std::streamsize size = f.tellg();
				std::cout << "size: " << size << std::endl;
				if (size >= 384) {
					size = 384;
					f.seekg(0, std::ios::beg);
					std::vector<char> buffer(size);
					if (f.read(buffer.data(), size))
					{
						udp_komatsu_2arm.convert(buffer.data(), size);
					}
				}
			}
			cv::imshow("img", img);
			cv::waitKey(1);
		}


		return 0;
	}


	// example to save video async
	if (false) {
		record3();
		return 0;
	}

	// example to save/read video and message in binary file
	if (false) {
		// record a video
		record();
		// Record a video and save some simple data
		//PlayerRecorder pr;
		//pr.play("data\\record_2018-10-15.05_06_05.dat", 60);
		
		//pr.play("D:\\workspace\\work\\ImPACT\\dev\\BirdView\\vs2015x64\\sample\\record\\record_2018-10-16.03_00_42.dat", 60);
		//unsigned int index_start = 0;
		//pr.unpack("C:\\Users\\Moro\\Downloads\\20181024_impact_test\\record_2018-10-24.03_22_36.dat", 60,
		//	"D:\\workspace\\work\\ImPACT\\dev\\UDPKomatsu2Arm\\data\\", index_start);
	}

	// record a video with a raw data saver
	if (false)
	{
		record2();
		// Record a video and save some simple data
		storedata::RawRecorder pr;
		pr.play("D:\\workspace\\work\\NHK\\D1Data\\D1Viewer\\vs2015x64\\code\\data\\record_2018-06-01.16_02_37.dat", 60);
	}

	if (true) {
		//record_rgbxyz();
		record_rgbxyz_multithread();
		//RawRecorder pr;
		//pr.play_raw("D:\\workspace\\university\\chuo\\projects\\HumanDetectionRGBD\\src\\ZEDCapture\\build\\data\\record_2018-11-28.03_22_52.dat", 60);

	}

	return 0;
}
