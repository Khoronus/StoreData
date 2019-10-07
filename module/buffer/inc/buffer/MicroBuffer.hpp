/* @file MicroBuffer.hpp
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
* @version 0.1.0.0
*
*/

#ifndef STOREDATA_MICROBUFFER_HPP__
#define STOREDATA_MICROBUFFER_HPP__

#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include <memory>
#include <list>

#include <opencv2/opencv.hpp>

namespace vb
{

///** @brief Container with pair of timestamp, image
//*/
//typedef std::array<std::pair<double, std::pair<std::string, cv::Mat> >, 10> Microbuffer;
///** @brief Shared memory for multiple micro-containers. When nothing point to 
//    them, they are destroyed.
//*/
//typedef std::shared_ptr<Microbuffer> PtrMicrobuffer;



/** @brief Base class to buffer objects.
*/
class MicroBufferObjBase
{
public:
	MicroBufferObjBase() {}

	virtual void clone(MicroBufferObjBase &obj) {};
	/** @brief Access to the data that is collected in the derived class.
	*/
	virtual void* get_item(int id) {
		return nullptr; }
};

/** @brief Container with pair of timestamp, image
*/
typedef std::array<std::pair<double, std::shared_ptr<MicroBufferObjBase> >, 10> Microbuffer;
/** @brief Shared memory for multiple micro-containers. When nothing point to 
    them, they are destroyed.
*/
typedef std::shared_ptr<Microbuffer> PtrMicrobuffer;

} // namespace vb

#endif // MICROBUFFER_HPP__