/* @file VolatileTimedBuffer.cpp
 * @brief Body of the related class
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


#include "buffer/inc/buffer/VolatileTimedBuffer.hpp"

namespace vb
{
// ----------------------------------------------------------------------------
VolatileTimedBuffer::VolatileTimedBuffer() {
	current_id_ = 0;
}
// ----------------------------------------------------------------------------
size_t VolatileTimedBuffer::size() {
	return macro_container_.size();
}
//-----------------------------------------------------------------------------
void VolatileTimedBuffer::clean(double timestamp, double timestamp_maxdiff) {
	//std::cout << "Total buffers: " <<
	//	macro_container_.size() << std::endl;
	std::vector<PtrMicrobuffer> vremove;
	for (auto it : macro_container_)
	{
		//std::cout << "timestamp - it->back().first: " << timestamp << " " << it->front().first << std::endl;
		// do not test the last container
		//if (it != macro_container_.back() &&
		//	(timestamp - it->back().first) > timestamp_maxdiff) {
		//	vremove.push_back(it);
		//}
		if ((timestamp - it->front().first) > timestamp_maxdiff) {
			vremove.push_back(it);
		}

	}
	//if (vremove.size() > 0) std::cout << "SmallBuffer::clean:remove" << std::endl;
	for (auto it : vremove)
	{
		macro_container_.remove(it);
	}
}
//-----------------------------------------------------------------------------
void VolatileTimedBuffer::create() {
	current_id_ = 0;
	macro_container_.push_back(PtrMicrobuffer(new Microbuffer));
	container_ = macro_container_.back();
}
////-----------------------------------------------------------------------------
//bool VolatileTimedBuffer::add(double timestamp, const std::string &fname,
//	const cv::Mat &m) {
//	if (!container_) {
//		//std::cout << "SmallBuffer::add" << std::endl;
//		create();
//	}
//	if (current_id_ < container_->size()) {
//		(*container_)[current_id_++] =
//			std::make_pair(timestamp, std::make_pair(fname, m.clone()));
//		return true;
//	} /*else {
//		std::cout << "SmallBuffer::add: Drop frame" << std::endl;
//	}*/
//	return false;
//}
////-----------------------------------------------------------------------------
//bool VolatileTimedBuffer::add_forceexpand(double timestamp, 
//	const std::string &fname, const cv::Mat &m) {
//	if (!container_) {
//		//std::cout << "SmallBuffer::add" << std::endl;
//		create();
//	}
//	if (current_id_ < container_->size()) {
//		(*container_)[current_id_++] =
//			std::make_pair(timestamp, std::make_pair(fname, m.clone()));
//		return true;
//	} else {
//		create();
//		return add(timestamp, fname, m);
//	}
//	return false;
//}
//-----------------------------------------------------------------------------
bool VolatileTimedBuffer::add(double timestamp, const std::shared_ptr<MicroBufferObjBase> &obj) {
	if (!container_) {
		//std::cout << "SmallBuffer::add" << std::endl;
		create();
	}
	if (current_id_ < container_->size()) {
		(*container_)[current_id_++] =
			std::make_pair(timestamp, obj);
		return true;
	} /*else {
		std::cout << "SmallBuffer::add: Drop frame" << std::endl;
	}*/
	return false;
}
//-----------------------------------------------------------------------------
bool VolatileTimedBuffer::add_forceexpand(double timestamp, const std::shared_ptr<MicroBufferObjBase> &obj) {
	if (!container_) {
		//std::cout << "SmallBuffer::add" << std::endl;
		create();
	}
	if (current_id_ < container_->size()) {
		(*container_)[current_id_++] =
			std::make_pair(timestamp, obj);
		return true;
	} else {
		create();
		return add(timestamp, obj);
	}
	return false;
}
//-----------------------------------------------------------------------------
void VolatileTimedBuffer::get_ptr_containers(int n,
	std::vector<PtrMicrobuffer> &vptr) {
	int num_items = 0;
	for (std::list<PtrMicrobuffer>::const_reverse_iterator it = macro_container_.rbegin();
		it != macro_container_.rend(); ++it) {

		if (num_items >= n) break;
		vptr.push_back(*it);
		++num_items;
	}
}


} // namespace vb