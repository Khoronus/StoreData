/* @file packunpack_images.hpp
 * @brief Class to pack or unpack a set of images.
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

#ifndef STOREDATA_CODIFY_PACKUNPACKIMAGES_HPP__
#define STOREDATA_CODIFY_PACKUNPACKIMAGES_HPP__


#include <vector>
#include <string>
#include <fstream>

#include "opencv2/opencv.hpp"

#include "codify_defines.hpp"

namespace storedata
{
namespace codify
{


/** @brief Class to pack or unpack a collection of images.

	It requires OpenCV.
*/
class PackUnpackImages
{
public:

	/** @brief Function to pack a set of images in a file.
	*/
	STOREDATA_CODIFY_EXPORT static int pack(const std::string &filename,
		std::vector<cv::Mat> &container, int maxsize,
		bool append);

	/** @brief Function to pack a set of images in a file.
	*/
	STOREDATA_CODIFY_EXPORT static int pack(const std::string &filename,
		char *data, int sizedata, int sizemax,
		bool append);

	/** @brief Function to pack a set of images in a file.
	*/
	STOREDATA_CODIFY_EXPORT static int pack(const std::string &filename,
		std::vector<char> &data, int sizemax,
		bool append);

	/** @brief Read from a binary file the memorized images.
	*/
	STOREDATA_CODIFY_EXPORT static int unpack(const std::string &filename,
		std::vector<cv::Mat> &container);

	/** @brief Read from a binary file the data.
	*/
	STOREDATA_CODIFY_EXPORT static int unpack(const std::string &filename,
		char* &data, int &size);

private:

	/** @brief Function to get the file size.
	*/
	static std::ifstream::pos_type filesize(const char* filename);

	/** @brief This function convert a memory block in a collection of images.

		This function convert a memory block in a collection of images.
		The block of memory is expected to be allocated as follow:
		cols(4),rows(4),channels(4),data(n),cols(4)...
		where the number is equal to the number of bytes used to memorize the
		data.
		@param[in] data The block data.
		@param[in] size The amount of data passed.
		@param[out] container A vector with the images memorized.
	*/
	static void memblock2images(const char *memblock, int size,
		std::vector<cv::Mat> &container);

};


} // namespace codify
} // namespace storedata

#endif /* STOREDATA_CODIFY_PACKUNPACKIMAGES_HPP__ */
