/* @file PlayerRecorder.cpp
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
 * @version 0.2.0.0
 * 
 */

#include "record/inc/record/PlayerRecorder.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
void PlayerRecorder::setup(const std::string &filename,
	int max_memory_allocable, int fps) {
	std::cout << "PlayerRecorder::setup" << std::endl;
	std::cout << "insert FileGeneratorParam" << std::endl;
	fgp_.insert(std::make_pair(0, FileGeneratorParams()));
	std::cout << "filename: " << filename << std::endl;
	fgp_[0].set_filename(filename);
	std::cout << "PlayerRecorder::setup:" << max_memory_allocable << " " << 
		fps << std::endl;
	fgm_.setup(max_memory_allocable, fgp_, fps);
}
// ----------------------------------------------------------------------------
void PlayerRecorder::setup_video(std::map<int, cv::Mat> &sources,
	const std::string &filename,
	int max_frames_allocable, int fps) {
	std::cout << "PlayerRecorder::setup_video" << std::endl;
	std::cout << "insert VideoGeneratorParam" << std::endl;
	for (auto &it : sources) {
		//std::cout << "source: " << it.first << " " << it.second.size() << 
		//	std::endl;
		int width = it.second.cols, height = it.second.rows,
			framerate = fps;
		vgp_.insert(std::make_pair(it.first, VideoGeneratorParams()));
		vgp_[it.first].set_filename(filename);
		vgp_[it.first].set_width(width); 
		vgp_[it.first].set_height(height);
	}
	std::cout << "PlayerRecorder::setup:" << max_frames_allocable << " " <<
		fps << std::endl;
	vgm_.setup(max_frames_allocable, vgp_, fps);
}
// ----------------------------------------------------------------------------
void PlayerRecorder::setup_metaframe(cv::Mat &meta_frame) {
	vgm_.setup_metaframe(meta_frame);
}
// ----------------------------------------------------------------------------
void PlayerRecorder::record(cv::Mat &curr, bool encoded, std::string &msg) {

	if (fgm_.under_writing()) return;

	// Container with the buffer data of the encoded image.
	std::vector< std::vector< uchar > > v_buffer_;

	int codified = 0;
	size_t size_img_data = curr.cols * curr.rows * curr.channels();
	uchar *data = curr.data;
	if (curr.empty()) size_img_data = 0;

	// Encode the source image
	if (encoded && !curr.empty()) {
		codified = 1;
		std::vector<int> params(2);
#if CV_MAJOR_VERSION == 4
		params[0] = cv::IMWRITE_JPEG_QUALITY;
#else
		params[0] = CV_IMWRITE_JPEG_QUALITY;
#endif
		params[1] = 100;//CV_IMWRITE_JPEG_QUALITY;
		v_buffer_.clear();
		v_buffer_.push_back( std::vector<uchar>() );
		cv::imencode(".jpg", curr, v_buffer_[ v_buffer_.size() - 1 ], params);
		// Image data size
		size_img_data = v_buffer_[ v_buffer_.size() - 1 ].size(); 
		data = &v_buffer_[ v_buffer_.size() - 1 ][0];
	}

	// Add the information to transmit
	size_t size_msg_data = msg.size();
	std::vector<char> info_to_transmit(size_msg_data);
	memcpy(&info_to_transmit[0], &msg[0], size_msg_data);

	// Prepare the container for the data to transmit
	std::map<int, std::vector<char> > m_data;
	int imagedatasize_bytes = 24; // IT MUST BE SAME LATER
			                        // Cols, Rows, Channels, Data size, Message size
	m_data[0] = std::vector<char>(size_img_data + size_msg_data + 
		imagedatasize_bytes);

	// Copy the data
	int channels = curr.channels();
	memcpy(&m_data[0][0], &codified, sizeof(int));
	memcpy(&m_data[0][4], &curr.cols, sizeof(int));
	memcpy(&m_data[0][8], &curr.rows, sizeof(int));
	memcpy(&m_data[0][12], &channels, sizeof(int));
	memcpy(&m_data[0][16], &size_img_data, sizeof(int));
	memcpy(&m_data[0][20], &size_msg_data, sizeof(int));
	memcpy(&m_data[0][24], data, size_img_data);
	memcpy(&m_data[0][24 + size_img_data], &info_to_transmit[0], 
		size_msg_data);
	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
}
// ----------------------------------------------------------------------------
void PlayerRecorder::record(
	cv::Mat &curr, 
	bool encoded, 
	unsigned char *msg, 
	size_t msg_size) {

	if (fgm_.under_writing()) return;

	// Container with the buffer data of the encoded image.
	std::vector< std::vector< uchar > > v_buffer_;

	int codified = 0;
	size_t size_img_data = curr.cols * curr.rows * curr.channels();
	uchar *data = curr.data;
	if (curr.empty()) size_img_data = 0;

	// Encode the source image
	if (encoded && !curr.empty()) {
		codified = 1;
		std::vector<int> params(2);
#if CV_MAJOR_VERSION == 4
		params[0] = cv::IMWRITE_JPEG_QUALITY;
#else
		params[0] = CV_IMWRITE_JPEG_QUALITY;
#endif
		params[1] = 100;//CV_IMWRITE_JPEG_QUALITY;
		v_buffer_.clear();
		v_buffer_.push_back(std::vector<uchar>());
		cv::imencode(".jpg", curr, v_buffer_[v_buffer_.size() - 1], params);
		// Image data size
		size_img_data = v_buffer_[v_buffer_.size() - 1].size();
		data = &v_buffer_[v_buffer_.size() - 1][0];
	}

	// Add the information to transmit
	size_t size_msg_data = msg_size;
	std::vector<char> info_to_transmit(size_msg_data);
	memcpy(&info_to_transmit[0], &msg[0], size_msg_data);

	// Prepare the container for the data to transmit
	std::map<int, std::vector<char> > m_data;
	// The size of the data header.
	int imagedatasize_bytes = 24; // IT MUST BE SAME LATER
								  // Cols, Rows, Channels, Data size, Message size
	m_data[0] = std::vector<char>(size_img_data + size_msg_data +
		imagedatasize_bytes);

	// Copy the data
	int channels = curr.channels();
	memcpy(&m_data[0][0], &codified, sizeof(int));
	memcpy(&m_data[0][4], &curr.cols, sizeof(int));
	memcpy(&m_data[0][8], &curr.rows, sizeof(int));
	memcpy(&m_data[0][12], &channels, sizeof(int));
	memcpy(&m_data[0][16], &size_img_data, sizeof(int));
	memcpy(&m_data[0][20], &size_msg_data, sizeof(int));
	memcpy(&m_data[0][24], data, size_img_data);
	memcpy(&m_data[0][24 + size_img_data], &info_to_transmit[0],
		size_msg_data);
	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
}
// ----------------------------------------------------------------------------
void PlayerRecorder::record_video(std::map<int, cv::Mat> &sources) {
	vgm_.push_data(sources);
}
// ----------------------------------------------------------------------------
void PlayerRecorder::play(const std::string &filename, int FPS) {
	int _FPS = (std::max)(1, FPS);
	// Read the data
	bool bPlayRecord = true;
	std::ifstream file(filename.c_str(),
		std::ios::in | std::ios::binary | std::ios::ate);
	int size = 0;
	char *memblock = nullptr;
	std::vector< std::pair<cv::Mat, std::vector<char> > > img_info;
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();
		data2data_type(memblock, size, img_info);
		delete[] memblock;
	}

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = img_info.begin(); it != img_info.end(); it++)
#else
	for (std::vector< std::pair<cv::Mat, std::vector<char> > >::const_iterator it = img_info.begin(); it != img_info.end(); it++)
