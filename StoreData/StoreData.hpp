/**
* @file StoreData.hpp
* @brief Header of the relative class
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
* @original author Alessandro Moro <alessandromoro.italy@google.com>
* @bug No known bugs.
* @version 0.1.0.0
*
*/


#ifndef STOREDATA_STOREDATA_HPP__
#define STOREDATA_STOREDATA_HPP__

#include <string>

#include "storedata_defines.hpp"
#include "../module/version/version.hpp"

/** @brief Class to record all the frames currently captured
*/
class STOREDATA_EXPORT StoreDataVersion
{

public:

	/** @brief Get the version of the library
	*/
	static std::string get_version();

	/** @brief Get the version of the library
	*/
	static int get_version_int();

private:

	StoreDataVersion() {}

};

#endif // STOREDATA_STOREDATA_HPP__