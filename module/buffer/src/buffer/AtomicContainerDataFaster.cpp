/* @file AtomicContainerDataFaster.cpp
 * @brief Main file with the example for the hog descriptor and visualization.
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

#include "buffer/inc/buffer/AtomicContainerDataFaster.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
AtomicContainerDataFaster::AtomicContainerDataFaster() {
	data_ = nullptr;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::set_unique_msg(const std::string &unique_msg) {
	unique_msg_ = unique_msg;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::copyFrom(const void* src, 
	size_t src_size_bytes) {
	if (data_) dispose();
	size_bytes_ = src_size_bytes;
	data_ = malloc(size_bytes_);
	std::memcpy(data_, src, size_bytes_);
	safe_dispose_ = true;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::copyFrom(AtomicContainerDataFaster &obj) {
	if (data_) dispose();
	size_bytes_ = obj.size_bytes();
	data_ = malloc(size_bytes_);
	memcpy(data_, obj.data(), obj.size_bytes());
	safe_dispose_ = true;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::assignFrom(void* src, size_t src_size_bytes) {
	if (data_) dispose();
	size_bytes_ = src_size_bytes;
	data_ = src;
	safe_dispose_ = false;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::assignFrom(AtomicContainerDataFaster &obj) {
	if (data_) dispose();
	size_bytes_ = obj.size_bytes();
	data_ = obj.data();
	safe_dispose_ = false;
}
//-----------------------------------------------------------------------------
void AtomicContainerDataFaster::dispose() {
	if (safe_dispose_ && data_) { free(data_); data_ = nullptr; }
}
//-----------------------------------------------------------------------------
std::string AtomicContainerDataFaster::unique_msg() {
	return unique_msg_;
}
//-----------------------------------------------------------------------------
void* AtomicContainerDataFaster::data() {
	return data_;
}
//-----------------------------------------------------------------------------
size_t AtomicContainerDataFaster::size_bytes() {
	return size_bytes_;
}
//-----------------------------------------------------------------------------
bool AtomicContainerDataFaster::safe_dispose() {
	return safe_dispose_;
}

} // namespace storedata
