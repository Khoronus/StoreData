/* @file empty_cmake.cpp
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
 * @author Alessandro Moro <alessandromoro.italy@google.com>
 * @bug No known bugs.
 * @version 0.2.0.0
 * 
 */

#include "record/inc/record/RawRecorder.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
RawRecorder::RawRecorder() {}
// ----------------------------------------------------------------------------
RawRecorder::~RawRecorder() {
	fgm_.close();
}
// ----------------------------------------------------------------------------
void RawRecorder::setup(const std::string &filename,
	int max_memory_allocable, int fps) {
	fgp_[0].set_filename(filename);
	fgm_.setup(max_memory_allocable, fgp_, fps);
}
// ----------------------------------------------------------------------------
void RawRecorder::record(uint8_t* data, size_t len) {
	// Add the information to transmit
	size_t size_msg_data = len;
	std::vector<char> info_to_transmit(size_msg_data);
	memcpy(&info_to_transmit[0], data, size_msg_data);

	// Prepare the container for the data to transmit
	std::map<int, std::vector<char> > m_data;
	m_data[0] = std::vector<char>(size_msg_data + 4);

	// Copy the data
	memcpy(&m_data[0][0], &size_msg_data, sizeof(int));
	memcpy(&m_data[0][4], &info_to_transmit[0],
		size_msg_data);
	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
}
// ----------------------------------------------------------------------------
void RawRecorder::record(const std::vector<uint8_t> &data) {
	// Add the information to transmit
	size_t size_msg_data = data.size();
	std::vector<char> info_to_transmit(size_msg_data);
	memcpy(&info_to_transmit[0], &data[0], size_msg_data);

	// Prepare the container for the data to transmit
	std::map<int, std::vector<char> > m_data;
	m_data[0] = std::vector<char>(size_msg_data + 4);

	// Copy the data
	memcpy(&m_data[0][0], &size_msg_data, sizeof(int));
	memcpy(&m_data[0][4], &info_to_transmit[0],
		size_msg_data);
	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
}
// ----------------------------------------------------------------------------
void RawRecorder::record(const std::string &msg) {

	// Add the information to transmit
	size_t size_msg_data = msg.size();
	std::vector<char> info_to_transmit(size_msg_data);
	memcpy(&info_to_transmit[0], &msg[0], size_msg_data);

	// Prepare the container for the data to transmit
	std::map<int, std::vector<char> > m_data;
	m_data[0] = std::vector<char>(size_msg_data + 4);

	// Copy the data
	memcpy(&m_data[0][0], &size_msg_data, sizeof(int));
	memcpy(&m_data[0][4], &info_to_transmit[0], 
		size_msg_data);
	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
}
// ----------------------------------------------------------------------------
void RawRecorder::play(const std::string &filename, int FPS) {
	int _FPS = (std::max)(1, FPS);
	// Read the data
	bool bPlayRecord = true;
	std::ifstream file(filename.c_str(),
		std::ios::in | std::ios::binary | std::ios::ate);
	int size = 0;
	char *memblock = nullptr;
	std::vector< std::vector<uint8_t> > data_info;
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();
		data2data_type(memblock, size, data_info);
		delete[] memblock;
	}

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = data_info.begin(); it != data_info.end(); it++)
#else
	for (std::vector< std::vector<char> >::const_iterator it = data_info.begin(); it != data_info.end(); it++)
#endif		
	{
		char msg[1024];
		for (int i = 0; i < it->size(); i++)
			msg[i] = (*it)[i];
		msg[it->size()] = '\0';
		std::cout << "msg: " << msg << std::endl;
	}
}
// ----------------------------------------------------------------------------
void RawRecorder::play_raw(const std::string &filename, int FPS) {
	int _FPS = (std::max)(1, FPS);
	// Read the data
	bool bPlayRecord = true;
	std::ifstream file(filename.c_str(),
		std::ios::in | std::ios::binary | std::ios::ate);
	int size = 0;
	char *memblock = nullptr;
	std::vector< std::vector<uint8_t> > data_info;
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();
		data2data_type(memblock, size, data_info);
		delete[] memblock;
	}

	std::cout << "Found: " << data_info.size() << " frames" << std::endl;

#ifdef DEF_LIB_ZLIB
	// allocate the space for the compressed memory
	uLong uncomprLen = 5000000;
	Byte* uncom = (Byte*)calloc((uInt)uncomprLen, 1);

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = data_info.begin(); it != data_info.end(); it++)
#else
	for (std::vector< std::vector<char> >::const_iterator it = data_info.begin(); it != data_info.end(); it++)