#endif		
	{
		// show the image
		if (!it->first.empty()) cv::imshow("record", it->first);
		// show the text result
		char msg[2048];
		for (int i = 0; i < it->second.size(); i++)
			msg[i] = it->second[i];
		msg[it->second.size()] = '\0';
		std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;

		//// specialized
		//memcpy(msg, &(it->second[0]), 384);
		//msg[384] = '\0';
		//std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;
		//memcpy(msg, &(it->second[384]), it->second.size() - 384);
		//msg[it->second.size() - 384] = '\0';
		//std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;
		cv::waitKey(1000 / _FPS);
	}
}
// ----------------------------------------------------------------------------
void PlayerRecorder::unpack(
	const std::string &filename, int FPS,
	const std::string &path, unsigned int &index_start) {
	int _FPS = (std::max)(1, FPS);
	// Read the data
	bool bPlayRecord = true;
	std::ifstream file(filename.c_str(),
		std::ios::in | std::ios::binary | std::ios::ate);
	int size = 0;
	char *memblock = nullptr;
	std::vector< std::pair<cv::Mat, std::vector<char> > > img_info;
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();
		data2data_type(memblock, size, img_info);
		delete[] memblock;
	}

	// save the data from this index
	unsigned int index_start_internal = index_start;

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = img_info.begin(); it != img_info.end(); it++)
#else
	for (std::vector< std::pair<cv::Mat, std::vector<char> > >::const_iterator it = img_info.begin(); it != img_info.end(); it++)
