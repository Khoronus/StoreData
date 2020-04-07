/**
* @file AtomicContainerDataFaster.hpp
* @brief Header of the defined class
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
* @original author Alessandro Moro <alessandromoro.italy@gmail.com>
* @bug No known bugs.
* @version 0.2.0.0
*
*/


#ifndef STOREDATA_BUFFER_ATOMICCONTAINERDATAFASTER_HPP__
#define STOREDATA_BUFFER_ATOMICCONTAINERDATAFASTER_HPP__

#include <string>
#include <memory>
#include "buffer_defines.hpp"

namespace storedata
{

/** @brief Container data to transfer data between threads. It copies the data.
*/
struct STOREDATA_BUFFER_EXPORT AtomicContainerDataFaster
{
//	std::unique_ptr<void> data_Ty;
	// Associated unique message to the container data
	std::string unique_msg_;
	void* data;
	size_t size_bytes;
	bool safe_dispose;
	AtomicContainerDataFaster() : data(nullptr) {}

	void set_unique_msg(const std::string &unique_msg) {
		unique_msg_ = unique_msg;
	}
	void copyFrom(const void* src, size_t src_size_bytes) {
		if (data) dispose();
		size_bytes = src_size_bytes;
		data = malloc(size_bytes);
		memcpy(data, src, size_bytes);
		safe_dispose = true;
	}
	void copyFrom(AtomicContainerDataFaster &obj) {
		if (data) dispose();
		size_bytes = obj.size_bytes;
		data = malloc(size_bytes);
		memcpy(data, obj.data, obj.size_bytes);
		safe_dispose = true;
	}
	void assignFrom(void* src, size_t src_size_bytes) {
		if (data) dispose();
		size_bytes = src_size_bytes;
		data = src;
		safe_dispose = false;
	}
	void assignFrom(AtomicContainerDataFaster &obj) {
		if (data) dispose();
		size_bytes = obj.size_bytes;
		data = obj.data;
		safe_dispose = false;
	}
	void dispose() {
		if (safe_dispose && data) { free(data); data = nullptr; }
	}

	//void moveFrom(std::unique_ptr<void> &obj) {
	//	data_Ty = std::move(obj);
	//}
	//void moveTo(std::unique_ptr<void> &obj) {
	//	obj = std::move(data_Ty);
	//}

};

} // namespace storedata

#endif // STOREDATA_BUFFER_ATOMICCONTAINERDATAFASTER_HPP__