#endif		
	{
		// uncompress
		int err = uncompress(uncom, &uncomprLen, it->data(), 
			static_cast<uLong>(it->size()));
		if (err != Z_OK) {
			fprintf(stderr, "error: %d\n", err);
			exit(1);
		} 

		// analyze the data
		size_t num_items = 0;
		size_t s0 = 0;
		size_t s1 = 0;
		size_t data_size = 0;
		memcpy(&num_items, &(uncom[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);
		memcpy(&s0, &(uncom[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);
		memcpy(&s1, &(uncom[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);

		std::cout << "#items: " << num_items << " " << s0 << " " << s1 << std::endl;

		cv::Mat img0(s1, s0, CV_8UC3, cv::Scalar::all(0));
		memcpy(img0.data, &(uncom[data_size]), sizeof(uchar) * s0 * s1 * img0.channels());
		data_size += sizeof(uchar) * s0 * s1 * img0.channels();
		cv::Mat img1(s1, s0, CV_32FC4, cv::Scalar::all(0));
		memcpy(img1.data, &(uncom[data_size]), sizeof(float) * s0 * s1 * img1.channels());
		data_size += sizeof(float) * s0 * s1 * img1.channels();

		cv::imshow("img0", img0);
		cv::imshow("img1", img1);
		cv::waitKey();


		//size_t num_items = 0;
		//size_t s0 = 0;
		//size_t s1 = 0;
		//size_t data_size = 0;
		//memcpy(&num_items, &(it->data()[data_size]), sizeof(size_t));
		//data_size += sizeof(size_t);
		//memcpy(&s0, &(it->data()[data_size]), sizeof(size_t));
		//data_size += sizeof(size_t);
		//memcpy(&s1, &(it->data()[data_size]), sizeof(size_t));
		//data_size += sizeof(size_t);

		//std::cout << "#items: " << num_items << " " << s0 << " " << s1 << std::endl;

		//cv::Mat img0(s1, s0, CV_8UC3, cv::Scalar::all(0));
		//memcpy(img0.data, &(it->data()[data_size]), sizeof(uchar) * s0 * s1 * img0.channels());
		//data_size += sizeof(uchar) * s0 * s1 * img0.channels();
		//cv::Mat img1(s1, s0, CV_32FC3, cv::Scalar::all(0));
		//memcpy(img1.data, &(it->data()[data_size]), sizeof(float) * s0 * s1 * img1.channels());
		//data_size += sizeof(float) * s0 * s1 * img1.channels();

		//cv::imshow("img0", img0);
		//cv::imshow("img1", img1);
		//cv::waitKey();
	}

	// free the allocated memory
	free(uncom);
#else

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = data_info.begin(); it != data_info.end(); it++)
#else
	for (std::vector< std::vector<char> >::const_iterator it = data_info.begin(); it != data_info.end(); it++)
#endif		
	{
		// analyze the data
		size_t num_items = 0;
		size_t s0 = 0;
		size_t s1 = 0;
		size_t data_size = 0;
		memcpy(&num_items, &(it->data()[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);
		memcpy(&s0, &(it->data()[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);
		memcpy(&s1, &(it->data()[data_size]), sizeof(size_t));
		data_size += sizeof(size_t);

		std::cout << "#items: " << num_items << " " << s0 << " " << s1 << std::endl;

		cv::Mat img0(s1, s0, CV_8UC3, cv::Scalar::all(0));
		memcpy(img0.data, &(it->data()[data_size]), sizeof(uchar) * s0 * s1 * img0.channels());
		data_size += sizeof(uchar) * s0 * s1 * img0.channels();
		cv::Mat img1(s1, s0, CV_32FC3, cv::Scalar::all(0));
		memcpy(img1.data, &(it->data()[data_size]), sizeof(float) * s0 * s1 * img1.channels());
		data_size += sizeof(float) * s0 * s1 * img1.channels();

		cv::imshow("img0", img0);
		cv::imshow("img1", img1);
		cv::waitKey();
	}

#endif
}
// ----------------------------------------------------------------------------
void RawRecorder::data2data_type(char *data, int maxsize,
	std::vector< std::vector<uint8_t> > &out)
{
	int pos = 0;
	while (pos < maxsize) {
		//std::cout << "read: " << pos << " " << maxsize << std::endl;
		int msgsize = 0;
		memcpy(&msgsize, &data[pos], 4);
		pos += 4;
		if (msgsize < maxsize)
		{
			std::vector< uchar > msg_tmp(msgsize);
			memcpy(&msg_tmp[0], &data[pos], msgsize);
			std::vector<uint8_t> v(msgsize);
			for (int i = 0; i < msgsize; i++)
				v[i] = msg_tmp[i];
			out.push_back(v);
		}
		pos += msgsize;
	}
}
// ----------------------------------------------------------------------------
void RawRecorder::set_callback_createfile(
	cbk_fname_changed callback_createfile) {
	fgm_.set_callback_createfile(callback_createfile);
}

} // namespace storedata