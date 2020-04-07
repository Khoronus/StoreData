/* @file VolatileTimedBuffer.hpp
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
 * @version 0.1.0.0
 *
 */

#ifndef STOREDATA_VOLATILETIMEDBUFFER_HPP__
#define STOREDATA_VOLATILETIMEDBUFFER_HPP__

#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include <memory>
#include <list>

#include <opencv2/opencv.hpp>
#include "MicroBuffer.hpp"

#include "buffer_defines.hpp"

namespace vb
{

/** @brief Class to manage small buffering data

	This data structure can be used to pass shared memory to other
	objects for a temporary operation. Once it is completed, the memory
	is automatically released.

	For example to keep track of some frames before a short term event happens.
	The memorized frames can be recovered from the event to add to another 
	buffer.

	@thread Non thread safe
*/
class VolatileTimedBuffer
{
public:

	STOREDATA_BUFFER_EXPORT VolatileTimedBuffer();

	/** @brief Size of the macro container
	*/
	STOREDATA_BUFFER_EXPORT size_t size();

	/** @brief Clean the memory
	*/
	STOREDATA_BUFFER_EXPORT void clear();

	/** @brief Clean the memory which timestamp is too old compared to the
	           current timestamp (i.e. 500ms)
	*/
	STOREDATA_BUFFER_EXPORT void clean_buffer(double timestamp, double timestamp_maxdiff);

	/** @brief It creates a new buffer to push new frames
	*/
	STOREDATA_BUFFER_EXPORT void create();

	///** @brief It adds a new frame to the current container.

	//	It adds a new frame to the current container. If there is no space,
	//	it drops the data.
	//*/
	//STOREDATA_BUFFER_EXPORT bool add(double timestamp, 
	//	const std::string &fname, const cv::Mat &m);

	///** @brief It adds a new frame to the current container. If there is no
	//           space, it creates a new buffer.
	//*/
	//STOREDATA_BUFFER_EXPORT bool add_forceexpand(double timestamp, const std::string &fname,
	//	const cv::Mat &m);

	/** @brief It adds a new frame to the current container.

		It adds a new frame to the current container. If there is no space,
		it drops the data.
	*/
	STOREDATA_BUFFER_EXPORT bool add(double timestamp,
		const std::shared_ptr<MicroBufferObjBase> &obj);

	/** @brief It adds a new frame to the current container. If there is no
			   space, it creates a new buffer.
	*/
	STOREDATA_BUFFER_EXPORT bool add_forceexpand(double timestamp, const std::shared_ptr<MicroBufferObjBase> &obj);

	/** @brief It gets the pointer to the container requested.

		Since the microbuffer has a shared memory, it is removed from this
		container but it persists in memory until it is not removed by all
		the observers.
	*/
	STOREDATA_BUFFER_EXPORT void get_ptr_containers(int n, std::vector<PtrMicrobuffer> &vptr);

private:

	/** @brief Pointer to the microbuffer
	*/
	PtrMicrobuffer container_;
	int current_id_;
	std::list<PtrMicrobuffer> macro_container_;
};


} // namespace vb

#endif // STOREDATA_VOLATILETIMEDBUFFER_HPP__