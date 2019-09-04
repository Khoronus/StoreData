/* @file PlayerRecorder.hpp
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

#ifndef STOREDATA_RECORD_PLAYRECORDER_HPP__
#define STOREDATA_RECORD_PLAYRECORDER_HPP__

#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

#define BOOST_BUILD
#include "create_file.hpp"
#include "create_video.hpp"

#include "record_defines.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
class PlayerRecorder
{
public:

	STOREDATA_RECORD_EXPORT PlayerRecorder() {
		clear();
	}

	STOREDATA_RECORD_EXPORT ~PlayerRecorder() {
		fgm_.close();
	}

	STOREDATA_RECORD_EXPORT void clear() {
		fgp_.clear();
		vgp_.clear();
	}

	/** @brief Setup the recorder properties
	*/
	STOREDATA_RECORD_EXPORT void setup(const std::string &filename,
		int max_memory_allocable, int fps);
	STOREDATA_RECORD_EXPORT void record(cv::Mat &curr, bool encoded, std::string &msg);
	STOREDATA_RECORD_EXPORT void record(cv::Mat &curr, bool encoded, unsigned char *msg, size_t msg_size);

	/** @brief Setup the recorder properties
		
		Setup the recorder properties.
		@param[in] sources The sources that generates the file (id, image size).
		@param[in] filename Root filename
		@param[in] max_frames_allocable Max number of frames for video
		@param[in] fps Expected video fps
	*/
	STOREDATA_RECORD_EXPORT void setup_video(
		std::map<int, cv::Mat> &sources,
		const std::string &filename,
		int max_frames_allocable, 
		int fps);
	/** @brief Setup the meta frame. It is the first frame of each new video.

		Setup the recorder properties. It is the first frame of each new video.
		@param[in] meta_frame Frame to place at the beginning of each new video
	*/
	STOREDATA_RECORD_EXPORT void setup_metaframe(cv::Mat &meta_frame);
	/** @brief Setup the recorder properties

		Setup the recorder properties.
		@param[in] sources The sources that generates the file (id, image size).
	*/
	STOREDATA_RECORD_EXPORT void record_video(std::map<int, cv::Mat> &sources);

	STOREDATA_RECORD_EXPORT void play(const std::string &filename, int FPS);

	/** @brief Unpack previously recorded data

		The data will be saved in the folder path with the correct pair 
		filename.
	*/
	STOREDATA_RECORD_EXPORT void unpack(const std::string &filename, int FPS,
		const std::string &path, unsigned int &index_start);

	/** @brief It sets the callback for the function that create a new file
	*/
	STOREDATA_RECORD_EXPORT void set_callback_createfile(
		cbk_fname_changed callback_createfile);

private:

	/** @brief File Generator manager
	*/
	FileGeneratorManagerAsync fgm_;
	/** @brief File Generator parameters
	*/
	std::map<int, FileGeneratorParams> fgp_;

	// <video>
	VideoGeneratorManager vgm_;
	/** @brief File Generator parameters
	*/
	std::map<int, VideoGeneratorParams> vgp_;

	/** @brief It reads the recorded data
	*/
	void data2data_type(char *data, int maxsize, 
		std::vector< std::pair<cv::Mat, std::vector<char> > > &out);
};

} // namespace storedata

#endif  // STOREDATA_RECORD_PLAYRECORDER_HPP__
