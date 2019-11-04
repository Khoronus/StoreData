/* @file sample_imgproc_codify_data.cpp
 * @brief Test the data codification over an image.
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
 * @version 0.1.0.0
 * 
 */



#include "codify/inc/codify/codify_headers.hpp"

namespace
{

void test()
{
	storedata::codify::CodifyData2Image::test();
}

}  // namespace anonymous


/** Main function
*/
void main()
{
	test();
}



#ifdef use_test_bit


template <typename _Ty>
void word2bit(_Ty word, std::vector< int > &bit)
{
	int s = sizeof(_Ty);
	for (int i = s - 1; i >= 0; --i)
	{
		_Ty mask = 1 << i;
		_Ty value = word & mask;
		bit.push_back((int)(value >> i));
	}
}

template <typename _Ty>
int bit2word(const std::vector< int > &bit, _Ty &word)
{
	int s = bit.size();
	int sw = sizeof(_Ty);
	if (s / 8 != sw) {
		std::cout << s << " " << sw << std::endl;
		return 0;
	}
	word = 0;
	for (int i = s - 1; i >= 0; --i)
	{
		word |= bit[i] << i;
	}
	return 1;
}

template <typename _Ty>
int bit2word(const std::vector< int > &bit, int index_start, int index_stop, 
	_Ty &word)
{
	int s = index_stop - index_start + 1;
	int sw = sizeof(_Ty);
	if (s / 8 != sw) {
		std::cout << s << " " << sw << std::endl;
		return 0;
	}
	word = 0;
	int idx = index_stop - index_start;
	for (int i = idx; i >= 0; --i)
	{
		word |= bit[i + index_start] << i;
	}
	return 1;
}



template <typename _Ty>
void data2bit(const _Ty *words, int length, std::vector< int > &bit) {
	for (int i = 0; i < length; i++)
	{
		word2bit<_Ty>(words[i], bit);
	}
}

template <typename _Ty>
void bit2data(std::vector< int > &bit, const _Ty *words, int max_length, int &length) {
	for (int i = 0; i < length; i++)
	{
		bit2word<_Ty>(bit, i * sizeof(_Ty), (i + 1) * sizeof(_Ty), words[i]);
	}
}


void bit2mask(const std::vector< int > &bit, cv::Mat &image, int index_start, 
	int &index_end)
{
	int maxsize = image.cols * image.rows * image.channels();
	int s = maxsize > bit.size() + index_start ? bit.size() + index_start : maxsize;
	for (int i = index_start; i < s; i++)
	{
		//std::cout << "i: " << (int)bit[i] << std::endl;
		image.data[i] = (char)bit[i];
	}
}

/** @brief It shift a source image of 1 bit for each byte and add the data.
*/
int compose_images(cv::Mat &src_to_shift, cv::Mat &data_to_add, cv::Mat &out) {
	if (src_to_shift.size() != data_to_add.size()) return 0;
	if (out.size() != src_to_shift.size() || out.type() != src_to_shift.type()) {
		out = cv::Mat(src_to_shift.size(), src_to_shift.type());
	}
	int s = src_to_shift.cols * src_to_shift.rows * src_to_shift.channels();
	for (int i = 0; i < s; i++) {
		out.data[i] = (src_to_shift.data[i] << 1);
	}
	out += data_to_add;

	return 1;
}

int extract_from_image(cv::Mat &src, cv::Mat &data_extracted) {
	if (src.size() != data_extracted.size() || src.type() != data_extracted.type()) {
		data_extracted = cv::Mat(src.size(), src.type());
	}
	int s = src.cols * src.rows * src.channels();
	int k = 32;
	for (int i = 0; i < 4; i++) {
		data_extracted.data[i] = src.data[i] & 1;
	}
	// Get the total size

	return 1;
}


void test()
{
	{
		std::vector<int> bit;
		bit.push_back(1);
		bit.push_back(0);
		bit.push_back(1);
		bit.push_back(0);
		bit.push_back(1);
		bit.push_back(0);
		bit.push_back(1);
		bit.push_back(1);
		unsigned char word;
		bit2word<unsigned char>(bit, 0, 7, word);
		std::cout << (int)word << std::endl;

	}
	return;

	std::string msg = "ciao come va";
	int size = msg.length();
	std::vector<int> bit;
	data2bit<int>(&size, 1, bit);
	data2bit<char>(msg.c_str(), msg.length(), bit);
	for (auto it = bit.begin(); it != bit.end(); it++)
		std::cout << *it;
	std::cout << std::endl;
	cv::Mat image(512, 512, CV_8UC3, cv::Scalar(0));
	int index_end = 0;
	bit2mask(bit, image, 0, index_end);

	cv::Mat m(512, 512, CV_8UC3, cv::Scalar::all(255));

	cv::Mat out;
	compose_images(m, image, out);

	// shift the image
	//cv::Mat m_offset(512, 512, CV_8UC3, cv::Scalar::all(2));
	//double min, max;
	//cv::minMaxLoc(m, &min, &max);
	//if (min < 2) min = 2;
	//if (max > 253) max = 253;
	//if (min > max) min = max;
	//std::cout << min << " " << max << std::endl;
	//m += m_offset;
	//cv::normalize(m, m, min, max, cv::NORM_MINMAX, CV_8UC1);
	cv::imshow("out", out);
	cv::imwrite("out.png", out);
	cv::waitKey();
}


#endif