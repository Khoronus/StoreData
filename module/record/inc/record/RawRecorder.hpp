/* @file RawRecorder.hpp
 * @brief Header of the related class
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

#ifndef STOREDATA_RECORD_RAWRECORDER_HPP__
#define STOREDATA_RECORD_RAWRECORDER_HPP__

#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

#define BOOST_BUILD
#include "create_file.hpp"

// get the library configuration
#include "lib_configuration.hpp"

#ifdef DEF_LIB_ZLIB
#include "zlib.h"
#endif

#include "record_defines.hpp"

namespace storedata
{

/** @brief Class to record raw binary data
*/
class RawRecorder
{
public:

	STOREDATA_RECORD_EXPORT RawRecorder();

	STOREDATA_RECORD_EXPORT ~RawRecorder();

	/** @brief Setup the recorder properties
	*/
	STOREDATA_RECORD_EXPORT void setup(const std::string &filename,
		int max_memory_allocable, int fps);

	STOREDATA_RECORD_EXPORT void record(const std::string &msg);
	STOREDATA_RECORD_EXPORT void record(const std::vector<uint8_t> &data);
	STOREDATA_RECORD_EXPORT void record(uint8_t* data, size_t len);
	template <typename _Ty>
	STOREDATA_RECORD_EXPORT void record_t(_Ty data, size_t len);

	STOREDATA_RECORD_EXPORT void play(const std::string &filename, int FPS);
	STOREDATA_RECORD_EXPORT void play_raw(const std::string &filename, int FPS);

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

	/** @brief It reads the recorded data

		It extracts all the objects pushed in the record
		the data size information is dropped and it is
		possible to extract from the vector size.
	*/
	void data2data_type(char *data, int maxsize, 
		std::vector< std::vector<uint8_t> > &out);

};

} // namespace storedata

//#include "RawRecorder.inl"

//// explicit instantiation
//template void storedata::RawRecorder::record_t<uint8_t*>;
//template STOREDATA_RECORD_EXPORT storedata::RawRecorder::record_t<const uint8_t*>;
//template STOREDATA_RECORD_EXPORT storedata::RawRecorder::record_t<int8_t*>;
//template STOREDATA_RECORD_EXPORT storedata::RawRecorder::record_t<const int8_t*>;

#endif  // STOREDATA_RECORD_RAWRECORDER_HPP__
