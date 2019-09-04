/**
* @file codify_data.cpp
* @brief Body of the classes of the header file.
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
* @author  Alessandro Moro <alessandromoro.italy@gmail.com>
* @bug No known bugs.
* @version 1.0.0.0
*
*/


#include "codify/inc/codify/codify_data.hpp"

namespace storedata
{
namespace codify
{

//-----------------------------------------------------------------------------
int CodifyData2Image::data2image(cv::Mat &image, const char *data, int length,
	int index_start, int &index_end)
{
	if (index_start < 0) return 0;
	int s = length;
	int cn = image.channels();
	index_end = index_start + s + 4;
	int max_size = image.cols * image.rows * image.channels();
	if (index_start >= max_size) return 0;
	if (index_end >= max_size) return 0;
	memcpy((image.data + index_start), &s, 4);
	memcpy((image.data + index_start + 4), data, s);

	return 1;
}
//-----------------------------------------------------------------------------
int CodifyData2Image::image2message(const cv::Mat &image, int index_start, 
	int &index_end, char *data, int length)
{
	if (index_start < 0) return 0;
	// convert the image to string
	int s = 0;
	memcpy(&s, (image.data + index_start), 4);
	int max_size = image.cols * image.rows * image.channels();
	if (index_start + 4 + s >= max_size) {
		index_end = 0;
		return 0;
	}
	if (length > s + 1) {
		memcpy(data, (image.data + index_start + 4), s);
		data[s] = '\0';
		index_end = index_start + s + 4;
	}
	return 1;
}
//-----------------------------------------------------------------------------
int CodifyData2Image::message2image(cv::Mat &image, const std::string &message,
	int index_start, int &index_end)
{
	if (index_start < 0) return 0;
	int s = message.length();
	int cn = image.channels();
	index_end = index_start + s + 4;
	int max_size = image.cols * image.rows * image.channels();
	if (index_start >= max_size) return 0;
	if (index_end >= max_size) return 0;
	memcpy((image.data + index_start), &s, 4);
	memcpy((image.data + index_start + 4), message.c_str(), s);

	return 1;
}
//-----------------------------------------------------------------------------
int CodifyData2Image::image2message(const cv::Mat &image, int index_start, 
	int &index_end, std::string &msg)
{
	if (index_start < 0) return 0;
	// convert the image to string
	int s = 0;
	memcpy(&s, (image.data + index_start), 4);
	int max_size = image.cols * image.rows * image.channels();
	if (index_start + 4 + s >= max_size) {
		index_end = 0;
		return 0;
	}
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	std::unique_ptr<char[]> buf(new char[s + 1]);
	memcpy(buf.get(), (image.data + index_start + 4), s);
	buf[s] = '\0';
	msg = buf.get();
#else
	char *buf = new char[s + 1];
	memcpy(buf, (image.data + index_start + 4), s);
	buf[s] = '\0';
	msg = buf;
	delete buf; buf = NULL;
#endif
	index_end = index_start + s + 4;
	return 1;
}
//-----------------------------------------------------------------------------
void CodifyData2Image::message2image(cv::Mat &image, 
	const std::vector< std::string > &msg, int x, int y)
{
	int cn = image.channels();
	int index = y * image.cols * cn + x * cn; // +0 B, + 1 G, +2 R
	int index_start = index + 4;
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = msg.begin(); it != msg.end(); it++)
#else
	for (std::vector< std::string >::const_iterator it = msg.begin(); it != msg.end(); it++)
#endif
	{
		int index_end = 0;
		if (!data2image(image, it->c_str(), it->length(), index_start,
			index_end)) break;
		index_start = index_end;
	}
	// set the memorized byte size to the first 4 bytes
	int max_size = image.cols * image.rows * image.channels();
	if (index < max_size) {
		int size = index_start - index + 4;
		memcpy(image.data + index, &size, 4);
	}
}
//-----------------------------------------------------------------------------
void CodifyData2Image::image2message(const cv::Mat &image, int x, int y,
	std::vector< std::string > &msg)
{
	msg.clear();
	int cn = image.channels();
	int index = y * image.cols * cn + x * cn; // +0 B, + 1 G, +2 R
	int max_size = image.cols * image.rows * image.channels();
	if (index < max_size) {

		// get the memorized byte size to the first 4 bytes
		int size = 0;
		memcpy(&size, image.data + index, 4);
		//std::cout << "size: " << size << std::endl;
		// Get the data
		int index_start = index + 4;
		while (index_start != 0 && index_start - index < size) {
			//std::string tmp;
			int index_end = 0;
			char tmp[1024];
			image2message(image, index_start, index_end, tmp, 1024);
			//image2message(image, index_start, index_end, tmp);
			index_start = index_end;
			msg.push_back(tmp);
			//std::cout << tmp << std::endl;
			//std::cout << index_start - index << " -> " << size << std::endl;
		}
	}
}
//-----------------------------------------------------------------------------
int CodifyData2Image::expected_size(const std::vector<std::string> &msg) {
	int s = 0;
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = msg.begin(); it != msg.end(); it++)
#else
	for (std::vector<std::string>::const_iterator it = msg.begin(); 
		it != msg.end(); it++)
#endif
	{
		s += 4 + it->length();
	}
	return s;
}
//-----------------------------------------------------------------------------
void CodifyData2Image::insert_message(const cv::Mat &src, 
	const std::vector<std::string> &msg, cv::Mat &out) {

	int rows = expected_size(msg) / src.cols + 2;
	out = cv::Mat::zeros(cv::Size(src.cols, src.rows + rows), src.type());
	src.copyTo(out(cv::Rect(0, 0, src.cols, src.rows)));
	int x = 0, y = src.rows;
	message2image(out, msg, x, y);
}
//-----------------------------------------------------------------------------
void CodifyData2Image::test()
{
	cv::Mat m(512, 512, CV_8UC3, cv::Scalar(0, 255));
	std::vector< std::string > vmsg, testmsg;
	vmsg.push_back("This is an example");
	vmsg.push_back("of how");
	vmsg.push_back("a data is codified in an image $%&#?!");

	cv::Mat out;
	CodifyData2Image::insert_message(m, vmsg, out);
	CodifyData2Image::image2message(out, 0, m.rows, testmsg);

	std::cout << "<<< CODIFY >>>" << std::endl;
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = testmsg.begin(); it != testmsg.end(); it++)
#else
	for (std::vector< std::string >::const_iterator it = testmsg.begin(); 
		it != testmsg.end(); it++)
#endif
	{
		std::cout << *it << std::endl;
	}

	cv::imshow("out", out);
	cv::waitKey();
}


}	// namespace codify
}	// namespace storedata
