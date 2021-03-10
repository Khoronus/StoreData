/**
* @file CodifyImage.hpp
* @brief Body of the codify image.
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
* @oauthor Alessandro Moro <alessandromoro.italy@gmail.com>
* @bug No known bugs.
* @version 0.4.0.0
*
*/

#include "codify/inc/codify/CodifyImage.hpp"

namespace storedata
{
namespace codify
{

//-----------------------------------------------------------------------------
void CodifyImage::estimate_data_size(const cv::Mat &img,
	size_t msg_len_max_bytes, int k, int offset, cv::Mat &img_data
) {
	size_t rows = (msg_len_max_bytes * 4 * k + msg_len_max_bytes * 4 *
		offset + offset + 4) / img.cols + 4; // add two lines as buffer
	rows *= (k + offset + 1);
	img_data = cv::Mat(static_cast<int>(rows), img.cols, img.type(), 
		cv::Scalar::all(0));
}
//-----------------------------------------------------------------------------
void CodifyImage::merge_strings2image(const std::vector<std::string> &msgs, 
	int k, int offset, cv::Mat &m, int &x, int &y) {
	// convert the length in a information
	size_t len = 0;
	for (auto &it : msgs) {
		len += it.size();
	}
	std::vector<uint8_t> ucdata(sizeof(len)); /*space must be allocated at this point*/
	::memcpy(ucdata.data(), &len, sizeof(len));

	buffer2image(&ucdata[0], ucdata.size(), m, x, y, k, offset);
	for (auto &it : msgs) {
		buffer2image(it.c_str(), it.size(), m, x, y, k, offset);
	}
}
//-----------------------------------------------------------------------------
void CodifyImage::data2image(const unsigned char* data, size_t len, int k, 
	int offset, cv::Mat &m, int &x, int &y) {
	// convert the length in a information
	std::vector<uint8_t> ucdata(sizeof(len)); /*space must be allocated at this point*/
	::memcpy(ucdata.data(), &len, sizeof(len));
	buffer2image(&ucdata[0], ucdata.size(), m, x, y, k, offset);
	buffer2image(data, len, m, x, y, k, offset);
}
//-----------------------------------------------------------------------------
void CodifyImage::image2string(cv::Mat &m, int &x, int &y, int k, int offset,
	std::string &msg_out) {
	size_t len = 0;
	std::vector<uint8_t> msg_size(sizeof(len)); /*space must be allocated at this point*/
	image2buffer(sizeof(len), m, x, y, k, offset, &msg_size[0], msg_size.size());
	// convert back the chars in a int value
	memcpy(&len, &msg_size[0], sizeof(len));
	if (len < 0) return;
	//std::cout << "len: " << len << std::endl;
	// extract the message
	int idx = 0;
	unsigned char *msg = new unsigned char[len + 1];
	image2buffer(len, m, x, y, k, offset, msg, len);
	char *msg_fin = new char[len + 1];
	memcpy(msg_fin, msg, len);
	msg_fin[len] = '\0';
	msg_out = msg_fin;
	delete[] msg;
	delete[] msg_fin;
}
//-----------------------------------------------------------------------------
void CodifyImage::image2data(cv::Mat &m, int &x, int &y, int k, int offset,
	size_t len_header, unsigned char* data, size_t maxlen, size_t &len) {
	if (len_header > sizeof(len_header)) {
		std::cout << "[e] header too big: " << __FILE__ << " " << __LINE__ <<
			std::endl;
		return;
	}
	std::vector<uint8_t> msg_size(len_header); /*space must be allocated at this point*/
	image2buffer(len_header, m, x, y, k, offset, &msg_size[0], msg_size.size());
	// convert back the chars in a int value
	memcpy(&len, &msg_size[0], len_header);
	if (len < 0) return;
	//std::cout << "len: " << len << std::endl;
	// extract the message
	int idx = 0;
	image2buffer(len, m, x, y, k, offset, data, maxlen);
}
//-----------------------------------------------------------------------------
unsigned char CodifyImage::image2byte(const cv::Mat &m) {
	std::map<int, int> mm;
	for (int y = 0; y < m.rows; ++y) {
		for (int x = 0; x < m.cols; ++x) {
			mm[m.at<cv::Vec3b>(y, x)[0]]++;
		}
	}
	int best = -1;
	unsigned char id = 0;
	for (auto &it : mm) {
		if (it.second > best) {
			best = it.second;
			id = it.first;
		}
	}
	//cv::Scalar s = cv::sum(m);
	//char r = s[0] / (m.cols * m.rows);
	return id;
}
//-----------------------------------------------------------------------------
void CodifyImage::char2image_block(unsigned char c, cv::Mat &m, int &x, int &y,
	int k, int offset) {
	for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
	{
		unsigned char v = ((c >> i) & 1);
		//std::cout << "xykk " << x << " " << y << " " << k << std::endl;
		if (k > 1) {
			//byte2image(v * 255, m(cv::Rect(x, y, k, k)));
			m(cv::Rect(x, y, k, k)) = cv::Scalar::all(c);
		} else {
			//m.at<cv::Vec3b>(y, x) = cv::Vec3b(v * 255, v * 255, v * 255);
			m.at<cv::Vec3b>(y, x) = cv::Vec3b(v * 255, v * 255, v * 255);
		}
		x += k + offset;
		//std::cout << "x: " << x;
		//std::cout << " " << x + k << " " << k << " " << m.cols << std::endl;
		if (x + k + offset >= m.cols) {
			y += k + offset;
			x = offset;
		}
		if (y + k + offset >= m.rows) {
			break;
		}
	}
}
//-----------------------------------------------------------------------------
unsigned char CodifyImage::image_block2char(cv::Mat &m, int &x, int &y, 
	int k, int offset) {
	
	std::bitset<8> mybits;
	for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
	{
		unsigned char v = 0;
		if (k > 1) {
			v = image2byte(m(cv::Rect(x, y, k, k)));
		}
		else {
			v = m.at<cv::Vec3b>(y, x)[0];
		}
		v > 127 ? mybits[i] = 1 : mybits[i] = 0;
		x += k + offset;
		//std::cout << "x: " << x;
		//std::cout << " " << x + k << " " << k << " " << m.cols << std::endl;
		if (x + k + offset >= m.cols) {
			y += k + offset;
			x = offset;
		}
		if (y + k + offset >= m.rows) {
			break;
		}
	}
	unsigned long i = mybits.to_ulong();
	unsigned char c = static_cast<unsigned char>(i);
	return c;
}
//-----------------------------------------------------------------------------
void CodifyImage::buffer2image(const unsigned char* msg, size_t len, cv::Mat &m,
	int &x, int &y, int k, int offset) {
	for (size_t i = 0; i < len; ++i) {
		//byte2image(msg[i], m(cv::Rect(x, y, k, k)));
		char2image_block(msg[i], m, x, y, k, offset);

		x += k + offset;
		if (x + k + offset >= m.cols) {
			y += k + offset;
			x = offset;
		}
		if (y + k + offset >= m.rows) {
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void CodifyImage::buffer2image(const char* msg, size_t len, cv::Mat &m, int &x,
	int &y, int k, int offset) {
	for (size_t i = 0; i < len; ++i) {
		//byte2image(msg[i], m(cv::Rect(x, y, k, k)));
		//std::cout << "[" << i << "]: " << (int)msg[i] << std::endl;
		char2image_block(static_cast<char>(msg[i]), m, x, y, k, offset);

		x += k + offset;
		if (x + k + offset >= m.cols) {
			y += k + offset;
			x = offset;
		}
		if (y + k + offset >= m.rows) {
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void CodifyImage::image2buffer(size_t len, cv::Mat &m, int &x, int &y, int k,
	int offset, unsigned char* msg, size_t maxlen) {
	if (len <= maxlen) {
		for (size_t i = 0; i < len; ++i) {
			//msg[i] = image2byte(m(cv::Rect(x, y, k, k)));
			msg[i] = image_block2char(m, x, y, k, offset);

			x += k + offset;
			if (x + k + offset >= m.cols) {
				y += k + offset;
				x = offset;
			}
			if (y + k + offset >= m.rows) {
				break;
			}
		}
	}
}

} //namespace codify
} // namespace storedata
