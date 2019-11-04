/* @file codify_data.hpp
 * @brief Class to map data on an image.
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

#ifndef STOREDATA_CODIFY_CODIFYDATA_HPP__
#define STOREDATA_CODIFY_CODIFYDATA_HPP__


#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <limits>
#include <memory>

#include "opencv2/opencv.hpp"

#include "codify_defines.hpp"

namespace storedata
{
namespace codify
{

/** @brief Class to codify some data into an image.
*/
class STOREDATA_CODIFY_EXPORT CodifyData2Image
{
public:

	/** @brief Convert a data in a image.

		Convert a data in a image.
		@param[in] image Image to modify.
		@param[in] data Data to copy
		@param[in] length Length of the data to copy
		@param[in] index_start Position where to start to memorize the data
		@param[out] index_end Final position where the data is memorized
		@return Return 1 in case of success. 0 otherwise.
		@note The data is memorized as follow
		<4 bytes><n bytes (data)>
	*/
	static int data2image(cv::Mat &image, const char *data, int length,
		int index_start, int &index_end);

	/** @brief Convert an image in a message.

	Convert an image in a message.
	@param[in] image Image to read.
	@param[in] index_start Position where to start to memorize the data
	@param[out] index_end Final position where the data is memorized
	@param[in] data Data where to memorize.
	@param[in] length Length of the container where to memorize the data.
	@return Return 1 in case of success. 0 otherwise.
	@note The data is memorized as follow
	<4 bytes><n bytes (data)>
	*/
	static int image2message(const cv::Mat &image, int index_start, int &index_end,
		char *data, int length);

	/** @brief Convert a data in a image.

	Convert a data in a image.
	@param[in] image Image to modify.
	@param[in] message The message to memorize
	@param[in] index_start Position where to start to memorize the data
	@param[out] index_end Final position where the data is memorized
	@return Return 1 in case of success. 0 otherwise.
	@note The data is memorized as follow
	<4 bytes><n bytes (data)>
	*/
	static int message2image(cv::Mat &image, const std::string &message,
		int index_start, int &index_end);

	/** @brief Convert an image in a message.

	Convert an image in a message.
	@param[in] image Image to read.
	@param[in] index_start Position where to start to memorize the data
	@param[out] index_end Final position where the data is memorized
	@param[out] msg The message memorized.
	@return Return 1 in case of success. 0 otherwise.
	@note The data is memorized as follow
	<4 bytes><n bytes (data)>
	*/
	static int image2message(const cv::Mat &image, int index_start, int &index_end,
		std::string &msg);


	/** @brief Convert a data in a image.

	Convert a data in a image.
	@param[in] image Image to modify.
	@param[in] msg Container with the message to memorize.
	@param[in] x Coordinate on the image where to memorize the data.
	@param[in] y Coordinate on the image where to memorize the data.
	@return Return 1 in case of success. 0 otherwise.
	@note The data is memorized as follow
	<4 bytes><n bytes (data)>
	*/
	static void message2image(cv::Mat &image, const std::vector< std::string > &msg,
		int x, int y);

	/** @brief Convert an image in a message.

	Convert an image in a message.
	@param[in] image Image to read.
	@param[in] x Coordinate on the image where to read the data.
	@param[in] y Coordinate on the image where to read the data.
	@param[out] msg The message read.
	@return Return 1 in case of success. 0 otherwise.
	@note The data is memorized as follow
	<4 bytes><n bytes (data)>
	*/
	static void image2message(const cv::Mat &image, int x, int y,
		std::vector< std::string > &msg);

	/** @brief Expected number of bytes to memorize this message.

		Expected number of bytes to memorize this message.
		@param[in] msg The container with the data to memorized.
		@return Return the size to memorize.
	*/
	static int expected_size(const std::vector<std::string> &msg);

	/** @brief Insert a message on an image and create a new image.

		Insert a message on an image and create a new image.
		@param[in] src The image used as reference to memorize the image.
		@param[in] msg Container with the data to memorize.
		@param[out] msg The new image with the output data.
	*/
	static void insert_message(const cv::Mat &src, const std::vector<std::string> &msg,
		cv::Mat &out);

	/** @brief Example of the setting data in the image.
	*/
	static void test();

};


}	// namespace codify
}	// namespace storedata

#endif /* STOREDATA_CODIFY_CODIFYDATA_HPP__ */
