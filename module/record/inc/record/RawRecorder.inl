/* @file RawRecorder.inl
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

#include "RawRecorder.hpp"

namespace storedata
{
//// ----------------------------------------------------------------------------
//template <typename _Ty>
//void RawRecorder::record_t(_Ty data, size_t len) {
//	if (fgm_.under_writing()) return;
//	// Add the information to transmit
//	size_t size_msg_data = len;
//	std::vector<char> info_to_transmit(size_msg_data);
//	memcpy(&info_to_transmit[0], data, size_msg_data);
//
//	// Prepare the container for the data to transmit
//	std::map<int, std::vector<char> > m_data;
//	m_data[0] = std::vector<char>(size_msg_data + 4);
//
//	// Copy the data
//	memcpy(&m_data[0][0], &size_msg_data, sizeof(int));
//	memcpy(&m_data[0][4], &info_to_transmit[0],
//		size_msg_data);
//	if (!fgm_.push_data(m_data)) { std::cout << "lost" << std::endl; }
//}
} // namespace storedata