#endif		
	{
		// show the image
		if (!it->first.empty()) {
			cv::imshow("record", it->first);
			cv::imwrite(path + "\\" + std::to_string(index_start_internal) + ".jpg", it->first);
		}
		// show the text result
		char msg[2048];
		for (int i = 0; i < it->second.size(); i++)
			msg[i] = it->second[i];
		msg[it->second.size()] = '\0';
		std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;

		// save the message
		std::ofstream fout(path + "\\" + std::to_string(index_start_internal) + ".txt", std::ios::binary);
		fout.write(&it->second[0], it->second.size());
		fout.flush();

		//// specialized
		//memcpy(msg, &(it->second[0]), 384);
		//msg[384] = '\0';
		//std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;
		//memcpy(msg, &(it->second[384]), it->second.size() - 384);
		//msg[it->second.size() - 384] = '\0';
		//std::cout << "msg[" << it->second.size() << "]: " << msg << std::endl;
		cv::waitKey(1000 / _FPS);
		// increment the counter
		++index_start_internal;
	}
}
// ----------------------------------------------------------------------------
void PlayerRecorder::data2data_type(char *data, int maxsize, 
	std::vector< std::pair<cv::Mat, std::vector<char> > > &out)
{
	const int sizeofinfo = 24; // Expected information about the size
								// of the image
	int pos = 0;
	while (pos < maxsize) {
		std::cout << "read: " << pos << " " << maxsize << std::endl;
		// Get the image size
		char imginfo[sizeofinfo];
		if (pos + sizeofinfo < maxsize) {
			memcpy(imginfo, &data[pos], sizeofinfo);
		}
		int codified = 0, cols = 0, rows = 0, channels = 0, imgsize = 0, 
			msgsize = 0;
		memcpy(&codified, &imginfo[0], 4);
		memcpy(&cols, &imginfo[4], 4);
		memcpy(&rows, &imginfo[8], 4);
		memcpy(&channels, &imginfo[12], 4);
		memcpy(&imgsize, &imginfo[16], 4);
		memcpy(&msgsize, &imginfo[20], 4);
		if (pos + sizeofinfo + imgsize + msgsize < maxsize)
		{
			cv::Mat m;
			if (codified == 0) {
				int ch = channels == 3 ? CV_8UC3 : CV_8U;
				m = cv::Mat::zeros(rows, cols, ch);
				memcpy(m.data, &data[pos + sizeofinfo], imgsize);
			} else if (codified == 1) {
				std::vector< uchar > data_tmp(imgsize);
				memcpy(&data_tmp[0], &data[pos + sizeofinfo], imgsize);
				int flags = channels == 3 ? 1 : 0;
				m = cv::imdecode(data_tmp, flags);
			}
			std::vector< uchar > msg_tmp(msgsize);
			memcpy(&msg_tmp[0], &data[pos + sizeofinfo + imgsize], msgsize);
			std::vector<char> v(msgsize);
			for (int i = 0; i < msgsize; i++)
				v[i] = msg_tmp[i];
			out.push_back(std::make_pair(m, v));
		}
		pos += sizeofinfo + imgsize + msgsize;
	}
}
// ----------------------------------------------------------------------------
void PlayerRecorder::set_callback_createfile(
	cbk_fname_changed callback_createfile) {
	vgm_.set_callback_createfile(callback_createfile);
	fgm_.set_callback_createfile(callback_createfile);
}

} // namespace storedata