/**
* @file AtomicContainerData.cpp
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


#include "buffer/inc/buffer/AtomicContainerData.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
AtomicContainerData::AtomicContainerData() {
	data_ = nullptr;
}
//-----------------------------------------------------------------------------
void AtomicContainerData::copyFrom(const void* src, size_t src_size_bytes) {
	if (data_) dispose();
	size_bytes_ = src_size_bytes;
	data_ = malloc(size_bytes_);
	std::memcpy(data_, src, size_bytes_);
}
//-----------------------------------------------------------------------------
void AtomicContainerData::copyFrom(AtomicContainerData &obj) {
	if (data_) dispose();
	size_bytes_ = obj.size_bytes();
	data_ = malloc(size_bytes_);
	memcpy(data_, obj.data(), size_bytes_);
}
//-----------------------------------------------------------------------------
void AtomicContainerData::dispose() {
	if (data_) { free(data_); data_ = nullptr; }
}
//-----------------------------------------------------------------------------
void* AtomicContainerData::data() {
	return data_;
}
//-----------------------------------------------------------------------------
size_t AtomicContainerData::size_bytes() {
	return size_bytes_;
}

} // namespace storedata
