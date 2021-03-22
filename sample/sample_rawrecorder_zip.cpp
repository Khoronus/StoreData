/* @file sample_rawrecord.cpp
 * @brief Example of recording data in raw format and compressed (zip).
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
 * @version 0.1.1.0
 * 
 */


#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>

#include <opencv2/opencv.hpp>

//#define BOOST_BUILD
//#define BOOST_NO_CXX11_SCOPED_ENUMS
//#include <boost/filesystem.hpp>
//#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include "record/record_headers.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief It creates a folder if necessary
*/
void create_folder(const std::string &folder) {
	std::filesystem::path dir(folder);
	if (std::filesystem::create_directory(dir)) {
		std::cout << "[+] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
	else {
		std::cout << "[-] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
}


/** @brief Change the name of the file.

Function from the callback
*/
std::string global_fname;
void name_changed(const std::string &fname) {
	std::cout << "name_changed: " << fname << std::endl;
	global_fname = fname;
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



/** @brief Compress some memory data (ZLIB)
*/
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

/** @brief Record 
*/
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
	pr.set_callback_createfile(std::bind(&name_changed,
		std::placeholders::_1));
	pr.setup("data\\record_", ".dat", 10000000, 100);

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
	pr.set_callback_createfile(std::bind(&name_changed,
		std::placeholders::_1));
	pr.setup("data\\record_", ".dat", 100000000, 100);

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
int main(int argc, char *argv[])
{
	std::cout << "Extract the code to read from zip file from the ";
	std::cout << "RawRecorder class or pass a functor for data processing"
		<< std::endl;

	{
		record_rgbxyz();
		storedata::RawRecorder pr;
		pr.read_all_raw_compressed("data\\record_" + global_fname + ".dat", 60);
	}
	{
		record_rgbxyz_multithread();
		storedata::RawRecorder pr;
		pr.read_all_raw_compressed("data\\record_" + global_fname + ".dat", 60);
	}
	return 0;
}
