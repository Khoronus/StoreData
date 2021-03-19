/* @file sample_logger.cpp
 * @brief Main logger example file.
 *
 * @section LICENSE
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
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


#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "logger/logger_headers.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief Function to test the small buffer
*/
void test_logger () {
	// Initialize the Log
	// It does not save and display messages from lower level than set one.
	std::cout << "Create LogMS: " <<
		CmnLib::control::LogMS::ResetLogFile("CoreLog.txt") << std::endl;
	CmnLib::control::LogMS::ResetLogLevel(CmnLib::control::LogLevel::Debug);
	CmnLib::control::LogMS::Info(std::string(std::string("Compiled: ") + __DATE__ + " " + __TIME__ + std::string("\n")).c_str());
}

} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[])
{
	test_logger();
	
	return 0;
}
