/* @file create_video.hpp
* @brief Header of the class which performs all the main functions.
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
* @version 0.5.0.0
*
*/

#ifndef STOREDATA_RECORD_CREATE_BASE_HPP__
#define STOREDATA_RECORD_CREATE_BASE_HPP__

#include <future>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <queue>

#include "record_defines.hpp"
#include "storedata_typedef.hpp"

namespace storedata
{

const int kSuccess = 1;
const int kFail = 0;
const int kOutOfMemory = -1;
const int kDataIsEmpty = -2;
const int kFileIsNotOpen = -3;

/** @brief Class Base for the memorization
*/
class MemorizeManagerBase
{
  public:

	  STOREDATA_RECORD_EXPORT MemorizeManagerBase() {};

	  STOREDATA_RECORD_EXPORT ~MemorizeManagerBase() {};

	  /** @brief Release the allocated data
	  */
	  STOREDATA_RECORD_EXPORT virtual void release() = 0;
};

} // namespace storedata

#endif // STOREDATA_RECORD_CREATE_BASE_HPP__
