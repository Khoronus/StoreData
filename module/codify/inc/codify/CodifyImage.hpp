/**
* @file CodifyImage.hpp
* @brief Performs simple template matching between images.
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

#ifndef RECORDDATA_CODIFY_CODIFYIMAGE_HPP__
#define RECORDDATA_CODIFY_CODIFYIMAGE_HPP__

#include <iostream>
#include <vector>
#include <map>
#include <bitset>

#include "opencv2/opencv.hpp"

#include "codify_defines.hpp"

namespace storedata
{
namespace codify
{

/** @brief It codifies some data inside an image
*/
class CodifyImage
{
public:

	/** @brief It estimates the data size and creates an image that will be
	           able to contain the information.

		It estimates the data size and creates an image that will be
	    able to contain the information.
		
		@param[in] img Source image (original)
		@param[in] msg_len_max_bytes maximum size in byte
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[out] img_data Portion of image that is able to contain the
		                     information. The image should be combined to
							 the Source outside.
	*/
	STOREDATA_CODIFY_EXPORT static void estimate_data_size(const cv::Mat &img,
		size_t msg_len_max_bytes, int k, int offset, cv::Mat &img_data);

	/** @brief It converts a set of strings in image
	
		It converts a set of strings in image.
		@param[in] msgs Container with the messages to convert.
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[inout] m Target image where to save the data.
		@param[inout] x Start x point
		@param[inout] y Start t point

		IMPORTANT: All the strings are coded TOGETHER. The function will not
		           preserve the information about how the strings are 
				   separated.
	*/
	STOREDATA_CODIFY_EXPORT static void merge_strings2image(
		const std::vector<std::string> &msgs, 
		int k, int offset,
		cv::Mat &m, int &x, int &y);

	/** @brief It converts a data buffer in image.

		It converts a data buffer in image.
		@param[in] data Container with the data to convert
		@param[in] len data length
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[inout] m Target image where to save the data.
		@param[inout] x Start x point
		@param[inout] y Start t point
	*/
	STOREDATA_CODIFY_EXPORT static void data2image(
		const unsigned char* data, size_t len, 
		int k, int offset, cv::Mat &m,
		int &x, int &y);

	/** @brief It converts an image in a string.

		It converts an image in a string.
		@param[in] m Target image to convert
		@param[inout] x Start x point
		@param[inout] y Start t point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[out] msg_out output string
	*/
	STOREDATA_CODIFY_EXPORT static void image2string(cv::Mat &m, int &x, 
		int &y, int k, int offset, std::string &msg_out);

	/** @brief It converts an image in a data frame.

		The image is expected to contains a set of black and white points.
		@param[in] m Source image
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[in] len_header Size of the header (cannot be bigger than size_t)
		@param[inout] data Information will be copied here
		@param[in] maxlen Maximum data size
		@param[out] len length of the data saved
		*/
	STOREDATA_CODIFY_EXPORT static void image2data(cv::Mat &m, int &x, int &y, 
		int k, int offset, size_t len_header, unsigned char* data, size_t maxlen,
		size_t &len);

private:

	/** @brief It converts an image in byte value.

		It extract the id with the maximum entries.
		In case of matrix with size > 1, the entries is equal to the principle
		of "the winner takes it all".
		@param[in] m Source
		@return The character with the maximum entries.
	*/
	static unsigned char image2byte(cv::Mat &m);

	/** @brief It convert a char in an image block.

		It convert a char in an image block.
		It creates a set of blocks (squares) of 1 square for each bit.

		@param[in] c character to convert
		@param[out] m image source where to write the data
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
	*/
	static void char2image_block(unsigned char c, cv::Mat &m, int &x, int &y,
		int k, int offset);

	/** @brief It convert an image block in a char.

		It convert an image block in a char.
		It creates a char from a set of blocks (squares) of 1 square for each bit.

		@param[in] m image source where to write the data
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point

		@return It return the reconstructed char
	*/
	static unsigned char image_block2char(cv::Mat &m, int &x, int &y, int k,
		int offset);

	/** @brief It convert a buffer in an image.

		It convert a buffer in an image.

		@param[in] msg The message to convert.
		@param[in] len The message length.
		@param[int] m image source where to write the data
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
	*/
	static void buffer2image(const unsigned char* msg, size_t len, cv::Mat &m,
		int &x, int &y, int k, int offset);

	/** @brief It convert a buffer in an image.

		It convert a buffer in an image.

		@param[in] msg The message to convert.
		@param[in] len The message length.
		@param[int] m image source where to write the data
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
	*/
	static void buffer2image(const char* msg, size_t len, cv::Mat &m, int &x,
		int &y, int k, int offset);

	/** @brief It convert an image in a buffer.

		It convert an image in a buffer.

		@param[in] len The message length.
		@param[int] m image source where to write the data
		@param[inout] x Start x point
		@param[inout] y Start y point
		@param[in] k Size of the point which contains information (square)
		@param[in] offset Distance between each point
		@param[out] msg The message to set 
		@param[in] maxlen The maximum buffer size.
		*/
	static void image2buffer(size_t len, cv::Mat &m, int &x, int &y, int k,
		int offset, unsigned char* msg, size_t maxlen);

};

} //namespace codify
} // namespace storedata

#endif  // RECORDDATA_CODIFY_CODIFYIMAGE_HPP__
