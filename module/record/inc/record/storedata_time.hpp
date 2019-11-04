/* @file storedata_time.hpp
* @brief Defines for this project
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

#ifndef STOREDATA_CORE_STOREDATATIME_HPP__
#define STOREDATA_CORE_STOREDATATIME_HPP__

#include <string>
// Include Boost headers for system time and threading
#include "boost/date_time/posix_time/posix_time.hpp"

#include "record_defines.hpp"

namespace storedata
{

/** @brief Class to manage the date and time
*/
class DateTime
{
public:

	/** @brief Get the current time and date in string form.
	*/
	static STOREDATA_RECORD_EXPORT std::string get_date_as_string(void);

	/** @brief Get the time in a string format
	*/
	static STOREDATA_RECORD_EXPORT std::string time2string();
};

} // storedata

#endif // STOREDATA_CORE_STOREDATATIME_HPP__
