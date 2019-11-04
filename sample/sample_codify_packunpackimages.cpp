/* @file sample_imgproc_packunpackimages.cpp
 * @brief Example of packing and unpacking image data.
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

#include <iostream>
#include "codify/inc/codify/packunpack_images.hpp"
//#include "ts.hpp"

namespace
{


/** @brief Simple function to save file data.
*/
void testwritedata()
{
	int size = 4 * 6 * 21;
	char *data = new char[size];
	for (int i = 0; i < 100; i++)
	{
		storedata::codify::PackUnpackImages::pack(
			"..\\..\\data\\packdata.dat", data, size, 50000, false);
	}
	delete[] data;

	std::vector<char> data2(100);
	for (int i = 0; i < 100; i++)
	{
		data2[i] = i;
	}
	storedata::codify::PackUnpackImages::pack(
		"..\\..\\data\\packdata.dat", data2, 500000, true);
}


/** @brief Pack image data in OpenCV format
*/
void test()
{
	std::vector<cv::Mat> container;
	for (int i = 0; i < 10; i++)
	{
		container.push_back(cv::Mat(rand() % 605 + 10, rand() % 705 + 10, CV_8UC3,
			cv::Scalar(rand() % 255, rand() % 255, rand() % 255)));
	}

	std::cout << "pack: " << storedata::codify::PackUnpackImages::pack(
		"packimage.dat", container, 5000000, false);
	std::cout << "unpack: " << storedata::codify::PackUnpackImages::unpack(
		"packimage.dat", container);

	std::cout << std::endl << container.size() << std::endl;

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = container.begin(); it != container.end(); it++)
#else
	for (std::vector<cv::Mat>::const_iterator it = container.begin(); it != container.end(); it++)
#endif
	{
		cv::imshow("image", *it);
		cv::waitKey(500);
	}
}

}	// namespace

#ifdef CmnLib

cmnLIBRARY_TEST_MAIN(&test, "data\\MemoryLeakCPP.txt", "data\\MemoryLeakC.txt");

#else

/** main
*/
int main(int argc, char *argv[])
{
	test();
	//testwritedata();
	return 0;
}

#endif