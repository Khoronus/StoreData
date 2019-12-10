/**
* @file recordcontainergeneric.hpp
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


#ifndef STOREDATA_BUFFER_ATOMICCONTAINERDATA_HPP__
#define STOREDATA_BUFFER_ATOMICCONTAINERDATA_HPP__

#include <memory>
#include "buffer_defines.hpp"

namespace storedata
{

/** @brief Container data to transfer data between threads. It copies the data.
*/
struct STOREDATA_BUFFER_EXPORT AtomicContainerData
{
	void* data;
	size_t size_bytes;
	AtomicContainerData() : data(nullptr) {}
	void copyFrom(const void* src, size_t src_size_bytes) {
		if (data) dispose();
		size_bytes = src_size_bytes;
		data = malloc(size_bytes);
		memcpy(data, src, size_bytes);
	}
	void copyFrom(AtomicContainerData &obj) {
		if (data) dispose();
		size_bytes = obj.size_bytes;
		data = malloc(size_bytes);
		memcpy(data, obj.data, obj.size_bytes);
	}
	void dispose() {
		if (data) { free(data); data = nullptr; }
	}
};

} // namespace storedata

#endif // STOREDATA_BUFFER_ATOMICCONTAINERDATA_HPP__